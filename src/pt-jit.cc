/*

Copyright (C) 2012 Max Brister <max@2bass.com>

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "pt-jit.h"

#include <typeinfo>

#include <llvm/LLVMContext.h>
#include <llvm/Module.h>
#include <llvm/Function.h>
#include <llvm/BasicBlock.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/JIT.h>
#include <llvm/PassManager.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Analysis/CallGraph.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Target/TargetData.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/ExecutionEngine/GenericValue.h>

#include "octave.h"
#include "ov-fcn-handle.h"
#include "ov-usr-fcn.h"
#include "pt-all.h"

// FIXME: Remove eventually
// For now we leave this in so people tell when JIT actually happens
static const bool debug_print = false;

static llvm::IRBuilder<> builder (llvm::getGlobalContext ());

// thrown when we should give up on JIT and interpret
class jit_fail_exception : public std::exception {};

static void
fail (void)
{
  throw jit_fail_exception ();
}

// function that jit code calls
extern "C" void
octave_jit_print_any (const char *name, octave_base_value *obv)
{
  obv->print_with_name (octave_stdout, name, true);
}

extern "C" void
octave_jit_print_double (const char *name, double value)
{
  // FIXME: We should avoid allocating a new octave_scalar each time
  octave_value ov (value);
  ov.print_with_name (octave_stdout, name);
}

extern "C" octave_base_value*
octave_jit_binary_any_any (octave_value::binary_op op, octave_base_value *lhs,
                           octave_base_value *rhs)
{
  octave_value olhs (lhs);
  octave_value orhs (rhs);
  octave_value result = do_binary_op (op, olhs, orhs);
  octave_base_value *rep = result.internal_rep ();
  rep->grab ();
  return rep;
}

extern "C" void
octave_jit_release_any (octave_base_value *obv)
{
  obv->release ();
}

extern "C" void
octave_jit_grab_any (octave_base_value *obv)
{
  obv->grab ();
}

// -------------------- jit_type --------------------
llvm::Type *
jit_type::to_llvm_arg (void) const
{
  return llvm_type ? llvm_type->getPointerTo () : 0;
}

// -------------------- jit_function --------------------
void
jit_function::add_overload (const overload& func,
                            const std::vector<jit_type*>& args)
{
  if (args.size () >= overloads.size ())
    overloads.resize (args.size () + 1);

  Array<overload>& over = overloads[args.size ()];
  dim_vector dv (over.dims ());
  Array<octave_idx_type> idx = to_idx (args);
  bool must_resize = false;

  if (dv.length () != idx.numel ())
    {
      dv.resize (idx.numel ());
      must_resize = true;
    }

  for (octave_idx_type i = 0; i < dv.length (); ++i)
    if (dv(i) <= idx(i))
      {
        must_resize = true;
        dv(i) = idx(i) + 1;
      }

  if (must_resize)
    over.resize (dv);

  over(idx) = func;
}

const jit_function::overload&
jit_function::get_overload (const std::vector<jit_type*>& types) const
{
  // FIXME: We should search for the next best overload on failure
  static overload null_overload;
  if (types.size () >= overloads.size ())
    return null_overload;

  const Array<overload>& over = overloads[types.size ()];
  dim_vector dv (over.dims ());
  Array<octave_idx_type> idx = to_idx (types);
  for (octave_idx_type i = 0; i < dv.length (); ++i)
    if (idx(i) >= dv(i))
      return null_overload;

  return over(idx);
}

Array<octave_idx_type>
jit_function::to_idx (const std::vector<jit_type*>& types) const
{
  octave_idx_type numel = types.size ();
  if (numel == 1)
    numel = 2;

  Array<octave_idx_type> idx (dim_vector (1, numel));
  for (octave_idx_type i = 0; i < static_cast<octave_idx_type> (types.size ());
       ++i)
    idx(i) = types[i]->type_id ();

  if (types.size () == 1)
    {
      idx(1) = idx(0);
      idx(0) = 0;
    }

  return idx;
}

// -------------------- jit_typeinfo --------------------
jit_typeinfo::jit_typeinfo (llvm::Module *m, llvm::ExecutionEngine *e)
  : module (m), engine (e), next_id (0)
{
  // FIXME: We should be registering types like in octave_value_typeinfo
  llvm::LLVMContext &ctx = m->getContext ();

  ov_t = llvm::StructType::create (ctx, "octave_base_value");
  ov_t = ov_t->getPointerTo ();

  llvm::Type *dbl = llvm::Type::getDoubleTy (ctx);
  llvm::Type *bool_t = llvm::Type::getInt1Ty (ctx);
  llvm::Type *index_t = 0;
  switch (sizeof(octave_idx_type))
    {
    case 4:
      index_t = llvm::Type::getInt32Ty (ctx);
      break;
    case 8:
      index_t = llvm::Type::getInt64Ty (ctx);
      break;
    default:
      assert (false && "Unrecognized index type size");
    }

  llvm::StructType *range_t = llvm::StructType::create (ctx, "range");
  std::vector<llvm::Type *> range_contents (4, dbl);
  range_contents[3] = index_t;
  range_t->setBody (range_contents);

  // create types
  any = new_type ("any", true, 0, ov_t);
  scalar = new_type ("scalar", false, any, dbl);
  range = new_type ("range", false, any, range_t);
  boolean = new_type ("bool", false, any, bool_t);
  index = new_type ("index", false, any, index_t);

  // any with anything is an any op
  llvm::Function *fn;
  llvm::Type *binary_op_type
    = llvm::Type::getIntNTy (ctx, sizeof (octave_value::binary_op));
  llvm::Function *any_binary = create_function ("octave_jit_binary_any_any",
                                                any->to_llvm (), binary_op_type,
                                                any->to_llvm (), any->to_llvm ());
  engine->addGlobalMapping (any_binary,
                            reinterpret_cast<void*>(&octave_jit_binary_any_any));

  binary_ops.resize (octave_value::num_binary_ops);
  for (int op = 0; op < octave_value::num_binary_ops; ++op)
    {
      llvm::Twine fn_name ("octave_jit_binary_any_any_");
      fn_name = fn_name + llvm::Twine (op);
      fn = create_function (fn_name, any, any, any);
      llvm::BasicBlock *block = llvm::BasicBlock::Create (ctx, "body", fn);
      builder.SetInsertPoint (block);
      llvm::APInt op_int(sizeof (octave_value::binary_op), op,
                         std::numeric_limits<octave_value::binary_op>::is_signed);
      llvm::Value *op_as_llvm = llvm::ConstantInt::get (binary_op_type, op_int);
      llvm::Value *ret = builder.CreateCall3 (any_binary,
                                                 op_as_llvm,
                                                 fn->arg_begin (),
                                                 ++fn->arg_begin ());
      builder.CreateRet (ret);

      jit_function::overload overload (fn, true, any, any, any);
      for (octave_idx_type i = 0; i < next_id; ++i)
        binary_ops[op].add_overload (overload);
    }

  llvm::Type *void_t = llvm::Type::getVoidTy (ctx);

  // grab any
  fn = create_function ("octave_jit_grab_any", void_t, any->to_llvm ());
                        
  engine->addGlobalMapping (fn, reinterpret_cast<void*>(&octave_jit_grab_any));
  grab_fn.add_overload (fn, false, 0, any);

  // release any
  fn = create_function ("octave_jit_release_any", void_t, any->to_llvm ());
  engine->addGlobalMapping (fn, reinterpret_cast<void*>(&octave_jit_release_any));
  release_fn.add_overload (fn, false, 0, any);

  // now for binary scalar operations
  // FIXME: Finish all operations
  add_binary_op (scalar, octave_value::op_add, llvm::Instruction::FAdd);
  add_binary_op (scalar, octave_value::op_sub, llvm::Instruction::FSub);
  add_binary_op (scalar, octave_value::op_mul, llvm::Instruction::FMul);
  add_binary_op (scalar, octave_value::op_el_mul, llvm::Instruction::FMul);

  // FIXME: Warn if rhs is zero
  add_binary_op (scalar, octave_value::op_div, llvm::Instruction::FDiv);
  add_binary_op (scalar, octave_value::op_el_div, llvm::Instruction::FDiv);

  add_binary_fcmp (scalar, octave_value::op_lt, llvm::CmpInst::FCMP_ULT);
  add_binary_fcmp (scalar, octave_value::op_le, llvm::CmpInst::FCMP_ULE);
  add_binary_fcmp (scalar, octave_value::op_eq, llvm::CmpInst::FCMP_UEQ);
  add_binary_fcmp (scalar, octave_value::op_ge, llvm::CmpInst::FCMP_UGE);
  add_binary_fcmp (scalar, octave_value::op_gt, llvm::CmpInst::FCMP_UGT);
  add_binary_fcmp (scalar, octave_value::op_ne, llvm::CmpInst::FCMP_UNE);

  // now for printing functions
  add_print (any, reinterpret_cast<void*> (&octave_jit_print_any));
  add_print (scalar, reinterpret_cast<void*> (&octave_jit_print_double));

  // bounds check for for loop
  fn = create_function ("octave_jit_simple_for_range", boolean, range, index);
  llvm::BasicBlock *body = llvm::BasicBlock::Create (ctx, "body", fn);
  builder.SetInsertPoint (body);
  {
    llvm::Value *nelem
      = builder.CreateExtractValue (fn->arg_begin (), 3);
    // llvm::Value *idx = builder.CreateLoad (++fn->arg_begin ());
    llvm::Value *idx = ++fn->arg_begin ();
    llvm::Value *ret = builder.CreateICmpULT (idx, nelem);
    builder.CreateRet (ret);
  }
  llvm::verifyFunction (*fn);
  simple_for_check.add_overload (fn, false, boolean, range, index);

  // increment for for loop
  fn = create_function ("octave_jit_imple_for_range_incr", index, index);
  body = llvm::BasicBlock::Create (ctx, "body", fn);
  builder.SetInsertPoint (body);
  {
    llvm::Value *one = llvm::ConstantInt::get (index_t, 1);
    llvm::Value *idx = fn->arg_begin ();
    llvm::Value *ret = builder.CreateAdd (idx, one);
    builder.CreateRet (ret);
  }
  llvm::verifyFunction (*fn);
  simple_for_incr.add_overload (fn, false, index, index);

  // index variabe for for loop
  fn = create_function ("octave_jit_simple_for_idx", scalar, range, index);
  body = llvm::BasicBlock::Create (ctx, "body", fn);
  builder.SetInsertPoint (body);
  {
    llvm::Value *idx = ++fn->arg_begin ();
    llvm::Value *didx = builder.CreateUIToFP (idx, dbl);
    llvm::Value *rng = fn->arg_begin ();
    llvm::Value *base = builder.CreateExtractValue (rng, 0);
    llvm::Value *inc = builder.CreateExtractValue (rng, 2);

    llvm::Value *ret = builder.CreateFMul (didx, inc);
    ret = builder.CreateFAdd (base, ret);
    builder.CreateRet (ret);
  }
  llvm::verifyFunction (*fn);
  simple_for_index.add_overload (fn, false, scalar, range, index);

  // logically true
  // FIXME: Check for NaN
  fn = create_function ("octave_logically_true_scalar", boolean, scalar);
  body = llvm::BasicBlock::Create (ctx, "body", fn);
  builder.SetInsertPoint (body);
  {
    llvm::Value *zero = llvm::ConstantFP::get (scalar->to_llvm (), 0);
    llvm::Value *ret = builder.CreateFCmpUNE (fn->arg_begin (), zero);
    builder.CreateRet (ret);
  }
  llvm::verifyFunction (*fn);
  logically_true.add_overload (fn, true, boolean, scalar);

  fn = create_function ("octave_logically_true_bool", boolean, boolean);
  body = llvm::BasicBlock::Create (ctx, "body", fn);
  builder.SetInsertPoint (body);
  builder.CreateRet (fn->arg_begin ());
  llvm::verifyFunction (*fn);
  logically_true.add_overload (fn, false, boolean, boolean);
}

void
jit_typeinfo::add_print (jit_type *ty, void *call)
{
  std::stringstream name;
  name << "octave_jit_print_" << ty->name ();

  llvm::LLVMContext &ctx = llvm::getGlobalContext ();
  llvm::Type *void_t = llvm::Type::getVoidTy (ctx);
  llvm::Function *fn = create_function (name.str (), void_t,
                                        llvm::Type::getInt8PtrTy (ctx),
                                        ty->to_llvm ());
  engine->addGlobalMapping (fn, call);

  jit_function::overload ol (fn, false, 0, ty);
  print_fn.add_overload (ol);
}

// FIXME: cp between add_binary_op, add_binary_icmp, and add_binary_fcmp
void
jit_typeinfo::add_binary_op (jit_type *ty, int op, int llvm_op)
{
  std::stringstream fname;
  octave_value::binary_op ov_op = static_cast<octave_value::binary_op>(op);
  fname << "octave_jit_" << octave_value::binary_op_as_string (ov_op)
        << "_" << ty->name ();

  llvm::LLVMContext &ctx = llvm::getGlobalContext ();
  llvm::Function *fn = create_function (fname.str (), ty, ty, ty);
  llvm::BasicBlock *block = llvm::BasicBlock::Create (ctx, "body", fn);
  builder.SetInsertPoint (block);
  llvm::Instruction::BinaryOps temp
    = static_cast<llvm::Instruction::BinaryOps>(llvm_op);
  llvm::Value *ret = builder.CreateBinOp (temp, fn->arg_begin (),
                                          ++fn->arg_begin ());
  builder.CreateRet (ret);
  llvm::verifyFunction (*fn);

  jit_function::overload ol(fn, false, ty, ty, ty);
  binary_ops[op].add_overload (ol);
}

void
jit_typeinfo::add_binary_icmp (jit_type *ty, int op, int llvm_op)
{
  std::stringstream fname;
  octave_value::binary_op ov_op = static_cast<octave_value::binary_op>(op);
  fname << "octave_jit" << octave_value::binary_op_as_string (ov_op)
        << "_" << ty->name ();

  llvm::LLVMContext &ctx = llvm::getGlobalContext ();
  llvm::Function *fn = create_function (fname.str (), boolean, ty, ty);
  llvm::BasicBlock *block = llvm::BasicBlock::Create (ctx, "body", fn);
  builder.SetInsertPoint (block);
  llvm::CmpInst::Predicate temp
    = static_cast<llvm::CmpInst::Predicate>(llvm_op);
  llvm::Value *ret = builder.CreateICmp (temp, fn->arg_begin (),
                                         ++fn->arg_begin ());
  builder.CreateRet (ret);
  llvm::verifyFunction (*fn);

  jit_function::overload ol (fn, false, boolean, ty, ty);
  binary_ops[op].add_overload (ol);
}

void
jit_typeinfo::add_binary_fcmp (jit_type *ty, int op, int llvm_op)
{
  std::stringstream fname;
  octave_value::binary_op ov_op = static_cast<octave_value::binary_op>(op);
  fname << "octave_jit" << octave_value::binary_op_as_string (ov_op)
        << "_" << ty->name ();

  llvm::LLVMContext &ctx = llvm::getGlobalContext ();
  llvm::Function *fn = create_function (fname.str (), boolean, ty, ty);
  llvm::BasicBlock *block = llvm::BasicBlock::Create (ctx, "body", fn);
  builder.SetInsertPoint (block);
  llvm::CmpInst::Predicate temp
    = static_cast<llvm::CmpInst::Predicate>(llvm_op);
  llvm::Value *ret = builder.CreateFCmp (temp, fn->arg_begin (),
                                         ++fn->arg_begin ());
  builder.CreateRet (ret);
  llvm::verifyFunction (*fn);

  jit_function::overload ol (fn, false, boolean, ty, ty);
  binary_ops[op].add_overload (ol);
}

llvm::Function *
jit_typeinfo::create_function (const llvm::Twine& name, llvm::Type *ret,
                               const std::vector<llvm::Type *>& args)
{
  llvm::FunctionType *ft = llvm::FunctionType::get (ret, args, false);
  llvm::Function *fn = llvm::Function::Create (ft,
                                               llvm::Function::ExternalLinkage,
                                               name, module);
  fn->addFnAttr (llvm::Attribute::AlwaysInline);
  return fn;
}    

jit_type*
jit_typeinfo::type_of (const octave_value &ov) const
{
  if (ov.is_undefined () || ov.is_function ())
    return 0;

  if (ov.is_double_type () && ov.is_real_scalar ())
    return get_scalar ();

  if (ov.is_range ())
    return get_range ();

  return get_any ();
}

const jit_function&
jit_typeinfo::binary_op (int op) const
{
  assert (static_cast<size_t>(op) < binary_ops.size ());
  return binary_ops[op];
}

const jit_function::overload&
jit_typeinfo::print_value (jit_type *to_print) const
{
  return print_fn.get_overload (to_print);
}

void
jit_typeinfo::to_generic (jit_type *type, llvm::GenericValue& gv)
{
  if (type == any)
    to_generic (type, gv, octave_value ());
  else if (type == scalar)
    to_generic (type, gv, octave_value (0));
  else if (type == range)
    to_generic (type, gv, octave_value (Range ()));
  else
    assert (false && "Type not supported yet");
}

void
jit_typeinfo::to_generic (jit_type *type, llvm::GenericValue& gv, octave_value ov)
{
  if (type == any)
    {
      octave_base_value *obv = ov.internal_rep ();
      obv->grab ();
      ov_out.push_back (obv);
      gv.PointerVal = &ov_out.back ();
    }
  else if (type == scalar)
    {
      scalar_out.push_back (ov.double_value ());
      gv.PointerVal = &scalar_out.back ();
    }
  else if (type == range)
    {
      range_out.push_back (ov.range_value ());
      gv.PointerVal = &range_out.back ();
    }
  else
    assert (false && "Type not supported yet");
}

octave_value
jit_typeinfo::to_octave_value (jit_type *type, llvm::GenericValue& gv)
{
  if (type == any)
    {
      octave_base_value **ptr = reinterpret_cast<octave_base_value **>(gv.PointerVal);
      return octave_value (*ptr);
    }
  else if (type == scalar)
    {
      double *ptr = reinterpret_cast<double *>(gv.PointerVal);
      return octave_value (*ptr);
    }
  else if (type == range)
    {
      jit_range *ptr = reinterpret_cast<jit_range *>(gv.PointerVal);
      Range rng = *ptr;
      return octave_value (rng);
    }
  else
    assert (false && "Type not supported yet");
}

void
jit_typeinfo::reset_generic (void)
{
  scalar_out.clear ();
  ov_out.clear ();
  range_out.clear ();
}

jit_type*
jit_typeinfo::new_type (const std::string& name, bool force_init,
                        jit_type *parent, llvm::Type *llvm_type)
{
  jit_type *ret = new jit_type (name, force_init, parent, llvm_type, next_id++);
  id_to_type.push_back (ret);
  return ret;
}

// -------------------- jit_infer --------------------
void
jit_infer::infer (tree_simple_for_command& cmd, jit_type *bounds)
{
  infer_simple_for (cmd, bounds);
}

void
jit_infer::visit_anon_fcn_handle (tree_anon_fcn_handle&)
{
  fail ();
}

void
jit_infer::visit_argument_list (tree_argument_list&)
{
  fail ();
}

void
jit_infer::visit_binary_expression (tree_binary_expression& be)
{
  if (is_lvalue)
    fail ();

  if (be.op_type () >= octave_value::num_binary_ops)
    fail ();

  tree_expression *lhs = be.lhs ();
  lhs->accept (*this);
  jit_type *tlhs = type_stack.back ();
  type_stack.pop_back ();

  tree_expression *rhs = be.rhs ();
  rhs->accept (*this);
  jit_type *trhs = type_stack.back ();

  jit_type *result = tinfo->binary_op_result (be.op_type (), tlhs, trhs);
  if (! result)
    fail ();

  type_stack.push_back (result);
}

void
jit_infer::visit_break_command (tree_break_command&)
{
  fail ();
}

void
jit_infer::visit_colon_expression (tree_colon_expression&)
{
  fail ();
}

void
jit_infer::visit_continue_command (tree_continue_command&)
{
  fail ();
}

void
jit_infer::visit_global_command (tree_global_command&)
{
  fail ();
}

void
jit_infer::visit_persistent_command (tree_persistent_command&)
{
  fail ();
}

void
jit_infer::visit_decl_elt (tree_decl_elt&)
{
  fail ();
}

void
jit_infer::visit_decl_init_list (tree_decl_init_list&)
{
  fail ();
}

void
jit_infer::visit_simple_for_command (tree_simple_for_command& cmd)
{
  tree_expression *control = cmd.control_expr ();
  control->accept (*this);

  jit_type *control_t = type_stack.back ();
  type_stack.pop_back ();

  // FIXME: We should improve type inference so we don't have to do this
  // to generate nested for loop code

  // quick hack, check if the for loop bounds are const. If we
  // run at least one, we don't have to merge types
  bool atleast_once = false;
  if (control->is_constant ())
    {
      octave_value over = control->rvalue1 ();
      if (over.is_range ())
        {
          Range rng = over.range_value ();
          atleast_once = rng.nelem () > 0;
        }
    }

  if (atleast_once)
    infer_simple_for (cmd, control_t);
  else
    {
      type_map fallthrough = types;
      infer_simple_for (cmd, control_t);
      merge (types, fallthrough);
    }
}

void
jit_infer::visit_complex_for_command (tree_complex_for_command&)
{
  fail ();
}

void
jit_infer::visit_octave_user_script (octave_user_script&)
{
  fail ();
}

void
jit_infer::visit_octave_user_function (octave_user_function&)
{
  fail ();
}

void
jit_infer::visit_octave_user_function_header (octave_user_function&)
{
  fail ();
}

void
jit_infer::visit_octave_user_function_trailer (octave_user_function&)
{
  fail ();
}

void
jit_infer::visit_function_def (tree_function_def&)
{
  fail ();
}

void
jit_infer::visit_identifier (tree_identifier& ti)
{
  symbol_table::symbol_record_ref record = ti.symbol ();
  handle_identifier (record);
}

void
jit_infer::visit_if_clause (tree_if_clause&)
{
  fail ();
}

void
jit_infer::visit_if_command (tree_if_command& cmd)
{
  if (is_lvalue)
    fail ();

  tree_if_command_list *lst = cmd.cmd_list ();
  assert (lst);
  lst->accept (*this);
}

void
jit_infer::visit_if_command_list (tree_if_command_list& lst)
{
  // determine the types on each branch of the if seperatly, then merge
  type_map fallthrough = types, last;
  bool first_time = true;
  for (tree_if_command_list::iterator p = lst.begin (); p != lst.end(); ++p)
    {
      tree_if_clause *tic = *p;

      if (! first_time)
        types = fallthrough;

      if (! tic->is_else_clause ())
        {
          tree_expression *expr = tic->condition ();
          expr->accept (*this);
        }

      fallthrough = types;

      tree_statement_list *stmt_lst = tic->commands ();
      assert (stmt_lst);
      stmt_lst->accept (*this);

      if (first_time)
        last = types;
      else
        merge (last, types);
    }

  types = last;

  tree_if_clause *last_clause = lst.back ();
  if (! last_clause->is_else_clause ())
    merge (types, fallthrough);
}

void
jit_infer::visit_index_expression (tree_index_expression&)
{
  fail ();
}

void
jit_infer::visit_matrix (tree_matrix&)
{
  fail ();
}

void
jit_infer::visit_cell (tree_cell&)
{
  fail ();
}

void
jit_infer::visit_multi_assignment (tree_multi_assignment&)
{
  fail ();
}

void
jit_infer::visit_no_op_command (tree_no_op_command&)
{
  fail ();
}

void
jit_infer::visit_constant (tree_constant& tc)
{
  if (is_lvalue)
    fail ();

  octave_value v = tc.rvalue1 ();
  jit_type *type = tinfo->type_of (v);
  if (! type)
    fail ();

  type_stack.push_back (type);
}

void
jit_infer::visit_fcn_handle (tree_fcn_handle&)
{
  fail ();
}

void
jit_infer::visit_parameter_list (tree_parameter_list&)
{
  fail ();
}

void
jit_infer::visit_postfix_expression (tree_postfix_expression&)
{
  fail ();
}

void
jit_infer::visit_prefix_expression (tree_prefix_expression&)
{
  fail ();
}

void
jit_infer::visit_return_command (tree_return_command&)
{
  fail ();
}

void
jit_infer::visit_return_list (tree_return_list&)
{
  fail ();
}

void
jit_infer::visit_simple_assignment (tree_simple_assignment& tsa)
{
  if (is_lvalue)
    fail ();

  // resolve rhs
  is_lvalue = false;
  tree_expression *rhs = tsa.right_hand_side ();
  rhs->accept (*this);

  jit_type *trhs = type_stack.back ();
  type_stack.pop_back ();

  // resolve lhs
  is_lvalue = true;
  rvalue_type = trhs;
  tree_expression *lhs = tsa.left_hand_side ();
  lhs->accept (*this);

  // we don't pop back here, as the resulting type should be the rhs type
  // which is equal to the lhs type anways
  jit_type *tlhs = type_stack.back ();
  if (tlhs != trhs)
    fail ();

  is_lvalue = false;
  rvalue_type = 0;
}

void
jit_infer::visit_statement (tree_statement& stmt)
{
  if (is_lvalue)
    fail ();

  tree_command *cmd = stmt.command ();
  tree_expression *expr = stmt.expression ();

  if (cmd)
    cmd->accept (*this);
  else
    {
      // ok, this check for ans appears three times as cp
      bool do_bind_ans = false;

      if (expr->is_identifier ())
        {
          tree_identifier *id = dynamic_cast<tree_identifier *> (expr);

          do_bind_ans = (! id->is_variable ());
        }
      else
        do_bind_ans = (! expr->is_assignment_expression ());

      expr->accept (*this);

      if (do_bind_ans)
        {
          is_lvalue = true;
          rvalue_type = type_stack.back ();
          type_stack.pop_back ();

          symbol_table::symbol_record_ref record (symbol_table::insert ("ans"));
          handle_identifier (record);

          if (rvalue_type != type_stack.back ())
            fail ();

          is_lvalue = false;
          rvalue_type = 0;
        }

      type_stack.pop_back ();
    }
}

void
jit_infer::visit_statement_list (tree_statement_list& lst)
{
  tree_statement_list::iterator iter;
  for (iter = lst.begin (); iter != lst.end (); ++iter)
    {
      tree_statement *stmt = *iter;
      assert (stmt); // FIXME: jwe can this be null?
      stmt->accept (*this);
    }
}

void
jit_infer::visit_switch_case (tree_switch_case&)
{
  fail ();
}

void
jit_infer::visit_switch_case_list (tree_switch_case_list&)
{
  fail ();
}

void
jit_infer::visit_switch_command (tree_switch_command&)
{
  fail ();
}

void
jit_infer::visit_try_catch_command (tree_try_catch_command&)
{
  fail ();
}

void
jit_infer::visit_unwind_protect_command (tree_unwind_protect_command&)
{
  fail ();
}

void
jit_infer::visit_while_command (tree_while_command&)
{
  fail ();
}

void
jit_infer::visit_do_until_command (tree_do_until_command&)
{
  fail ();
}

void
jit_infer::infer_simple_for (tree_simple_for_command& cmd,
                             jit_type *bounds)
{
  if (is_lvalue)
    fail ();

  jit_type *iter = tinfo->get_simple_for_index_result (bounds);
  if (! iter)
    fail ();

  is_lvalue = true;
  rvalue_type = iter;
  tree_expression *lhs = cmd.left_hand_side ();
  lhs->accept (*this);
  if (type_stack.back () != iter)
    fail ();
  type_stack.pop_back ();
  is_lvalue = false;
  rvalue_type = 0;

  tree_statement_list *body = cmd.body ();
  body->accept (*this);
}

void
jit_infer::handle_identifier (const symbol_table::symbol_record_ref& record)
{
  type_map::iterator iter = types.find (record);
  if (iter == types.end ())
    {
      jit_type *ty = tinfo->type_of (record->find ());
      bool argin = false;
      if (is_lvalue)
        {
          if (! ty)
            ty = rvalue_type;
        }
      else
        {
          if (! ty)
            fail ();
          argin = true;
        }

      types[record] = type_entry (argin, ty);
      type_stack.push_back (ty);
    }
  else
    type_stack.push_back (iter->second.second);
}

void
jit_infer::merge (type_map& dest, const type_map& src)
{
  if (dest.size () != src.size ())
    fail ();

  type_map::iterator dest_iter;
  type_map::const_iterator src_iter;
  for (dest_iter = dest.begin (), src_iter = src.begin ();
       dest_iter != dest.end (); ++dest_iter, ++src_iter)
    {
      if (dest_iter->first.name () != src_iter->first.name ()
          || dest_iter->second.second != src_iter->second.second)
        fail ();

      // require argin if one path requires argin
      dest_iter->second.first = dest_iter->second.first
        || src_iter->second.first;
    }
}

// -------------------- jit_generator --------------------
jit_generator::jit_generator (jit_typeinfo *ti, llvm::Module *mod,
                              tree_simple_for_command& cmd, jit_type *bounds,
                              const type_map& infered_types)
  : tinfo (ti), module (mod), is_lvalue (false)
{
  // create new vectors that include bounds
  std::vector<std::string> names (infered_types.size () + 1);
  std::vector<bool> argin (infered_types.size () + 1);
  std::vector<jit_type *> types (infered_types.size () + 1);
  names[0] = "#bounds";
  argin[0] = true;
  types[0] = bounds;
  size_t i;
  type_map::const_iterator iter;
  for (i = 1, iter = infered_types.begin (); iter != infered_types.end ();
       ++i, ++iter)
    {
      names[i] = iter->first.name ();
      argin[i] = iter->second.first;
      types[i] = iter->second.second;
    }

  initialize (names, argin, types);

  try
    {
      value var_bounds = variables["#bounds"];
      var_bounds.second = builder.CreateLoad (var_bounds.second);
      emit_simple_for (cmd, var_bounds, true);
    }
  catch (const jit_fail_exception&)
    {
      function->eraseFromParent ();
      function = 0;
      return;
    }

  finalize (names);
}

void
jit_generator::visit_anon_fcn_handle (tree_anon_fcn_handle&)
{
  fail ();
}

void
jit_generator::visit_argument_list (tree_argument_list&)
{
  fail ();
}

void
jit_generator::visit_binary_expression (tree_binary_expression& be)
{
  tree_expression *lhs = be.lhs ();
  lhs->accept (*this);
  value lhsv = value_stack.back ();
  value_stack.pop_back ();

  tree_expression *rhs = be.rhs ();
  rhs->accept (*this);
  value rhsv = value_stack.back ();
  value_stack.pop_back ();

  const jit_function::overload& ol
    = tinfo->binary_op_overload (be.op_type (), lhsv.first, rhsv.first);

  if (! ol.function)
    fail ();

  llvm::Value *result = builder.CreateCall2 (ol.function, lhsv.second,
                                             rhsv.second);
  push_value (ol.result, result);
}

void
jit_generator::visit_break_command (tree_break_command&)
{
  fail ();
}

void
jit_generator::visit_colon_expression (tree_colon_expression&)
{
  fail ();
}

void
jit_generator::visit_continue_command (tree_continue_command&)
{
  fail ();
}

void
jit_generator::visit_global_command (tree_global_command&)
{
  fail ();
}

void
jit_generator::visit_persistent_command (tree_persistent_command&)
{
  fail ();
}

void
jit_generator::visit_decl_elt (tree_decl_elt&)
{
  fail ();
}

void
jit_generator::visit_decl_init_list (tree_decl_init_list&)
{
  fail ();
}

void
jit_generator::visit_simple_for_command (tree_simple_for_command& cmd)
{
  if (is_lvalue)
    fail ();

  tree_expression *control = cmd.control_expr ();
  assert (control); // FIXME: jwe, can this be null?

  control->accept (*this);
  value over = value_stack.back ();
  value_stack.pop_back ();

  emit_simple_for (cmd, over, false);
}

void
jit_generator::visit_complex_for_command (tree_complex_for_command&)
{
  fail ();
}

void
jit_generator::visit_octave_user_script (octave_user_script&)
{
  fail ();
}

void
jit_generator::visit_octave_user_function (octave_user_function&)
{
  fail ();
}

void
jit_generator::visit_octave_user_function_header (octave_user_function&)
{
  fail ();
}

void
jit_generator::visit_octave_user_function_trailer (octave_user_function&)
{
  fail ();
}

void
jit_generator::visit_function_def (tree_function_def&)
{
  fail ();
}

void
jit_generator::visit_identifier (tree_identifier& ti)
{
  std::string name = ti.name ();
  value variable = variables[name];
  if (is_lvalue)
    {
      value_stack.push_back (variable);

      const jit_function::overload& ol = tinfo->release (variable.first);
      if (ol.function)
        {
          llvm::Value *load = builder.CreateLoad (variable.second, name);
          builder.CreateCall (ol.function, load);
        }
    }
  else
    {
      llvm::Value *load = builder.CreateLoad (variable.second, name);
      push_value (variable.first, load);

      const jit_function::overload& ol = tinfo->grab (variable.first);
      if (ol.function)
        builder.CreateCall (ol.function, load);
    }
}

void
jit_generator::visit_if_clause (tree_if_clause&)
{
  fail ();
}

void
jit_generator::visit_if_command (tree_if_command& cmd)
{
  tree_if_command_list *lst = cmd.cmd_list ();
  assert (lst);
  lst->accept (*this);
}

void
jit_generator::visit_if_command_list (tree_if_command_list& lst)
{
  llvm::LLVMContext &ctx = llvm::getGlobalContext ();
  llvm::BasicBlock *tail = llvm::BasicBlock::Create (ctx, "if_tail", function);
  std::vector<llvm::BasicBlock *> clause_entry (lst.size ());
  tree_if_command_list::iterator p;
  size_t i;
  for (p = lst.begin (), i = 0; p != lst.end (); ++p, ++i)
    {
      tree_if_clause *tic = *p;
      if (tic->is_else_clause ())
        clause_entry[i] = llvm::BasicBlock::Create (ctx, "else_body", function,
                                                    tail);
      else
        clause_entry[i] = llvm::BasicBlock::Create (ctx, "if_cond", function,
                                                    tail);
    }

  builder.CreateBr (clause_entry[0]);

  for (p = lst.begin (), i = 0; p != lst.end (); ++p, ++i)
    {
      tree_if_clause *tic = *p;
      llvm::BasicBlock *body;
      if (tic->is_else_clause ())
        body = clause_entry[i];
      else
        {
          llvm::BasicBlock *cond = clause_entry[i];
          builder.SetInsertPoint (cond);

          tree_expression *expr = tic->condition ();
          expr->accept (*this);

          // FIXME: Handle undefined case
          value condv = value_stack.back ();
          value_stack.pop_back ();

          const jit_function::overload& ol = tinfo->get_logically_true (condv.first);
          if (! ol.function)
            fail ();

          bool last = i + 1 == clause_entry.size ();
          llvm::BasicBlock *next = last ? tail : clause_entry[i + 1];
          body = llvm::BasicBlock::Create (ctx, "if_body", function, tail);

          llvm::Value *is_true = builder.CreateCall (ol.function, condv.second);
          builder.CreateCondBr (is_true, body, next);
        }

      tree_statement_list *stmt_lst = tic->commands ();
      builder.SetInsertPoint (body);
      stmt_lst->accept (*this);
      builder.CreateBr (tail);
    }

  builder.SetInsertPoint (tail);
}

void
jit_generator::visit_index_expression (tree_index_expression&)
{
  fail ();
}

void
jit_generator::visit_matrix (tree_matrix&)
{
  fail ();
}

void
jit_generator::visit_cell (tree_cell&)
{
  fail ();
}

void
jit_generator::visit_multi_assignment (tree_multi_assignment&)
{
  fail ();
}

void
jit_generator::visit_no_op_command (tree_no_op_command&)
{
  fail ();
}

void
jit_generator::visit_constant (tree_constant& tc)
{
  octave_value v = tc.rvalue1 ();
  llvm::LLVMContext& ctx = llvm::getGlobalContext ();
  if (v.is_real_scalar () && v.is_double_type ())
    {
      double dv = v.double_value ();
      llvm::Value *lv = llvm::ConstantFP::get (ctx, llvm::APFloat (dv));
      push_value (tinfo->get_scalar (), lv);
    }
  else if (v.is_range ())
    {
      Range rng = v.range_value ();
      llvm::Type *range = tinfo->get_range_llvm ();
      llvm::Type *scalar = tinfo->get_scalar_llvm ();
      llvm::Type *index = tinfo->get_index_llvm ();

      std::vector<llvm::Constant *> values (4);
      values[0] = llvm::ConstantFP::get (scalar, rng.base ());
      values[1] = llvm::ConstantFP::get (scalar, rng.limit ());
      values[2] = llvm::ConstantFP::get (scalar, rng.inc ());
      values[3] = llvm::ConstantInt::get (index, rng.nelem ());

      llvm::StructType *llvm_range = llvm::cast<llvm::StructType>(range);
      llvm::Value *lv = llvm::ConstantStruct::get (llvm_range, values);
      push_value (tinfo->get_range (), lv);
    }
  else
    fail ();
}

void
jit_generator::visit_fcn_handle (tree_fcn_handle&)
{
  fail ();
}

void
jit_generator::visit_parameter_list (tree_parameter_list&)
{
  fail ();
}

void
jit_generator::visit_postfix_expression (tree_postfix_expression&)
{
  fail ();
}

void
jit_generator::visit_prefix_expression (tree_prefix_expression&)
{
  fail ();
}

void
jit_generator::visit_return_command (tree_return_command&)
{
  fail ();
}

void
jit_generator::visit_return_list (tree_return_list&)
{
  fail ();
}

void
jit_generator::visit_simple_assignment (tree_simple_assignment& tsa)
{
  if (is_lvalue)
    fail ();

  // resolve rhs
  tree_expression *rhs = tsa.right_hand_side ();
  rhs->accept (*this);

  value rhsv = value_stack.back ();
  value_stack.pop_back ();

  // resolve lhs
  is_lvalue = true;
  tree_expression *lhs = tsa.left_hand_side ();
  lhs->accept (*this);
  is_lvalue = false;

  value lhsv = value_stack.back ();
  value_stack.pop_back ();

  // do assign, then keep rhs as the result
  builder.CreateStore (rhsv.second, lhsv.second);

  if (tsa.print_result ())
    emit_print (lhs->name (), rhsv);

  value_stack.push_back (rhsv);
}

void
jit_generator::visit_statement (tree_statement& stmt)
{
  tree_command *cmd = stmt.command ();
  tree_expression *expr = stmt.expression ();

  if (cmd)
    cmd->accept (*this);
  else
    {
      // stolen from tree_evaluator::visit_statement
      bool do_bind_ans = false;

      if (expr->is_identifier ())
        {
          tree_identifier *id = dynamic_cast<tree_identifier *> (expr);

          do_bind_ans = (! id->is_variable ());
        }
      else
        do_bind_ans = (! expr->is_assignment_expression ());

      expr->accept (*this);

      if (do_bind_ans)
        {
          value rhs = value_stack.back ();
          value ans = variables["ans"];
          if (ans.first != rhs.first)
            fail ();

          builder.CreateStore (rhs.second, ans.second);

          if (expr->print_result ())
            emit_print ("ans", rhs);
        }
      else if (expr->is_identifier () && expr->print_result ())
        {
          // FIXME: ugly hack, we need to come up with a way to pass
          // nargout to visit_identifier
          emit_print (expr->name (), value_stack.back ());
        }


      value_stack.pop_back ();
    }
}

void
jit_generator::visit_statement_list (tree_statement_list& lst)
{
  tree_statement_list::iterator iter;
  for (iter = lst.begin (); iter != lst.end (); ++iter)
    {
      tree_statement *stmt = *iter;
      assert (stmt); // FIXME: jwe can this be null?
      stmt->accept (*this);
    }
}

void
jit_generator::visit_switch_case (tree_switch_case&)
{
  fail ();
}

void
jit_generator::visit_switch_case_list (tree_switch_case_list&)
{
  fail ();
}

void
jit_generator::visit_switch_command (tree_switch_command&)
{
  fail ();
}

void
jit_generator::visit_try_catch_command (tree_try_catch_command&)
{
  fail ();
}

void
jit_generator::visit_unwind_protect_command (tree_unwind_protect_command&)
{
  fail ();
}

void
jit_generator::visit_while_command (tree_while_command&)
{
  fail ();
}

void
jit_generator::visit_do_until_command (tree_do_until_command&)
{
  fail ();
}

void
jit_generator::emit_simple_for (tree_simple_for_command& cmd, value over,
                                bool atleast_once)
{
  if (is_lvalue)
    fail ();

  jit_type *index = tinfo->get_index ();
  llvm::Value *init_index = 0;
  if (over.first == tinfo->get_range ())
    init_index = llvm::ConstantInt::get (index->to_llvm (), 0);
  else
    fail ();

  llvm::Value *llvm_index = builder.CreateAlloca (index->to_llvm (), 0, "index");
  builder.CreateStore (init_index, llvm_index);

  // FIXME: Support break
  llvm::LLVMContext &ctx = llvm::getGlobalContext ();
  llvm::BasicBlock *body = llvm::BasicBlock::Create (ctx, "for_body", function);
  llvm::BasicBlock *cond_check = llvm::BasicBlock::Create (ctx, "for_check", function);
  llvm::BasicBlock *tail = llvm::BasicBlock::Create (ctx, "for_tail", function);

  // initialize the iter from the index
  if (atleast_once)
    builder.CreateBr (body);
  else
    builder.CreateBr (cond_check);

  builder.SetInsertPoint (body);

  is_lvalue = true;
  tree_expression *lhs = cmd.left_hand_side ();
  lhs->accept (*this);
  is_lvalue = false;

  value lhsv = value_stack.back ();
  value_stack.pop_back ();

  const jit_function::overload& index_ol = tinfo->get_simple_for_index (over.first);
  llvm::Value *lindex = builder.CreateLoad (llvm_index);
  llvm::Value *llvm_iter = builder.CreateCall2 (index_ol.function, over.second, lindex);
  value iter(index_ol.result, llvm_iter);
  builder.CreateStore (iter.second, lhsv.second);

  tree_statement_list *lst = cmd.body ();
  lst->accept (*this);

  llvm::Value *one = llvm::ConstantInt::get (index->to_llvm (), 1);
  lindex = builder.CreateLoad (llvm_index);
  lindex = builder.CreateAdd (lindex, one);
  builder.CreateStore (lindex, llvm_index);
  builder.CreateBr (cond_check);

  builder.SetInsertPoint (cond_check);
  lindex = builder.CreateLoad (llvm_index);
  const jit_function::overload& check_ol = tinfo->get_simple_for_check (over.first);
  llvm::Value *cond = builder.CreateCall2 (check_ol.function, over.second, lindex);
  builder.CreateCondBr (cond, body, tail);

  builder.SetInsertPoint (tail);
}

void
jit_generator::emit_print (const std::string& name, const value& v)
{
  const jit_function::overload& ol = tinfo->print_value (v.first);
  if (! ol.function)
    fail ();

  llvm::Value *str = builder.CreateGlobalStringPtr (name);
  builder.CreateCall2 (ol.function, str, v.second);
}

void
jit_generator::initialize (const std::vector<std::string>& names,
                           const std::vector<bool>& argin,
                           const std::vector<jit_type *> types)
{
  std::vector<llvm::Type *> arg_types (names.size ());
  for (size_t i = 0; i < types.size (); ++i)
    arg_types[i] = types[i]->to_llvm_arg ();

  llvm::LLVMContext &ctx = llvm::getGlobalContext ();
  llvm::Type *tvoid = llvm::Type::getVoidTy (ctx);
  llvm::FunctionType *ft = llvm::FunctionType::get (tvoid, arg_types, false);
  function = llvm::Function::Create (ft, llvm::Function::ExternalLinkage,
                                     "foobar", module);

  // create variables and copy initial values
  llvm::BasicBlock *body = llvm::BasicBlock::Create (ctx, "body", function);
  builder.SetInsertPoint (body);
  llvm::Function::arg_iterator arg_iter = function->arg_begin();
  for (size_t i = 0; i < names.size (); ++i, ++arg_iter)
    {
      llvm::Type *vartype = types[i]->to_llvm ();
      const std::string& name = names[i];
      llvm::Value *var = builder.CreateAlloca (vartype, 0, name);
      variables[name] = value (types[i], var);

      if (argin[i] || types[i]->force_init ())
        {
          llvm::Value *loaded_arg = builder.CreateLoad (arg_iter);
          builder.CreateStore (loaded_arg, var);
        }
    }
}

void
jit_generator::finalize (const std::vector<std::string>& names)
{
  // copy computed values back into arguments
  // we use names instead of looping through variables because order is
  // important
  llvm::Function::arg_iterator arg_iter = function->arg_begin();
  for (size_t i = 0; i < names.size (); ++i, ++arg_iter)
    {
      llvm::Value *var = variables[names[i]].second;
      llvm::Value *loaded_var = builder.CreateLoad (var);
      builder.CreateStore (loaded_var, arg_iter);
    }
  builder.CreateRetVoid ();
}

// -------------------- tree_jit --------------------

tree_jit::tree_jit (void) : context (llvm::getGlobalContext ()), engine (0)
{
  llvm::InitializeNativeTarget ();
  module = new llvm::Module ("octave", context);
}

tree_jit::~tree_jit (void)
{
  delete tinfo;
}

bool
tree_jit::execute (tree_simple_for_command& cmd, const octave_value& bounds)
{
  if (! initialize ())
    return false;

  jit_type *bounds_t = tinfo->type_of (bounds);
  jit_info *jinfo = cmd.get_info (bounds_t);
  if (! jinfo)
    {
      jinfo = new jit_info (*this, cmd, bounds_t);
      cmd.stash_info (bounds_t, jinfo);
    }

  return jinfo->execute (bounds);
}

bool
tree_jit::initialize (void)
{
  if (engine)
    return true;

  // sometimes this fails pre main
  engine = llvm::ExecutionEngine::createJIT (module);

  if (! engine)
    return false;

  module_pass_manager = new llvm::PassManager ();
  module_pass_manager->add (llvm::createAlwaysInlinerPass ());

  pass_manager = new llvm::FunctionPassManager (module);
  pass_manager->add (new llvm::TargetData(*engine->getTargetData ()));
  pass_manager->add (llvm::createBasicAliasAnalysisPass ());
  pass_manager->add (llvm::createPromoteMemoryToRegisterPass ());
  pass_manager->add (llvm::createInstructionCombiningPass ());
  pass_manager->add (llvm::createReassociatePass ());
  pass_manager->add (llvm::createGVNPass ());
  pass_manager->add (llvm::createCFGSimplificationPass ());
  pass_manager->doInitialization ();

  tinfo = new jit_typeinfo (module, engine);

  return true;
}


void
tree_jit::optimize (llvm::Function *fn)
{
  module_pass_manager->run (*module);
  pass_manager->run (*fn);
}

// -------------------- jit_info --------------------
jit_info::jit_info (tree_jit& tjit, tree_simple_for_command& cmd,
                    jit_type *bounds) : tinfo (tjit.get_typeinfo ()),
                                        engine (tjit.get_engine ()),
                                        bounds_t (bounds)
{
  jit_infer infer(tinfo);

  try
    {
      infer.infer (cmd, bounds);
    }
  catch (const jit_fail_exception&)
    {
      function = 0;
      return;
    }

  types = infer.get_types ();

  jit_generator gen(tinfo, tjit.get_module (), cmd, bounds, types);
  function = gen.get_function ();

  if (function)
    {
      if (debug_print)
        {
          std::cout << "Compiled code:\n";
          std::cout << cmd.str_print_code () << std::endl;

          std::cout << "Before optimization:\n";

          llvm::raw_os_ostream os (std::cout);
          function->print (os);
        }
      llvm::verifyFunction (*function);
      tjit.optimize (function);

      if (debug_print)
        {
          std::cout << "After optimization:\n";

          llvm::raw_os_ostream os (std::cout);
          function->print (os);
        }
    }
}

bool
jit_info::execute (const octave_value& bounds) const
{
  if (! function)
    return false;

  std::vector<llvm::GenericValue> args (types.size () + 1);
  tinfo->to_generic (bounds_t, args[0], bounds);

  size_t idx;
  type_map::const_iterator iter;
  for (idx = 1, iter = types.begin (); iter != types.end (); ++iter, ++idx)
    {
      if (iter->second.first) // argin?
        {
          octave_value ov = iter->first->varval ();
          tinfo->to_generic (iter->second.second, args[idx], ov);
        }
      else
        tinfo->to_generic (iter->second.second, args[idx]);
    }

  engine->runFunction (function, args);

  for (idx = 1, iter = types.begin (); iter != types.end (); ++iter, ++idx)
    {
      octave_value result = tinfo->to_octave_value (iter->second.second, args[idx]);
      octave_value &ref = iter->first->varref ();
      ref = result;
    }

  tinfo->reset_generic ();

  return true;
}

bool
jit_info::match () const
{
  for (type_map::const_iterator iter = types.begin (); iter != types.end ();
       ++iter)
       
    {
      if (iter->second.first) // argin?
        {
          jit_type *required_type = iter->second.second;
          octave_value val = iter->first->varval ();
          jit_type *current_type = tinfo->type_of (val);

          // FIXME: should be: ! required_type->is_parent (current_type)
          if (required_type != current_type)
            return false;
        }
    }

  return true;
}
