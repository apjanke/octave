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

#include "octave.h"
#include "ov-fcn-handle.h"
#include "ov-usr-fcn.h"
#include "ov-scalar.h"
#include "pt-all.h"

// FIXME: Remove eventually
// For now we leave this in so people tell when JIT actually happens
static const bool debug_print = false;

static llvm::IRBuilder<> builder (llvm::getGlobalContext ());

static llvm::LLVMContext& context = llvm::getGlobalContext ();

jit_typeinfo *jit_typeinfo::instance;

// thrown when we should give up on JIT and interpret
class jit_fail_exception : public std::runtime_error
{
public:
  jit_fail_exception (void) : std::runtime_error ("unknown"), mknown (false) {}
  jit_fail_exception (const std::string& reason) : std::runtime_error (reason),
                                                   mknown (true)
  {}

  bool known (void) const { return mknown; }
private:
  bool mknown;
};

static void
fail (void)
{
  throw jit_fail_exception ();
}

static void
fail (const std::string& reason)
{
  throw jit_fail_exception (reason);
}

std::ostream& jit_print (std::ostream& os, jit_type *atype)
{
  if (! atype)
    return os << "null";
  return os << atype->name ();
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

extern "C" octave_base_value *
octave_jit_grab_any (octave_base_value *obv)
{
  obv->grab ();
  return obv;
}

extern "C" double
octave_jit_cast_scalar_any (octave_base_value *obv)
{
  double ret = obv->double_value ();
  obv->release ();
  return ret;
}

extern "C" octave_base_value *
octave_jit_cast_any_scalar (double value)
{
  return new octave_scalar (value);
}

// -------------------- jit_range --------------------
std::ostream&
operator<< (std::ostream& os, const jit_range& rng)
{
  return os << "Range[" << rng.base << ", " << rng.limit << ", " << rng.inc
            << ", " << rng.nelem << "]";
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

  for (size_t i  =0; i < types.size (); ++i)
    if (! types[i])
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
void
jit_typeinfo::initialize (llvm::Module *m, llvm::ExecutionEngine *e)
{
  instance = new jit_typeinfo (m, e);
}

jit_typeinfo::jit_typeinfo (llvm::Module *m, llvm::ExecutionEngine *e)
  : module (m), engine (e), next_id (0)
{
  // FIXME: We should be registering types like in octave_value_typeinfo
  ov_t = llvm::StructType::create (context, "octave_base_value");
  ov_t = ov_t->getPointerTo ();

  llvm::Type *dbl = llvm::Type::getDoubleTy (context);
  llvm::Type *bool_t = llvm::Type::getInt1Ty (context);
  llvm::Type *string_t = llvm::Type::getInt8Ty (context);
  string_t = string_t->getPointerTo ();
  llvm::Type *index_t = 0;
  switch (sizeof(octave_idx_type))
    {
    case 4:
      index_t = llvm::Type::getInt32Ty (context);
      break;
    case 8:
      index_t = llvm::Type::getInt64Ty (context);
      break;
    default:
      assert (false && "Unrecognized index type size");
    }

  llvm::StructType *range_t = llvm::StructType::create (context, "range");
  std::vector<llvm::Type *> range_contents (4, dbl);
  range_contents[3] = index_t;
  range_t->setBody (range_contents);

  // create types
  any = new_type ("any", 0, ov_t);
  scalar = new_type ("scalar", any, dbl);
  range = new_type ("range", any, range_t);
  string = new_type ("string", any, string_t);
  boolean = new_type ("bool", any, bool_t);
  index = new_type ("index", any, index_t);

  casts.resize (next_id + 1);
  identities.resize (next_id + 1, 0);

  // any with anything is an any op
  llvm::Function *fn;
  llvm::Type *binary_op_type
    = llvm::Type::getIntNTy (context, sizeof (octave_value::binary_op));
  llvm::Function *any_binary = create_function ("octave_jit_binary_any_any",
                                                any->to_llvm (), binary_op_type,
                                                any->to_llvm (), any->to_llvm ());
  engine->addGlobalMapping (any_binary,
                            reinterpret_cast<void*>(&octave_jit_binary_any_any));

  binary_ops.resize (octave_value::num_binary_ops);
  for (size_t i = 0; i < octave_value::num_binary_ops; ++i)
    {
      octave_value::binary_op op = static_cast<octave_value::binary_op> (i);
      std::string op_name = octave_value::binary_op_as_string (op);
      binary_ops[i].stash_name ("binary" + op_name);
    }

  for (int op = 0; op < octave_value::num_binary_ops; ++op)
    {
      llvm::Twine fn_name ("octave_jit_binary_any_any_");
      fn_name = fn_name + llvm::Twine (op);
      fn = create_function (fn_name, any, any, any);
      llvm::BasicBlock *block = llvm::BasicBlock::Create (context, "body", fn);
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

  llvm::Type *void_t = llvm::Type::getVoidTy (context);

  // grab any
  fn = create_function ("octave_jit_grab_any", any, any);
                        
  engine->addGlobalMapping (fn, reinterpret_cast<void*>(&octave_jit_grab_any));
  grab_fn.add_overload (fn, false, any, any);
  grab_fn.stash_name ("grab");

  // grab scalar
  fn = create_identity (scalar);
  grab_fn.add_overload (fn, false, scalar, scalar);

  // grab index
  fn = create_identity (index);
  grab_fn.add_overload (fn, false, index, index);

  // release any
  fn = create_function ("octave_jit_release_any", void_t, any->to_llvm ());
  engine->addGlobalMapping (fn, reinterpret_cast<void*>(&octave_jit_release_any));
  release_fn.add_overload (fn, false, 0, any);
  release_fn.stash_name ("release");

  // release scalar
  fn = create_identity (scalar);
  release_fn.add_overload (fn, false, 0, scalar);

  // release index
  fn = create_identity (index);
  release_fn.add_overload (fn, false, 0, index);

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

  // now for binary index operators
  add_binary_op (index, octave_value::op_add, llvm::Instruction::Add);

  // now for printing functions
  print_fn.stash_name ("print");
  add_print (any, reinterpret_cast<void*> (&octave_jit_print_any));
  add_print (scalar, reinterpret_cast<void*> (&octave_jit_print_double));

  // initialize for loop
  for_init_fn.stash_name ("for_init");

  fn = create_function ("octave_jit_for_range_init", index, range);
  llvm::BasicBlock *body = llvm::BasicBlock::Create (context, "body", fn); 
  builder.SetInsertPoint (body);
  {
    llvm::Value *zero = llvm::ConstantInt::get (index_t, 0);
    builder.CreateRet (zero);
  }
  llvm::verifyFunction (*fn);
  for_init_fn.add_overload (fn, false, index, range);

  // bounds check for for loop
  for_check_fn.stash_name ("for_check");

  fn = create_function ("octave_jit_for_range_check", boolean, range, index);
  body = llvm::BasicBlock::Create (context, "body", fn);
  builder.SetInsertPoint (body);
  {
    llvm::Value *nelem
      = builder.CreateExtractValue (fn->arg_begin (), 3);
    llvm::Value *idx = ++fn->arg_begin ();
    llvm::Value *ret = builder.CreateICmpULT (idx, nelem);
    builder.CreateRet (ret);
  }
  llvm::verifyFunction (*fn);
  for_check_fn.add_overload (fn, false, boolean, range, index);

  // index variabe for for loop
  for_index_fn.stash_name ("for_index");

  fn = create_function ("octave_jit_for_range_idx", scalar, range, index);
  body = llvm::BasicBlock::Create (context, "body", fn);
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
  for_index_fn.add_overload (fn, false, scalar, range, index);

  // logically true
  // FIXME: Check for NaN
  fn = create_function ("octave_logically_true_scalar", boolean, scalar);
  body = llvm::BasicBlock::Create (context, "body", fn);
  builder.SetInsertPoint (body);
  {
    llvm::Value *zero = llvm::ConstantFP::get (scalar->to_llvm (), 0);
    llvm::Value *ret = builder.CreateFCmpUNE (fn->arg_begin (), zero);
    builder.CreateRet (ret);
  }
  llvm::verifyFunction (*fn);
  logically_true.add_overload (fn, true, boolean, scalar);

  fn = create_function ("octave_logically_true_bool", boolean, boolean);
  body = llvm::BasicBlock::Create (context, "body", fn);
  builder.SetInsertPoint (body);
  builder.CreateRet (fn->arg_begin ());
  llvm::verifyFunction (*fn);
  logically_true.add_overload (fn, false, boolean, boolean);
  logically_true.stash_name ("logically_true");

  casts[any->type_id ()].stash_name ("(any)");
  casts[scalar->type_id ()].stash_name ("(scalar)");

  // cast any <- scalar
  fn = create_function ("octave_jit_cast_any_scalar", any, scalar);
  engine->addGlobalMapping (fn, reinterpret_cast<void*> (&octave_jit_cast_any_scalar));
  casts[any->type_id ()].add_overload (fn, false, any, scalar);

  // cast scalar <- any
  fn = create_function ("octave_jit_cast_scalar_any", scalar, any);
  engine->addGlobalMapping (fn, reinterpret_cast<void*> (&octave_jit_cast_scalar_any));
  casts[scalar->type_id ()].add_overload (fn, false, scalar, any);

  // cast any <- any
  fn = create_identity (any);
  casts[any->type_id ()].add_overload (fn, false, any, any);

  // cast scalar <- scalar
  fn = create_identity (scalar);
  casts[scalar->type_id ()].add_overload (fn, false, scalar, scalar);
}

void
jit_typeinfo::add_print (jit_type *ty, void *call)
{
  std::stringstream name;
  name << "octave_jit_print_" << ty->name ();

  llvm::Type *void_t = llvm::Type::getVoidTy (context);
  llvm::Function *fn = create_function (name.str (), void_t,
                                        llvm::Type::getInt8PtrTy (context),
                                        ty->to_llvm ());
  engine->addGlobalMapping (fn, call);

  jit_function::overload ol (fn, false, 0, string, ty);
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

  llvm::Function *fn = create_function (fname.str (), ty, ty, ty);
  llvm::BasicBlock *block = llvm::BasicBlock::Create (context, "body", fn);
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

  llvm::Function *fn = create_function (fname.str (), boolean, ty, ty);
  llvm::BasicBlock *block = llvm::BasicBlock::Create (context, "body", fn);
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

  llvm::Function *fn = create_function (fname.str (), boolean, ty, ty);
  llvm::BasicBlock *block = llvm::BasicBlock::Create (context, "body", fn);
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

llvm::Function *
jit_typeinfo::create_identity (jit_type *type)
{
  size_t id = type->type_id ();
  if (id >= identities.size ())
    identities.resize (id + 1, 0);

  if (! identities[id])
    {
      llvm::Function *fn = create_function ("id", type, type);
      llvm::BasicBlock *body = llvm::BasicBlock::Create (context, "body", fn);
      builder.SetInsertPoint (body);
      builder.CreateRet (fn->arg_begin ());
      llvm::verifyFunction (*fn);
      identities[id] = fn;
    }

  return identities[id];
}

jit_type *
jit_typeinfo::do_type_of (const octave_value &ov) const
{
  if (ov.is_function ())
    return 0;

  if (ov.is_double_type () && ov.is_real_scalar ())
    return get_scalar ();

  if (ov.is_range ())
    return get_range ();

  return get_any ();
}

jit_type*
jit_typeinfo::new_type (const std::string& name, jit_type *parent,
                        llvm::Type *llvm_type)
{
  jit_type *ret = new jit_type (name, parent, llvm_type, next_id++);
  id_to_type.push_back (ret);
  return ret;
}

// -------------------- jit_use --------------------
jit_block *
jit_use::user_parent (void) const
{
  return muser->parent ();
}

// -------------------- jit_value --------------------
jit_value::~jit_value (void)
{
  replace_with (0);
}

void
jit_value::replace_with (jit_value *value)
{
  while (use_head)
    {
      jit_instruction *user = use_head->user ();
      size_t idx = use_head->index ();
      if (idx < user->argument_count ())
        user->stash_argument (idx, value);
      else
        user->stash_tag (0);
    }
}

#define JIT_METH(clname)                                \
  void                                                  \
  jit_ ## clname::accept (jit_ir_walker& walker)        \
  {                                                     \
    walker.visit (*this);                               \
  }

JIT_VISIT_IR_NOTEMPLATE
#undef JIT_METH

std::ostream&
operator<< (std::ostream& os, const jit_value& value)
{
  return value.short_print (os);
}

// -------------------- jit_instruction --------------------
void
jit_instruction::push_variable (void)
{
  if (tag ())
    tag ()->push (this);
}

void
jit_instruction::pop_variable (void)
{
  if (tag ())
    tag ()->pop ();
}

llvm::BasicBlock *
jit_instruction::parent_llvm (void) const
{
  return mparent->to_llvm ();
}

std::ostream&
jit_instruction::short_print (std::ostream& os) const
{
  if (type ())
    jit_print (os, type ()) << ": ";

  if (tag ())
    os << tag ()->name () << "." << id;
  else
    os << "#" << id;
  return os;
}

jit_variable *
jit_instruction::tag (void) const
{
  return reinterpret_cast<jit_variable *> (mtag.value ());
}

void
jit_instruction::stash_tag (jit_variable *atag)
{
  mtag.stash_value (atag, this);
}

// -------------------- jit_block --------------------
jit_instruction *
jit_block::prepend (jit_instruction *instr)
{
  instructions.push_front (instr);
  instr->stash_parent (this);
  return instr;
}

jit_instruction *
jit_block::append (jit_instruction *instr)
{
  instructions.push_back (instr);
  instr->stash_parent (this);
  return instr;
}

jit_instruction *
jit_block::insert_before (iterator loc, jit_instruction *instr)
{
  instructions.insert (loc, instr);
  instr->stash_parent (this);
  return instr;
}

jit_instruction *
jit_block::insert_after (iterator loc, jit_instruction *instr)
{
  ++loc;
  instructions.insert (loc, instr);
  instr->stash_parent (this);
  return instr;
}

jit_terminator *
jit_block::terminator (void) const
{
  if (instructions.empty ())
    return 0;

  jit_instruction *last = instructions.back ();
  return dynamic_cast<jit_terminator *> (last);
}

jit_block *
jit_block::pred (size_t idx) const
{
  // FIXME: Make this O(1)
  
  // here we get the use in backwards order. This means we preserve phi
  // information when new blocks are added
  assert (idx < use_count ());
  jit_use *use;
  size_t real_idx = use_count () - idx - 1;
  size_t i;
  for (use = first_use (), i = 0; use && i < real_idx; ++i,
         use = use->next ());
    
  return use->user_parent ();
}

size_t
jit_block::pred_index (jit_block *apred) const
{
  for (size_t i = 0; i < pred_count (); ++i)
    if (pred (i) == apred)
      return i;

  fail ("No such predecessor");
  return 0; // silly compiler, why you warn?
}

void
jit_block::create_merge (llvm::Function *inside, size_t pred_idx)
{
  mpred_llvm.resize (pred_count ());

  jit_block *ipred = pred (pred_idx);
  if (! mpred_llvm[pred_idx] && ipred->pred_count () > 1)
    {
      llvm::BasicBlock *merge;
      merge = llvm::BasicBlock::Create (context, "phi_merge", inside,
                                        to_llvm ());
          
      // fix the predecessor jump if it has been created
      jit_terminator *jterm = pred_terminator (pred_idx);
      if (jterm->has_llvm ())
        {
          llvm::Value *term = jterm->to_llvm ();
          llvm::TerminatorInst *branch = llvm::cast<llvm::TerminatorInst> (term);
          for (size_t i = 0; i < branch->getNumSuccessors (); ++i)
            {
              if (branch->getSuccessor (i) == to_llvm ())
                branch->setSuccessor (i, merge);
            }
        }

      llvm::IRBuilder<> temp (merge);
      temp.CreateBr (to_llvm ());
      mpred_llvm[pred_idx] = merge;
    }
}

jit_block *
jit_block::succ (size_t i) const
{
  jit_terminator *term = terminator ();
  return term->sucessor (i);
}

size_t
jit_block::succ_count (void) const
{
  jit_terminator *term = terminator ();
  return term ? term->sucessor_count () : 0;
}

llvm::BasicBlock *
jit_block::to_llvm (void) const
{
  return llvm::cast<llvm::BasicBlock> (llvm_value);
}

std::ostream&
jit_block::print_dom (std::ostream& os) const
{
  short_print (os);
  os << ":\n";
  os << "  mid: " << mid << std::endl;
  os << "  pred: ";
  for (size_t i = 0; i < pred_count (); ++i)
    os << *pred (i) << " ";
  os << std::endl;

  os << "  succ: ";
  for (size_t i = 0; i < succ_count (); ++i)
    os << *succ (i) << " ";
  os << std::endl;

  os << "  idom: ";
  if (idom)
    os << *idom;
  else
    os << "NULL";
  os << std::endl;
  os << "  df: ";
  for (df_iterator iter = df_begin (); iter != df_end (); ++iter)
    os << **iter << " ";
  os << std::endl;

  os << "  dom_succ: ";
  for (size_t i = 0; i < dom_succ.size (); ++i)
    os << *dom_succ[i] << " ";

  return os << std::endl;
}

void
jit_block::compute_df (size_t visit_count)
{
  if (mvisit_count > visit_count)
    return;
  ++mvisit_count;

  if (pred_count () >= 2)
    {
      for (size_t i = 0; i < pred_count (); ++i)
        {
          jit_block *runner = pred (i);
          while (runner != idom)
            {
              runner->mdf.insert (this);
              runner = runner->idom;
            }
        }
    }

  for (size_t i = 0; i < succ_count (); ++i)
    succ (i)->compute_df (visit_count);
}

bool
jit_block::update_idom (size_t visit_count)
{
  if (mvisit_count > visit_count)
    return false;
  ++mvisit_count;

  if (! pred_count ())
    return false;

  bool changed = false;
  for (size_t i = 0; i < pred_count (); ++i)
    changed = pred (i)->update_idom (visit_count) || changed;

  jit_block *new_idom = pred (0);
  for (size_t i = 1; i < pred_count (); ++i)
    {
      jit_block *pidom = pred (i)->idom;
      if (! new_idom)
        new_idom = pidom;
      else if (pidom)
        new_idom = pidom->idom_intersect (new_idom);
    }

  if (idom != new_idom)
    {
      idom = new_idom;
      return true;
    }

  return changed;
}

void
jit_block::finish_phi (jit_block *apred)
{
  size_t pred_idx = pred_index (apred);
  for (iterator iter = begin (); iter != end ()
         && dynamic_cast<jit_phi *> (*iter); ++iter)
    {
      jit_instruction *phi = *iter;
      jit_variable *var = phi->tag ();
      phi->stash_argument (pred_idx, var->top ());
    }
}

void
jit_block::do_construct_ssa (jit_convert& convert, size_t visit_count)
{
  if (mvisit_count > visit_count)
    return;
  ++mvisit_count;

  for (iterator iter = begin (); iter != end (); ++iter)
    {
      jit_instruction *instr = *iter;
      bool isphi = dynamic_cast<jit_phi *> (instr);

      if (! isphi)
        {
          for (size_t i = 0; i < instr->argument_count (); ++i)
            {
              jit_variable *var;
              var = dynamic_cast<jit_variable *> (instr->argument (i));
              if (var)
                instr->stash_argument (i, var->top ());
            }

          // FIXME: Remove need for jit_store_argument dynamic cast
          jit_variable *tag = instr->tag ();
          if (tag && tag->has_top ()
              && ! dynamic_cast<jit_store_argument *> (instr))
            {
              jit_call *rel = convert.create<jit_call> (jit_typeinfo::release,
                                                        tag->top ());
              insert_after (iter, rel);
              ++iter;
            }
        }

      instr->push_variable ();
    }

  for (size_t i = 0; i < succ_count (); ++i)
    succ (i)->finish_phi (this);

  for (size_t i = 0; i < dom_succ.size (); ++i)
    dom_succ[i]->do_construct_ssa (convert, visit_count);

  for (iterator iter = begin (); iter != end (); ++iter)
    {
      jit_instruction *instr = *iter;
      instr->pop_variable ();
    }
}

void
jit_block::create_dom_tree (size_t visit_count)
{
  if (mvisit_count > visit_count)
    return;
  ++mvisit_count;

  if (idom != this)
    idom->dom_succ.push_back (this);

  for (size_t i = 0; i < succ_count (); ++i)
    succ (i)->create_dom_tree (visit_count);
}

jit_block *
jit_block::idom_intersect (jit_block *b)
{
  jit_block *i = this;
  jit_block *j = b;

  while (i != j)
    {
      while (i->id () > j->id ())
        i = i->idom;

      while (j->id () > i->id ())
        j = j->idom;
    }

  return i;
}

// -------------------- jit_call --------------------
bool
jit_call::infer (void)
{
  // FIXME: explain algorithm
  for (size_t i = 0; i < argument_count (); ++i)
    {
      already_infered[i] = argument_type (i);
      if (! already_infered[i])
        return false;
    }

  jit_type *infered = mfunction.get_result (already_infered);
  if (! infered && use_count ())
    {
      std::stringstream ss;
      ss << "Missing overload in type inference for ";
      print (ss, 0);
      fail (ss.str ());
    }

  if (infered != type ())
    {
      stash_type (infered);
      return true;
    }

  return false;
}

// -------------------- jit_convert --------------------
jit_convert::jit_convert (llvm::Module *module, tree &tee)
  : iterator_count (0), breaking (false)
{
  jit_instruction::reset_ids ();

  entry_block = create<jit_block> ("body");
  blocks.push_back (entry_block);
  block = entry_block;
  visit (tee);

  // FIXME: Remove if we no longer only compile loops
  assert (! breaking);
  assert (breaks.empty ());
  assert (continues.empty ());

  jit_block *final_block = block;
  for (vmap_t::iterator iter = vmap.begin (); iter != vmap.end (); ++iter)
       
    {
      jit_variable *var = iter->second;
      const std::string& name = var->name ();
      if (name.size () && name[0] != '#')
        final_block->append (create<jit_store_argument> (var));
    }

  construct_ssa (final_block);

  // initialize the worklist to instructions derived from constants
  for (std::list<jit_value *>::iterator iter = constants.begin ();
       iter != constants.end (); ++iter)
    append_users (*iter);

  if (debug_print)
      print_blocks ("octave jit ir");

  // FIXME: Describe algorithm here
  while (worklist.size ())
    {
      jit_instruction *next = worklist.front ();
      worklist.pop_front ();

      if (next->infer ())
        append_users (next);
    }

  if (debug_print)
    {
      std::cout << "-------------------- Compiling tree --------------------\n";
      std::cout << tee.str_print_code () << std::endl;
      print_blocks ("octave jit ir");
    }

  // for now just init arguments from entry, later we will have to do something
  // more interesting
  for (jit_block::iterator iter = entry_block->begin ();
       iter != entry_block->end (); ++iter)
    {
      if (jit_extract_argument *extract = dynamic_cast<jit_extract_argument *> (*iter))
        arguments.push_back (std::make_pair (extract->name (), true));
    }

  convert_llvm to_llvm;
  function = to_llvm.convert (module, arguments, blocks);

  if (debug_print)
    {
      std::cout << "-------------------- llvm ir --------------------";
      llvm::raw_os_ostream llvm_cout (std::cout);
      function->print (llvm_cout);
      std::cout << std::endl;
      llvm::verifyFunction (*function);
    }
}

jit_convert::~jit_convert (void)
{
  for (std::list<jit_value *>::iterator iter = all_values.begin ();
       iter != all_values.end (); ++iter)
    delete *iter;
}

void
jit_convert::visit_anon_fcn_handle (tree_anon_fcn_handle&)
{
  fail ();
}

void
jit_convert::visit_argument_list (tree_argument_list&)
{
  fail ();
}

void
jit_convert::visit_binary_expression (tree_binary_expression& be)
{
  if (be.op_type () >= octave_value::num_binary_ops)
    // this is the case for bool_or and bool_and
    fail ();

  tree_expression *lhs = be.lhs ();
  jit_value *lhsv = visit (lhs);

  tree_expression *rhs = be.rhs ();
  jit_value *rhsv = visit (rhs);

  const jit_function& fn = jit_typeinfo::binary_op (be.op_type ());
  result = block->append (create<jit_call> (fn, lhsv, rhsv));
}

void
jit_convert::visit_break_command (tree_break_command&)
{
  breaks.push_back (block);
  breaking = true;
}

void
jit_convert::visit_colon_expression (tree_colon_expression&)
{
  fail ();
}

void
jit_convert::visit_continue_command (tree_continue_command&)
{
  continues.push_back (block);
  breaking = true;
}

void
jit_convert::visit_global_command (tree_global_command&)
{
  fail ();
}

void
jit_convert::visit_persistent_command (tree_persistent_command&)
{
  fail ();
}

void
jit_convert::visit_decl_elt (tree_decl_elt&)
{
  fail ();
}

void
jit_convert::visit_decl_init_list (tree_decl_init_list&)
{
  fail ();
}

void
jit_convert::visit_simple_for_command (tree_simple_for_command& cmd)
{
  // how a for statement is compiled. Note we do an initial check
  // to see if the loop will run atleast once. This allows us to get
  // better type inference bounds on variables defined and used only
  // inside the for loop (e.g. the index variable)

  // If we are a nested for loop we need to store the previous breaks
  assert (! breaking);
  unwind_protect prot;
  prot.protect_var (breaks);
  prot.protect_var (continues);
  prot.protect_var (breaking);
  breaks.clear ();

  // FIXME: one of these days we will introduce proper lvalues...
  tree_identifier *lhs = dynamic_cast<tree_identifier *>(cmd.left_hand_side ());
  if (! lhs)
    fail ();
  std::string lhs_name = lhs->name ();

  // we need a variable for our iterator, because it is used in multiple blocks
  std::stringstream ss;
  ss << "#iter" << iterator_count++;
  std::string iter_name = ss.str ();
  jit_variable *iterator = create<jit_variable> (iter_name);
  vmap[iter_name] = iterator;

  jit_block *body = create<jit_block> ("for_body");
  blocks.push_back (body);

  jit_block *tail = create<jit_block> ("for_tail");

  // do control expression, iter init, and condition check in prev_block (block)
  jit_value *control = visit (cmd.control_expr ());
  jit_call *init_iter = create<jit_call> (jit_typeinfo::for_init, control);
  init_iter->stash_tag (iterator);
  block->append (init_iter);
  
  jit_value *check = block->append (create<jit_call> (jit_typeinfo::for_check,
                                                      control, iterator));
  block->append (create<jit_cond_break> (check, body, tail));
  block = body;

  // compute the syntactical iterator
  jit_call *idx_rhs = create<jit_call> (jit_typeinfo::for_index, control, iterator);
  block->append (idx_rhs);
  do_assign (lhs_name, idx_rhs, false);
  
  // do loop
  tree_statement_list *pt_body = cmd.body ();
  pt_body->accept (*this);

  if (breaking && continues.empty ())
    {
      // WTF are you doing user? Every branch was a continue, why did you have
      // a loop??? Users are silly people...
      finish_breaks (tail, breaks);
      blocks.push_back (tail);
      block = tail;
      return;
    }

  // check our condition, continues jump to this block
  jit_block *check_block = create<jit_block> ("for_check");
  blocks.push_back (check_block);

  if (! breaking)
    block->append (create<jit_break> (check_block));
  finish_breaks (check_block, continues);

  block = check_block;
  const jit_function& add_fn = jit_typeinfo::binary_op (octave_value::op_add);
  jit_instruction *one = create<jit_const_index> (1);
  block->append (one);

  jit_call *iter_inc = create<jit_call> (add_fn, iterator, one);
  iter_inc->stash_tag (iterator);
  block->append (iter_inc);
  check = block->append (create<jit_call> (jit_typeinfo::for_check, control,
                                           iterator));
  block->append (create<jit_cond_break> (check, body, tail));

  // breaks will go to our tail
  blocks.push_back (tail);
  finish_breaks (tail, breaks);
  block = tail;
}

void
jit_convert::visit_complex_for_command (tree_complex_for_command&)
{
  fail ();
}

void
jit_convert::visit_octave_user_script (octave_user_script&)
{
  fail ();
}

void
jit_convert::visit_octave_user_function (octave_user_function&)
{
  fail ();
}

void
jit_convert::visit_octave_user_function_header (octave_user_function&)
{
  fail ();
}

void
jit_convert::visit_octave_user_function_trailer (octave_user_function&)
{
  fail ();
}

void
jit_convert::visit_function_def (tree_function_def&)
{
  fail ();
}

void
jit_convert::visit_identifier (tree_identifier& ti)
{
  const jit_function& fn = jit_typeinfo::grab ();
  jit_value *decl = get_variable (ti.name ());
  result = block->append (create<jit_call> (fn, decl));
}

void
jit_convert::visit_if_clause (tree_if_clause&)
{
  fail ();
}

void
jit_convert::visit_if_command (tree_if_command& cmd)
{
  tree_if_command_list *lst = cmd.cmd_list ();
  assert (lst); // jwe: Can this be null?
  lst->accept (*this);
}

void
jit_convert::visit_if_command_list (tree_if_command_list& lst)
{
  // Example code:
  // if a == 1
  //  c = c + 1;
  // elseif b == 1
  //  c = c + 2;
  // else
  //  c = c + 3;
  // endif

  // ********************
  // FIXME: Documentation no longer reflects current version
  // ********************

  // Generates:
  // prev_block0: % pred - ?
  //   #temp.0 = call binary== (a.0, 1)
  //   cond_break #temp.0, if_body1, ifelse_cond2
  // if_body1:
  //   c.1 = call binary+ (c.0, 1)
  //   break if_tail5
  // ifelse_cond2:
  //   #temp.1 = call binary== (b.0, 1)
  //   cond_break #temp.1, ifelse_body3, else4
  // ifelse_body3:
  //   c.2 = call binary+ (c.0, 2)
  //   break if_tail5
  // else4:
  //   c.3 = call binary+ (c.0, 3)
  //   break if_tail5
  // if_tail5:
  //   c.4 = phi | if_body1 -> c.1
  //             | ifelse_body3 -> c.2
  //             | else4 -> c.3


  tree_if_clause *last = lst.back ();
  size_t last_else = static_cast<size_t> (last->is_else_clause ());

  // entry_blocks represents the block you need to enter in order to execute
  // the condition check for the ith clause. For the else, it is simple the
  // else body. If there is no else body, then it is padded with the tail
  std::vector<jit_block *> entry_blocks (lst.size () + 1 - last_else);
  std::vector<jit_block *> branch_blocks (lst.size (), 0); // final blocks
  entry_blocks[0] = block;

  // we need to construct blocks first, because they have jumps to eachother
  tree_if_command_list::iterator iter = lst.begin ();
  ++iter;
  for (size_t i = 1; iter != lst.end (); ++iter, ++i)
    {
      tree_if_clause *tic = *iter;
      if (tic->is_else_clause ())
        entry_blocks[i] = create<jit_block> ("else");
      else
        entry_blocks[i] = create<jit_block> ("ifelse_cond");
    }

  jit_block *tail = create<jit_block> ("if_tail");
  if (! last_else)
    entry_blocks[entry_blocks.size () - 1] = tail;

  size_t num_incomming = 0; // number of incomming blocks to our tail
  iter = lst.begin ();
  for (size_t i = 0; iter != lst.end (); ++iter, ++i)
    {
      tree_if_clause *tic = *iter;
      block = entry_blocks[i];
      assert (block);

      if (i) // the first block is prev_block, so it has already been added
        blocks.push_back (entry_blocks[i]);

      if (! tic->is_else_clause ())
        {
          tree_expression *expr = tic->condition ();
          jit_value *cond = visit (expr);

          jit_block *body = create<jit_block> (i == 0 ? "if_body" : "ifelse_body");
          blocks.push_back (body);

          jit_instruction *br = create<jit_cond_break> (cond, body,
                                                        entry_blocks[i + 1]);
          block->append (br);
          block = body;
        }

      tree_statement_list *stmt_lst = tic->commands ();
      assert (stmt_lst); // jwe: Can this be null?
      stmt_lst->accept (*this);

      if (breaking)
        breaking = false;
      else
        {
          ++num_incomming;
          block->append (create<jit_break> (tail));
        }
    }

  if (num_incomming || ! last_else)
    {
      blocks.push_back (tail);
      block = tail;
    }
  else
    // every branch broke, so we don't have a tail
    breaking = true;
}

void
jit_convert::visit_index_expression (tree_index_expression&)
{
  fail ();
}

void
jit_convert::visit_matrix (tree_matrix&)
{
  fail ();
}

void
jit_convert::visit_cell (tree_cell&)
{
  fail ();
}

void
jit_convert::visit_multi_assignment (tree_multi_assignment&)
{
  fail ();
}

void
jit_convert::visit_no_op_command (tree_no_op_command&)
{
  fail ();
}

void
jit_convert::visit_constant (tree_constant& tc)
{
  octave_value v = tc.rvalue1 ();
  if (v.is_real_scalar () && v.is_double_type ())
    {
      double dv = v.double_value ();
      result = create<jit_const_scalar> (dv);
    }
  else if (v.is_range ())
    {
      Range rv = v.range_value ();
      result = create<jit_const_range> (rv);
    }
  else
    fail ("Unknown constant");

  block->append (result);
}

void
jit_convert::visit_fcn_handle (tree_fcn_handle&)
{
  fail ();
}

void
jit_convert::visit_parameter_list (tree_parameter_list&)
{
  fail ();
}

void
jit_convert::visit_postfix_expression (tree_postfix_expression&)
{
  fail ();
}

void
jit_convert::visit_prefix_expression (tree_prefix_expression&)
{
  fail ();
}

void
jit_convert::visit_return_command (tree_return_command&)
{
  fail ();
}

void
jit_convert::visit_return_list (tree_return_list&)
{
  fail ();
}

void
jit_convert::visit_simple_assignment (tree_simple_assignment& tsa)
{
  // resolve rhs
  tree_expression *rhs = tsa.right_hand_side ();
  jit_instruction *rhsv = visit (rhs);

  // resolve lhs
  tree_expression *lhs = tsa.left_hand_side ();
  if (! lhs->is_identifier ())
    fail ();

  std::string lhs_name = lhs->name ();
  result = do_assign (lhs_name, rhsv, tsa.print_result ());
}

void
jit_convert::visit_statement (tree_statement& stmt)
{
  tree_command *cmd = stmt.command ();
  tree_expression *expr = stmt.expression ();

  if (cmd)
    visit (cmd);
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

      jit_instruction *expr_result = visit (expr);

      if (do_bind_ans)
        do_assign ("ans", expr_result, expr->print_result ());
      else if (expr->is_identifier () && expr->print_result ())
        {
          // FIXME: ugly hack, we need to come up with a way to pass
          // nargout to visit_identifier
          const jit_function& fn = jit_typeinfo::print_value ();
          jit_const_string *name = create<jit_const_string> (expr->name ());
          block->append (create<jit_call> (fn, name, expr_result));
        }
    }
}

void
jit_convert::visit_statement_list (tree_statement_list& lst)
{
  for (tree_statement_list::iterator iter = lst.begin (); iter != lst.end();
       ++iter)
    {
      tree_statement *elt = *iter;
      // jwe: Can this ever be null?
      assert (elt);
      elt->accept (*this);

      if (breaking)
        break;
    }
}

void
jit_convert::visit_switch_case (tree_switch_case&)
{
  fail ();
}

void
jit_convert::visit_switch_case_list (tree_switch_case_list&)
{
  fail ();
}

void
jit_convert::visit_switch_command (tree_switch_command&)
{
  fail ();
}

void
jit_convert::visit_try_catch_command (tree_try_catch_command&)
{
  fail ();
}

void
jit_convert::visit_unwind_protect_command (tree_unwind_protect_command&)
{
  fail ();
}

void
jit_convert::visit_while_command (tree_while_command&)
{
  fail ();
}

void
jit_convert::visit_do_until_command (tree_do_until_command&)
{
  fail ();
}

jit_variable *
jit_convert::get_variable (const std::string& vname)
{
  vmap_t::iterator iter;
  iter = vmap.find (vname);
  if (iter != vmap.end ())
    return iter->second;

  jit_variable *var = create<jit_variable> (vname);
  octave_value val = symbol_table::find (vname);
  jit_type *type = jit_typeinfo::type_of (val);
  jit_extract_argument *extract;
  extract = create<jit_extract_argument> (type, var);
  entry_block->prepend (extract);

  return vmap[vname] = var;
}

jit_instruction *
jit_convert::do_assign (const std::string& lhs, jit_instruction *rhs,
                        bool print)
{
  jit_variable *var = get_variable (lhs);
  rhs->stash_tag (var);

  if (print)
    {
      const jit_function& print_fn = jit_typeinfo::print_value ();
      jit_const_string *name = create<jit_const_string> (lhs);
      block->append (create<jit_call> (print_fn, name, var));
    }

  return rhs;
}

jit_instruction *
jit_convert::visit (tree& tee)
{
  result = 0;
  tee.accept (*this);

  jit_instruction *ret = result;
  result = 0;
  return ret;
}

void
jit_convert::construct_ssa (jit_block *final_block)
{
  final_block->label ();
  entry_block->compute_idom (final_block);
  entry_block->compute_df ();
  entry_block->create_dom_tree ();

  // insert phi nodes where needed
  for (vmap_t::iterator iter = vmap.begin (); iter != vmap.end (); ++iter)
    {
      jit_block::df_set visited, added_phi;
      std::list<jit_block *> ssa_worklist;
      iter->second->use_blocks (visited);
      ssa_worklist.insert (ssa_worklist.begin (), visited.begin (), visited.end ());

      while (ssa_worklist.size ())
        {
          jit_block *b = ssa_worklist.front ();
          ssa_worklist.pop_front ();

          for (jit_block::df_iterator diter = b->df_begin ();
               diter != b->df_end (); ++diter)
            {
              jit_block *dblock = *diter;
              if (! added_phi.count (dblock))
                {
                  jit_phi *phi = create<jit_phi> (iter->second,
                                                  dblock->pred_count ());
                  dblock->prepend (phi);
                  added_phi.insert (dblock);
                }

              if (! visited.count (dblock))
                {
                  ssa_worklist.push_back (dblock);
                  visited.insert (dblock);
                }
            }
        }
    }

  entry_block->construct_ssa (*this);
}

void
jit_convert::finish_breaks (jit_block *dest, const break_list& lst)
{
  for (break_list::const_iterator iter = lst.begin (); iter != lst.end ();
       ++iter)
    {
      jit_block *b = *iter;
      b->append (create<jit_break> (dest));
    }
}

// -------------------- jit_convert::convert_llvm --------------------
llvm::Function *
jit_convert::convert_llvm::convert (llvm::Module *module,
                                    const std::vector<std::pair< std::string, bool> >& args,
                                    const std::list<jit_block *>& blocks)
{
  jit_type *any = jit_typeinfo::get_any ();

  // argument is an array of octave_base_value*, or octave_base_value**
  llvm::Type *arg_type = any->to_llvm (); // this is octave_base_value*
  arg_type = arg_type->getPointerTo ();
  llvm::FunctionType *ft = llvm::FunctionType::get (llvm::Type::getVoidTy (context),
                                                    arg_type, false);
  function = llvm::Function::Create (ft, llvm::Function::ExternalLinkage,
                                     "foobar", module);

  try
    {
      llvm::BasicBlock *prelude = llvm::BasicBlock::Create (context, "prelude",
                                                            function);
      builder.SetInsertPoint (prelude);

      llvm::Value *arg = function->arg_begin ();
      for (size_t i = 0; i < args.size (); ++i)
        {
          llvm::Value *loaded_arg = builder.CreateConstInBoundsGEP1_32 (arg, i);
          arguments[args[i].first] = loaded_arg;
        }

      std::list<jit_block *>::const_iterator biter;
      for (biter = blocks.begin (); biter != blocks.end (); ++biter)
        {
          jit_block *jblock = *biter;
          llvm::BasicBlock *block = llvm::BasicBlock::Create (context, jblock->name (),
                                                              function);
          jblock->stash_llvm (block);
        }

      jit_block *first = *blocks.begin ();
      builder.CreateBr (first->to_llvm ());

      // convert all instructions
      for (biter = blocks.begin (); biter != blocks.end (); ++biter)
        visit (*biter);

      // now finish phi nodes
      for (biter = blocks.begin (); biter != blocks.end (); ++biter)
        {
          jit_block& block = **biter;
          for (jit_block::iterator piter = block.begin ();
               piter != block.end () && dynamic_cast<jit_phi *> (*piter); ++piter)
            {
              // our phi nodes don't have to have the same incomming type,
              // so we do casts here
              jit_instruction *phi = *piter;
              jit_block *pblock = phi->parent ();
              llvm::PHINode *llvm_phi = llvm::cast<llvm::PHINode> (phi->to_llvm ());
              for (size_t i = 0; i < phi->argument_count (); ++i)
                {
                  llvm::BasicBlock *pred = pblock->pred_llvm (i);
                  if (phi->argument_type_llvm (i) == phi->type_llvm ())
                    {
                      llvm_phi->addIncoming (phi->argument_llvm (i), pred);
                    }
                  else
                    {
                      // add cast right before pred terminator
                      builder.SetInsertPoint (--pred->end ());

                      const jit_function::overload& ol
                        = jit_typeinfo::cast (phi->type (),
                                              phi->argument_type (i));
                      if (! ol.function)
                        {
                          std::stringstream ss;
                          ss << "No cast for phi(" << i << "): ";
                          phi->print (ss);
                          fail (ss.str ());
                        }

                      llvm::Value *casted;
                      casted = builder.CreateCall (ol.function,
                                                   phi->argument_llvm (i));
                      llvm_phi->addIncoming (casted, pred);
                    }
                }
            }
        }

      jit_block *last = blocks.back ();
      builder.SetInsertPoint (last->to_llvm ());
      builder.CreateRetVoid ();
    } catch (const jit_fail_exception& e)
    {
      function->eraseFromParent ();
      throw;
    }

  return function;
}

void
jit_convert::convert_llvm::visit (jit_const_string& cs)
{
  cs.stash_llvm (builder.CreateGlobalStringPtr (cs.value ()));
}

void
jit_convert::convert_llvm::visit (jit_const_scalar& cs)
{
  cs.stash_llvm (llvm::ConstantFP::get (cs.type_llvm (), cs.value ()));
}

void jit_convert::convert_llvm::visit (jit_const_index& ci)
{
  ci.stash_llvm (llvm::ConstantInt::get (ci.type_llvm (), ci.value ()));
}

void
jit_convert::convert_llvm::visit (jit_const_range& cr)
{
  llvm::StructType *stype = llvm::cast<llvm::StructType>(cr.type_llvm ());
  llvm::Type *dbl = jit_typeinfo::get_scalar_llvm ();
  llvm::Type *idx = jit_typeinfo::get_index_llvm ();
  const jit_range& rng = cr.value ();

  llvm::Constant *constants[4];
  constants[0] = llvm::ConstantFP::get (dbl, rng.base);
  constants[1] = llvm::ConstantFP::get (dbl, rng.limit);
  constants[2] = llvm::ConstantFP::get (dbl, rng.inc);
  constants[3] = llvm::ConstantInt::get (idx, rng.nelem);

  llvm::Value *as_llvm;
  as_llvm = llvm::ConstantStruct::get (stype,
                                       llvm::makeArrayRef (constants, 4));
  cr.stash_llvm (as_llvm);
}

void
jit_convert::convert_llvm::visit (jit_block& b)
{
  llvm::BasicBlock *block = b.to_llvm ();
  builder.SetInsertPoint (block);
  for (jit_block::iterator iter = b.begin (); iter != b.end (); ++iter)
    visit (*iter);
}

void
jit_convert::convert_llvm::visit (jit_break& b)
{
  b.stash_llvm (builder.CreateBr (b.sucessor_llvm ()));
}

void
jit_convert::convert_llvm::visit (jit_cond_break& cb)
{
  llvm::Value *cond = cb.cond_llvm ();
  llvm::Value *br;
  br = builder.CreateCondBr (cond, cb.sucessor_llvm (0), cb.sucessor_llvm (1));
  cb.stash_llvm (br);
}

void
jit_convert::convert_llvm::visit (jit_call& call)
{
  const jit_function::overload& ol = call.overload ();
  if (! ol.function)
    fail ("No overload for: " + call.print_string ());

  std::vector<llvm::Value *> args (call.argument_count ());
  for (size_t i = 0; i < call.argument_count (); ++i)
    args[i] = call.argument_llvm (i);

  call.stash_llvm (builder.CreateCall (ol.function, args));
}

void
jit_convert::convert_llvm::visit (jit_extract_argument& extract)
{
  const jit_function::overload& ol = extract.overload ();
  if (! ol.function)
    fail ();

  llvm::Value *arg = arguments[extract.name ()];
  assert (arg);
  arg = builder.CreateLoad (arg);
  extract.stash_llvm (builder.CreateCall (ol.function, arg, extract.name ()));
}

void
jit_convert::convert_llvm::visit (jit_store_argument& store)
{
  llvm::Value *arg_value = store.result_llvm ();
  const jit_function::overload& ol = store.overload ();
  if (! ol.function)
    fail ();

  arg_value = builder.CreateCall (ol.function, arg_value);

  llvm::Value *arg = arguments[store.name ()];
  store.stash_llvm (builder.CreateStore (arg_value, arg));
}

void
jit_convert::convert_llvm::visit (jit_phi& phi)
{
  // we might not have converted all incoming branches, so we don't
  // set incomming branches now
  llvm::PHINode *node = llvm::PHINode::Create (phi.type_llvm (),
                                               phi.argument_count ());
  builder.Insert (node);
  phi.stash_llvm (node);

  jit_block *parent = phi.parent ();
  for (size_t i = 0; i < phi.argument_count (); ++i)
    if (phi.argument_type (i) != phi.type ())
      parent->create_merge (function, i);
}

void
jit_convert::convert_llvm::visit (jit_variable&)
{
  fail ("ERROR: SSA construction should remove all variables");
}

// -------------------- tree_jit --------------------

tree_jit::tree_jit (void) : module (0), engine (0)
{
}

tree_jit::~tree_jit (void)
{}

bool
tree_jit::execute (tree_simple_for_command& cmd)
{
  if (! initialize ())
    return false;

  jit_info *info = cmd.get_info ();
  if (! info || ! info->match ())
    {
      delete info;
      info = new jit_info (*this, cmd);
      cmd.stash_info (info);
    }

  return info->execute ();
}

bool
tree_jit::initialize (void)
{
  if (engine)
    return true;

  if (! module)
    {
      llvm::InitializeNativeTarget ();
      module = new llvm::Module ("octave", context);
    }

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

  jit_typeinfo::initialize (module, engine);

  return true;
}


void
tree_jit::optimize (llvm::Function *fn)
{
  module_pass_manager->run (*module);
  pass_manager->run (*fn);
}

// -------------------- jit_info --------------------
jit_info::jit_info (tree_jit& tjit, tree& tee)
  : engine (tjit.get_engine ())
{
  llvm::Function *fun = 0;
  try
    {
      jit_convert conv (tjit.get_module (), tee);
      fun = conv.get_function ();
      arguments = conv.get_arguments ();
      bounds = conv.get_bounds ();
    }
  catch (const jit_fail_exception& e)
    {
      if (debug_print && e.known ())
        std::cout << "jit fail: " << e.what () << std::endl;
    }

  if (! fun)
    {
      function = 0;
      return;
    }

  tjit.optimize (fun);

  if (debug_print)
    {
      std::cout << "-------------------- optimized llvm ir --------------------\n";
      llvm::raw_os_ostream llvm_cout (std::cout);
      fun->print (llvm_cout);
      std::cout << std::endl;
    }

  function = reinterpret_cast<jited_function>(engine->getPointerToFunction (fun));
}

bool
jit_info::execute (void) const
{
  if (! function)
    return false;

  std::vector<octave_base_value *> real_arguments (arguments.size ());
  for (size_t i = 0; i < arguments.size (); ++i)
    {
      if (arguments[i].second)
        {
          octave_value current = symbol_table::varval (arguments[i].first);
          octave_base_value *obv = current.internal_rep ();
          obv->grab ();
          real_arguments[i] = obv;
        }
    }

  function (&real_arguments[0]);

  for (size_t i = 0; i < arguments.size (); ++i)
    symbol_table::varref (arguments[i].first) = real_arguments[i];

  return true;
}

bool
jit_info::match (void) const
{
  if (! function)
    return true;

  for (size_t i = 0; i < bounds.size (); ++i)
    {
      const std::string& arg_name = bounds[i].second;
      octave_value value = symbol_table::find (arg_name);
      jit_type *type = jit_typeinfo::type_of (value);

      // FIXME: Check for a parent relationship
      if (type != bounds[i].first)
        return false;
    }

  return true;
}
