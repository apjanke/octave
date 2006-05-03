/*

Copyright (C) 1996, 1997 John W. Eaton

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, write to the Free
Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "Array-flags.h"
#include "data-conv.h"
#include "quit.h"
#include "str-vec.h"

#include "oct-obj.h"
#include "oct-stream.h"
#include "ov.h"
#include "ov-base.h"
#include "ov-bool.h"
#include "ov-bool-mat.h"
#include "ov-cell.h"
#include "ov-scalar.h"
#include "ov-re-mat.h"
#include "ov-bool-sparse.h"
#include "ov-cx-sparse.h"
#include "ov-re-sparse.h"
#include "ov-int8.h"
#include "ov-int16.h"
#include "ov-int32.h"
#include "ov-int64.h"
#include "ov-uint8.h"
#include "ov-uint16.h"
#include "ov-uint32.h"
#include "ov-uint64.h"
#include "ov-complex.h"
#include "ov-cx-mat.h"
#include "ov-ch-mat.h"
#include "ov-str-mat.h"
#include "ov-range.h"
#include "ov-struct.h"
#include "ov-streamoff.h"
#include "ov-list.h"
#include "ov-cs-list.h"
#include "ov-colon.h"
#include "ov-va-args.h"
#include "ov-builtin.h"
#include "ov-mapper.h"
#include "ov-dld-fcn.h"
#include "ov-usr-fcn.h"
#include "ov-fcn-handle.h"
#include "ov-fcn-inline.h"
#include "ov-typeinfo.h"

#include "defun.h"
#include "error.h"
#include "gripes.h"
#include "pager.h"
#include "parse.h"
#include "pr-output.h"
#include "utils.h"
#include "variables.h"

// We are likely to have a lot of octave_value objects to allocate, so
// make the grow_size large.
DEFINE_OCTAVE_ALLOCATOR2(octave_value, 1024);

// FIXME

// Octave's value type.

std::string
octave_value::unary_op_as_string (unary_op op)
{
  std::string retval;

  switch (op)
    {
    case op_not:
      retval = "!";
      break;

    case op_uplus:
      retval = "+";
      break;

    case op_uminus:
      retval = "-";
      break;

    case op_transpose:
      retval = ".'";
      break;

    case op_hermitian:
      retval = "'";
      break;

    case op_incr:
      retval = "++";
      break;

    case op_decr:
      retval = "--";
      break;

    default:
      retval = "<unknown>";
    }

  return retval;
}

std::string
octave_value::binary_op_as_string (binary_op op)
{
  std::string retval;

  switch (op)
    {
    case op_add:
      retval = "+";
      break;

    case op_sub:
      retval = "-";
      break;

    case op_mul:
      retval = "*";
      break;

    case op_div:
      retval = "/";
      break;

    case op_pow:
      retval = "^";
      break;

    case op_ldiv:
      retval = "\\";
      break;

    case op_lshift:
      retval = "<<";
      break;

    case op_rshift:
      retval = ">>";
      break;

    case op_lt:
      retval = "<";
      break;

    case op_le:
      retval = "<=";
      break;

    case op_eq:
      retval = "==";
      break;

    case op_ge:
      retval = ">=";
      break;

    case op_gt:
      retval = ">";
      break;

    case op_ne:
      retval = "!=";
      break;

    case op_el_mul:
      retval = ".*";
      break;

    case op_el_div:
      retval = "./";
      break;

    case op_el_pow:
      retval = ".^";
      break;

    case op_el_ldiv:
      retval = ".\\";
      break;

    case op_el_and:
      retval = "&";
      break;

    case op_el_or:
      retval = "|";
      break;

    case op_struct_ref:
      retval = ".";
      break;

    default:
      retval = "<unknown>";
    }

  return retval;
}

std::string
octave_value::assign_op_as_string (assign_op op)
{
  std::string retval;

  switch (op)
    {
    case op_asn_eq:
      retval = "=";
      break;

    case op_add_eq:
      retval = "+=";
      break;

    case op_sub_eq:
      retval = "-=";
      break;

    case op_mul_eq:
      retval = "*=";
      break;

    case op_div_eq:
      retval = "/=";
      break;

    case op_ldiv_eq:
      retval = "\\=";
      break;

    case op_pow_eq:
      retval = "^=";
      break;

    case op_lshift_eq:
      retval = "<<=";
      break;

    case op_rshift_eq:
      retval = ">>=";
      break;

    case op_el_mul_eq:
      retval = ".*=";
      break;

    case op_el_div_eq:
      retval = "./=";
      break;

    case op_el_ldiv_eq:
      retval = ".\\=";
      break;

    case op_el_pow_eq:
      retval = ".^=";
      break;

    case op_el_and_eq:
      retval = "&=";
      break;

    case op_el_or_eq:
      retval = "|=";
      break;

    default:
      retval = "<unknown>";
    }

  return retval;
}

octave_value::octave_value (void)
  : rep (new octave_base_value ())
{
}

octave_value::octave_value (short int i)
  : rep (new octave_scalar (i))
{
}

octave_value::octave_value (unsigned short int i)
  : rep (new octave_scalar (i))
{
}

octave_value::octave_value (int i)
  : rep (new octave_scalar (i))
{
}

octave_value::octave_value (unsigned int i)
  : rep (new octave_scalar (i))
{
}

octave_value::octave_value (long int i)
  : rep (new octave_scalar (i))
{
}

octave_value::octave_value (unsigned long int i)
  : rep (new octave_scalar (i))
{
}

#if defined (HAVE_LONG_LONG_INT)
octave_value::octave_value (long long int i)
  : rep (new octave_scalar (i))
{
}
#endif

#if defined (HAVE_UNSIGNED_LONG_LONG_INT)
octave_value::octave_value (unsigned long long int i)
  : rep (new octave_scalar (i))
{
}
#endif

octave_value::octave_value (octave_time t)
  : rep (new octave_scalar (t))
{
}

octave_value::octave_value (double d)
  : rep (new octave_scalar (d))
{
}

octave_value::octave_value (const Cell& c, bool is_csl)
  : rep (is_csl
	 ? dynamic_cast<octave_base_value *> (new octave_cs_list (c))
	 : dynamic_cast<octave_base_value *> (new octave_cell (c)))
{
}

octave_value::octave_value (const ArrayN<octave_value>& a, bool is_csl)
  : rep (is_csl
	 ? dynamic_cast<octave_base_value *> (new octave_cs_list (Cell (a)))
	 : dynamic_cast<octave_base_value *> (new octave_cell (Cell (a))))
{
}

octave_value::octave_value (const Matrix& m, const MatrixType& t)
  : rep (new octave_matrix (m, t))
{
  maybe_mutate ();
}

octave_value::octave_value (const NDArray& a)
  : rep (new octave_matrix (a))
{
  maybe_mutate ();
}

octave_value::octave_value (const ArrayN<double>& a)
  : rep (new octave_matrix (a))
{
  maybe_mutate ();
}

octave_value::octave_value (const DiagMatrix& d)
  : rep (new octave_matrix (d))
{
  maybe_mutate ();
}

octave_value::octave_value (const RowVector& v)
  : rep (new octave_matrix (v))
{
  maybe_mutate ();
}

octave_value::octave_value (const ColumnVector& v)
  : rep (new octave_matrix (v))
{
  maybe_mutate ();
}

octave_value::octave_value (const Complex& C)
  : rep (new octave_complex (C))
{
  maybe_mutate ();
}

octave_value::octave_value (const ComplexMatrix& m, const MatrixType& t)
  : rep (new octave_complex_matrix (m, t))
{
  maybe_mutate ();
}

octave_value::octave_value (const ComplexNDArray& a)
  : rep (new octave_complex_matrix (a))
{
  maybe_mutate ();
}

octave_value::octave_value (const ArrayN<Complex>& a)
  : rep (new octave_complex_matrix (a))
{
  maybe_mutate ();
}

octave_value::octave_value (const ComplexDiagMatrix& d)
  : rep (new octave_complex_matrix (d))
{
  maybe_mutate ();
}

octave_value::octave_value (const ComplexRowVector& v)
  : rep (new octave_complex_matrix (v))
{
  maybe_mutate ();
}

octave_value::octave_value (const ComplexColumnVector& v)
  : rep (new octave_complex_matrix (v))
{
  maybe_mutate ();
}

octave_value::octave_value (bool b)
  : rep (new octave_bool (b))
{
}

octave_value::octave_value (const boolMatrix& bm, const MatrixType& t)
  : rep (new octave_bool_matrix (bm, t))
{
  maybe_mutate ();
}

octave_value::octave_value (const boolNDArray& bnda)
  : rep (new octave_bool_matrix (bnda))
{
  maybe_mutate ();
}

octave_value::octave_value (char c, char type)
  : rep (type == '"'
	 ? new octave_char_matrix_dq_str (c)
	 : new octave_char_matrix_sq_str (c))
{
  maybe_mutate ();
}

octave_value::octave_value (const char *s, char type)
  : rep (type == '"'
	 ? new octave_char_matrix_dq_str (s)
	 : new octave_char_matrix_sq_str (s))
{
  maybe_mutate ();
}

octave_value::octave_value (const std::string& s, char type)
  : rep (type == '"'
	 ? new octave_char_matrix_dq_str (s)
	 : new octave_char_matrix_sq_str (s))
{
  maybe_mutate ();
}

octave_value::octave_value (const string_vector& s, char type)
  : rep (type == '"'
	 ? new octave_char_matrix_dq_str (s)
	 : new octave_char_matrix_sq_str (s))
{
  maybe_mutate ();
}

octave_value::octave_value (const charMatrix& chm, bool is_str, char type)
  : rep (is_str
	 ? (type == '"'
	    ? new octave_char_matrix_dq_str (chm)
	    : new octave_char_matrix_sq_str (chm))
	 : new octave_char_matrix (chm))
{
  maybe_mutate ();
}

octave_value::octave_value (const charNDArray& chm, bool is_str, char type)
  : rep (is_str
	 ? (type == '"'
	    ? new octave_char_matrix_dq_str (chm)
	    : new octave_char_matrix_sq_str (chm))
	 : new octave_char_matrix (chm))
{
  maybe_mutate ();
}

octave_value::octave_value (const ArrayN<char>& chm, bool is_str, char type)
  : rep (is_str
	 ? (type == '"'
	    ? new octave_char_matrix_dq_str (chm)
	    : new octave_char_matrix_sq_str (chm))
	 : new octave_char_matrix (chm))
{
  maybe_mutate ();
}

octave_value::octave_value (const SparseMatrix& m, const MatrixType &t)
  : rep (new octave_sparse_matrix (m, t))
{
  maybe_mutate ();
}

octave_value::octave_value (const SparseComplexMatrix& m, const MatrixType &t)
  : rep (new octave_sparse_complex_matrix (m, t))
{
  maybe_mutate ();
}

octave_value::octave_value (const SparseBoolMatrix& bm, const MatrixType &t)
  : rep (new octave_sparse_bool_matrix (bm, t))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_int8& i)
  : rep (new octave_int8_scalar (i))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_uint8& i)
  : rep (new octave_uint8_scalar (i))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_int16& i)
  : rep (new octave_int16_scalar (i))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_uint16& i)
  : rep (new octave_uint16_scalar (i))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_int32& i)
  : rep (new octave_int32_scalar (i))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_uint32& i)
  : rep (new octave_uint32_scalar (i))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_int64& i)
  : rep (new octave_int64_scalar (i))
{
  maybe_mutate ();
}

octave_value::octave_value (const octave_uint64& i)
  : rep (new octave_uint64_scalar (i))
{
  maybe_mutate ();
}

octave_value::octave_value (const int8NDArray& inda)
  : rep (new octave_int8_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const uint8NDArray& inda)
  : rep (new octave_uint8_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const int16NDArray& inda)
  : rep (new octave_int16_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const uint16NDArray& inda)
  : rep (new octave_uint16_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const int32NDArray& inda)
  : rep (new octave_int32_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const uint32NDArray& inda)
  : rep (new octave_uint32_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const int64NDArray& inda)
  : rep (new octave_int64_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (const uint64NDArray& inda)
  : rep (new octave_uint64_matrix (inda))
{
  maybe_mutate ();
}

octave_value::octave_value (double base, double limit, double inc)
  : rep (new octave_range (base, limit, inc))
{
  maybe_mutate ();
}

octave_value::octave_value (const Range& r)
  : rep (new octave_range (r))
{
  maybe_mutate ();
}

octave_value::octave_value (const Octave_map& m)
  : rep (new octave_struct (m))
{
}

octave_value::octave_value (const streamoff_array& off)
  : rep (new octave_streamoff (off))
{
}

octave_value::octave_value (const octave_value_list& l, bool is_csl)
  : rep (is_csl
	 ? dynamic_cast<octave_base_value *> (new octave_cs_list (l))
	 : dynamic_cast<octave_base_value *> (new octave_list (l)))
{
}

octave_value::octave_value (octave_value::magic_colon)
  : rep (new octave_magic_colon ())
{
}

octave_value::octave_value (octave_value::all_va_args)
  : rep (new octave_all_va_args ())
{
}

octave_value::octave_value (octave_base_value *new_rep)
  : rep (new_rep)
{
}

octave_value::~octave_value (void)
{
#if defined (MDEBUG)
  std::cerr << "~octave_value: rep: " << rep
	    << " rep->count: " << rep->count << std::endl;
#endif

  if (--rep->count == 0)
    delete rep;
}

octave_base_value *
octave_value::clone (void) const
{
  panic_impossible ();
  return 0;
}

void
octave_value::maybe_mutate (void)
{
  octave_base_value *tmp = rep->try_narrowing_conversion ();

  if (tmp && tmp != rep)
    {
      if (--rep->count == 0)
	delete rep;

      rep = tmp;
    }    
}

octave_value
octave_value::single_subsref (const std::string& type,
			      const octave_value_list& idx)
{
  std::list<octave_value_list> i;

  i.push_back (idx);

  return rep->subsref (type, i);
}

octave_value_list
octave_value::subsref (const std::string& type,
		       const std::list<octave_value_list>& idx, int nargout)
{
  if (is_constant ())
    return rep->subsref (type, idx);
  else
    return rep->subsref (type, idx, nargout);
}

octave_value
octave_value::next_subsref (const std::string& type,
			    const std::list<octave_value_list>& idx,
			    size_t skip) 
{
  if (! error_state && idx.size () > skip)
    {
      std::list<octave_value_list> new_idx (idx);
      for (size_t i = 0; i < skip; i++)
	new_idx.erase (new_idx.begin ());
      return subsref (type.substr (skip), new_idx);
    }
  else
    return *this;
}

octave_value_list
octave_value::next_subsref (int nargout, const std::string& type,
			    const std::list<octave_value_list>& idx,
			    size_t skip) 
{
  if (! error_state && idx.size () > skip)
    {
      std::list<octave_value_list> new_idx (idx);
      for (size_t i = 0; i < skip; i++)
	new_idx.erase (new_idx.begin ());
      return subsref (type.substr (skip), new_idx, nargout);
    }
  else
    return *this;
}

octave_value_list
octave_value::do_multi_index_op (int nargout, const octave_value_list& idx)
{
  return rep->do_multi_index_op (nargout, idx);
}

#if 0
static void
gripe_assign_failed (const std::string& on, const std::string& tn1,
		     const std::string& tn2)
{
  error ("assignment failed for `%s %s %s'",
	 tn1.c_str (), on.c_str (), tn2.c_str ());
}
#endif

static void
gripe_assign_failed_or_no_method (const std::string& on,
				  const std::string& tn1,
				  const std::string& tn2)
{
  error ("assignment failed, or no method for `%s %s %s'",
	 tn1.c_str (), on.c_str (), tn2.c_str ());
}

octave_value
octave_value::subsasgn (const std::string& type,
			const std::list<octave_value_list>& idx,
			const octave_value& rhs)
{
  return rep->subsasgn (type, idx, rhs);
}

octave_value
octave_value::assign (assign_op op, const std::string& type,
		      const std::list<octave_value_list>& idx,
		      const octave_value& rhs)
{
  octave_value retval;

  make_unique ();

  octave_value t_rhs = rhs;

  if (op != op_asn_eq)
    {
      // FIXME -- only do the following stuff if we can't find
      // a specific function to call to handle the op= operation for
      // the types we have.

      octave_value t;
      if (is_constant ())
	t = subsref (type, idx);
      else
	{
	  octave_value_list tl = subsref (type, idx, 1);
	  if (tl.length () > 0)
	    t = tl(0);
	}

      if (! error_state)
	{
	  binary_op binop = op_eq_to_binary_op (op);

	  if (! error_state)
	    t_rhs = do_binary_op (binop, t, rhs);
	}
    }

  if (! error_state)
    {
      if (type[0] == '.' && ! is_map ())
	{
	  octave_value tmp = Octave_map ();
	  retval = tmp.subsasgn (type, idx, t_rhs);
	}
      else
	retval = subsasgn (type, idx, t_rhs);
    }

  if (error_state)
    gripe_assign_failed_or_no_method (assign_op_as_string (op),
				      type_name (), rhs.type_name ());

  return retval;
}

const octave_value&
octave_value::assign (assign_op op, const octave_value& rhs)
{
  if (op == op_asn_eq)
    operator = (rhs);
  else
    {
      // FIXME -- only do the following stuff if we can't find
      // a specific function to call to handle the op= operation for
      // the types we have.

      binary_op binop = op_eq_to_binary_op (op);

      if (! error_state)
	{
	  octave_value t = do_binary_op (binop, *this, rhs);

	  if (! error_state)
	    operator = (t);
	}

      if (error_state)
	gripe_assign_failed_or_no_method (assign_op_as_string (op),
					  type_name (), rhs.type_name ());
    }

  return *this;
}

octave_idx_type
octave_value::length (void) const
{
  int retval = 0;

  dim_vector dv = dims ();

  for (int i = 0; i < dv.length (); i++)
    {
      if (dv(i) < 0)
	{
	  retval = -1;
	  break;
	}

      if (dv(i) == 0)
	{
	  retval = 0;
	  break;
	}

      if (dv(i) > retval)
	retval = dv(i);
    }

  return retval;
}

Matrix
octave_value::size (void) const
{
  dim_vector dv = dims ();

  int n_dims = dv.length ();

  Matrix retval (1, n_dims);

  while (n_dims--)
    retval(n_dims) = dv(n_dims);

  return retval;
}

Cell
octave_value::cell_value (void) const
{
  return rep->cell_value ();
}

Octave_map
octave_value::map_value (void) const
{
  return rep->map_value ();
}

std::streamoff
octave_value::streamoff_value (void) const
{
  return rep->streamoff_value ();
}

streamoff_array
octave_value::streamoff_array_value (void) const
{
  return rep->streamoff_array_value ();
}

octave_function *
octave_value::function_value (bool silent)
{
  return rep->function_value (silent);
}

octave_user_function *
octave_value::user_function_value (bool silent)
{
  return rep->user_function_value (silent);
}

octave_fcn_handle *
octave_value::fcn_handle_value (bool silent)
{
  return rep->fcn_handle_value (silent);
}

octave_fcn_inline *
octave_value::fcn_inline_value (bool silent)
{
  return rep->fcn_inline_value (silent);
}

octave_value_list
octave_value::list_value (void) const
{
  return rep->list_value ();
}

ColumnVector
octave_value::column_vector_value (bool force_string_conv,
				   bool /* frc_vec_conv */) const
{
  ColumnVector retval;

  Matrix m = matrix_value (force_string_conv);

  if (error_state)
    return retval;

  octave_idx_type nr = m.rows ();
  octave_idx_type nc = m.columns ();

  if (nc == 1)
    {
      retval.resize (nr);
      for (octave_idx_type i = 0; i < nr; i++)
	retval (i) = m (i, 0);
    }
  else
    {
      std::string tn = type_name ();
      gripe_invalid_conversion (tn.c_str (), "real column vector");
    }

  return retval;
}

ComplexColumnVector
octave_value::complex_column_vector_value (bool force_string_conv,
					   bool /* frc_vec_conv */) const
{
  ComplexColumnVector retval;

  ComplexMatrix m = complex_matrix_value (force_string_conv);

  if (error_state)
    return retval;

  octave_idx_type nr = m.rows ();
  octave_idx_type nc = m.columns ();

  if (nc == 1)
    {
      retval.resize (nr);
      for (octave_idx_type i = 0; i < nr; i++)
	retval (i) = m (i, 0);
    }
  else
    {
      std::string tn = type_name ();
      gripe_invalid_conversion (tn.c_str (), "complex column vector");
    }

  return retval;
}

RowVector
octave_value::row_vector_value (bool force_string_conv,
				bool /* frc_vec_conv */) const
{
  RowVector retval;

  Matrix m = matrix_value (force_string_conv);

  if (error_state)
    return retval;

  octave_idx_type nr = m.rows ();
  octave_idx_type nc = m.columns ();

  if (nr == 1)
    {
      retval.resize (nc);
      for (octave_idx_type i = 0; i < nc; i++)
	retval (i) = m (0, i);
    }
  else
    {
      std::string tn = type_name ();
      gripe_invalid_conversion (tn.c_str (), "real row vector");
    }

  return retval;
}

ComplexRowVector
octave_value::complex_row_vector_value (bool force_string_conv,
					bool /* frc_vec_conv */) const
{
  ComplexRowVector retval;

  ComplexMatrix m = complex_matrix_value (force_string_conv);

  if (error_state)
    return retval;

  octave_idx_type nr = m.rows ();
  octave_idx_type nc = m.columns ();

  if (nr == 1)
    {
      retval.resize (nc);
      for (octave_idx_type i = 0; i < nc; i++)
	retval (i) = m (0, i);
    }
  else
    {
      std::string tn = type_name ();
      gripe_invalid_conversion (tn.c_str (), "complex row vector");
    }

  return retval;
}

// Sloppy...

Array<double>
octave_value::vector_value (bool force_string_conv,
			    bool force_vector_conversion) const
{
  Array<double> retval;

  Matrix m = matrix_value (force_string_conv);

  if (error_state)
    return retval;

  octave_idx_type nr = m.rows ();
  octave_idx_type nc = m.columns ();

  if (nr == 1)
    {
      retval.resize (nc);
      for (octave_idx_type i = 0; i < nc; i++)
	retval (i) = m (0, i);
    }
  else if (nc == 1)
    {
      retval.resize (nr);
      for (octave_idx_type i = 0; i < nr; i++)
	retval (i) = m (i, 0);
    }
  else if (nr > 0 && nc > 0)
    {
      if (! force_vector_conversion)
	gripe_implicit_conversion ("Octave:array-as-vector",
				   type_name (), "real vector");

      retval.resize (nr * nc);
      octave_idx_type k = 0;
      for (octave_idx_type j = 0; j < nc; j++)
	for (octave_idx_type i = 0; i < nr; i++)
	  {
	    OCTAVE_QUIT;

	    retval (k++) = m (i, j);
	  }
    }
  else
    {
      std::string tn = type_name ();
      gripe_invalid_conversion (tn.c_str (), "real vector");
    }

  return retval;
}

Array<int>
octave_value::int_vector_value (bool force_string_conv, bool require_int,
				bool force_vector_conversion) const
{
  Array<int> retval;

  Matrix m = matrix_value (force_string_conv);

  if (error_state)
    return retval;

  octave_idx_type nr = m.rows ();
  octave_idx_type nc = m.columns ();

  if (nr == 1)
    {
      retval.resize (nc);
      for (octave_idx_type i = 0; i < nc; i++)
	{
	  OCTAVE_QUIT;

	  double d = m (0, i);

	  if (require_int && D_NINT (d) != d)
	    {
	      error ("conversion to integer value failed");
	      return retval;
	    }

	  retval (i) = static_cast<int> (d);
	}
    }
  else if (nc == 1)
    {
      retval.resize (nr);
      for (octave_idx_type i = 0; i < nr; i++)
	{
	  OCTAVE_QUIT;

	  double d = m (i, 0);

	  if (require_int && D_NINT (d) != d)
	    {
	      error ("conversion to integer value failed");
	      return retval;
	    }

	  retval (i) = static_cast<int> (d);
	}
    }
  else if (nr > 0 && nc > 0)
    {
      if (! force_vector_conversion)
	gripe_implicit_conversion ("Octave:array-as-vector",
				   type_name (), "real vector");

      retval.resize (nr * nc);
      octave_idx_type k = 0;
      for (octave_idx_type j = 0; j < nc; j++)
	{
	  for (octave_idx_type i = 0; i < nr; i++)
	    {
	      OCTAVE_QUIT;

	      double d = m (i, j);

	      if (require_int && D_NINT (d) != d)
		{
		  error ("conversion to integer value failed");
		  return retval;
		}

	      retval (k++) = static_cast<int> (d);
	    }
	}
    }
  else
    {
      std::string tn = type_name ();
      gripe_invalid_conversion (tn.c_str (), "real vector");
    }

  return retval;
}

Array<Complex>
octave_value::complex_vector_value (bool force_string_conv,
				    bool force_vector_conversion) const
{
  Array<Complex> retval;

  ComplexMatrix m = complex_matrix_value (force_string_conv);

  if (error_state)
    return retval;

  octave_idx_type nr = m.rows ();
  octave_idx_type nc = m.columns ();

  if (nr == 1)
    {
      retval.resize (nc);
      for (octave_idx_type i = 0; i < nc; i++)
	{
	  OCTAVE_QUIT;
	  retval (i) = m (0, i);
	}
    }
  else if (nc == 1)
    {
      retval.resize (nr);
      for (octave_idx_type i = 0; i < nr; i++)
	{
	  OCTAVE_QUIT;
	  retval (i) = m (i, 0);
	}
    }
  else if (nr > 0 && nc > 0)
    {
      if (! force_vector_conversion)
	gripe_implicit_conversion ("Octave:array-as-vector",
				   type_name (), "complex vector");

      retval.resize (nr * nc);
      octave_idx_type k = 0;
      for (octave_idx_type j = 0; j < nc; j++)
	for (octave_idx_type i = 0; i < nr; i++)
	  {
	    OCTAVE_QUIT;
	    retval (k++) = m (i, j);
	  }
    }
  else
    {
      std::string tn = type_name ();
      gripe_invalid_conversion (tn.c_str (), "complex vector");
    }

  return retval;
}

int
octave_value::write (octave_stream& os, int block_size,
		     oct_data_conv::data_type output_type, int skip,
		     oct_mach_info::float_format flt_fmt) const
{
  return rep->write (os, block_size, output_type, skip, flt_fmt);
}

static void
gripe_binary_op (const std::string& on, const std::string& tn1,
		 const std::string& tn2)
{
  error ("binary operator `%s' not implemented for `%s' by `%s' operations",
	 on.c_str (), tn1.c_str (), tn2.c_str ());
}

static void
gripe_binary_op_conv (const std::string& on)
{
  error ("type conversion failed for binary operator `%s'", on.c_str ());
}

octave_value
do_binary_op (octave_value::binary_op op,
	      const octave_value& v1, const octave_value& v2)
{
  octave_value retval;

  int t1 = v1.type_id ();
  int t2 = v2.type_id ();

  octave_value_typeinfo::binary_op_fcn f
    = octave_value_typeinfo::lookup_binary_op (op, t1, t2);

  if (f)
    retval = f (*v1.rep, *v2.rep);
  else
    {
      octave_value tv1;
      octave_base_value::type_conv_fcn cf1 = v1.numeric_conversion_function ();

      if (cf1)
	{
	  octave_base_value *tmp = cf1 (*v1.rep);

	  if (tmp)
	    {
	      tv1 = octave_value (tmp);
	      t1 = tv1.type_id ();
	    }
	  else
	    {
	      gripe_binary_op_conv (octave_value::binary_op_as_string (op));
	      return retval;
	    }
	}
      else
	tv1 = v1;

      octave_value tv2;
      octave_base_value::type_conv_fcn cf2 = v2.numeric_conversion_function ();

      if (cf2)
	{
	  octave_base_value *tmp = cf2 (*v2.rep);

	  if (tmp)
	    {
	      tv2 = octave_value (tmp);
	      t2 = tv2.type_id ();
	    }
	  else
	    {
	      gripe_binary_op_conv (octave_value::binary_op_as_string (op));
	      return retval;
	    }
	}
      else
	tv2 = v2;

      if (cf1 || cf2)
	{
	  f = octave_value_typeinfo::lookup_binary_op (op, t1, t2);

	  if (f)
	    retval = f (*tv1.rep, *tv2.rep);
	  else
	    gripe_binary_op (octave_value::binary_op_as_string (op),
			     v1.type_name (), v2.type_name ());
	}
      else
	gripe_binary_op (octave_value::binary_op_as_string (op),
			 v1.type_name (), v2.type_name ());
    }

  return retval;
}

static void
gripe_cat_op (const std::string& tn1, const std::string& tn2)
{
  error ("concatenation operator not implemented for `%s' by `%s' operations",
	 tn1.c_str (), tn2.c_str ());
}

static void
gripe_cat_op_conv (void)
{
  error ("type conversion failed for concatenation operator");
}

octave_value
do_cat_op (const octave_value& v1, const octave_value& v2, 
	   const Array<int>& ra_idx)
{
  octave_value retval;

  // Rapid return for concatenation with an empty object. Dimension
  // checking handled elsewhere.
  if (v1.all_zero_dims ())
    return v2;
  if (v2.all_zero_dims ())
    return v1;

  int t1 = v1.type_id ();
  int t2 = v2.type_id ();

  octave_value_typeinfo::cat_op_fcn f
    = octave_value_typeinfo::lookup_cat_op (t1, t2);

  if (f)
    retval = f (*v1.rep, *v2.rep, ra_idx);
  else
    {
      octave_value tv1;
      octave_base_value::type_conv_fcn cf1 = v1.numeric_conversion_function ();

      if (cf1)
	{
	  octave_base_value *tmp = cf1 (*v1.rep);

	  if (tmp)
	    {
	      tv1 = octave_value (tmp);
	      t1 = tv1.type_id ();
	    }
	  else
	    {
	      gripe_cat_op_conv ();
	      return retval;
	    }
	}
      else
	tv1 = v1;

      octave_value tv2;
      octave_base_value::type_conv_fcn cf2 = v2.numeric_conversion_function ();

      if (cf2)
	{
	  octave_base_value *tmp = cf2 (*v2.rep);

	  if (tmp)
	    {
	      tv2 = octave_value (tmp);
	      t2 = tv2.type_id ();
	    }
	  else
	    {
	      gripe_cat_op_conv ();
	      return retval;
	    }
	}
      else
	tv2 = v2;

      if (cf1 || cf2)
	{
	  f = octave_value_typeinfo::lookup_cat_op (t1, t2);

	  if (f)
	    retval = f (*tv1.rep, *tv2.rep, ra_idx);
	  else
	    gripe_cat_op (v1.type_name (), v2.type_name ());
	}
      else
	gripe_cat_op (v1.type_name (), v2.type_name ());
    }

  return retval;
}

void
octave_value::print_info (std::ostream& os, const std::string& prefix) const
{
  os << prefix << "type_name: " << type_name () << "\n"
     << prefix << "count:     " << get_count () << "\n"
     << prefix << "rep info:  ";

  rep->print_info (os, prefix + " ");
}

static void
gripe_unary_op (const std::string& on, const std::string& tn)
{
  error ("unary operator `%s' not implemented for `%s' operands",
	 on.c_str (), tn.c_str ());
}

static void
gripe_unary_op_conv (const std::string& on)
{
  error ("type conversion failed for unary operator `%s'", on.c_str ());
}

octave_value
do_unary_op (octave_value::unary_op op, const octave_value& v)
{
  octave_value retval;

  int t = v.type_id ();

  octave_value_typeinfo::unary_op_fcn f
    = octave_value_typeinfo::lookup_unary_op (op, t);

  if (f)
    retval = f (*v.rep);
  else
    {
      octave_value tv;
      octave_base_value::type_conv_fcn cf = v.numeric_conversion_function ();

      if (cf)
	{
	  octave_base_value *tmp = cf (*v.rep);

	  if (tmp)
	    {
	      tv = octave_value (tmp);
	      t = tv.type_id ();

	      f = octave_value_typeinfo::lookup_unary_op (op, t);

	      if (f)
		retval = f (*tv.rep);
	      else
		gripe_unary_op (octave_value::unary_op_as_string (op),
				v.type_name ());
	    }
	  else
	    gripe_unary_op_conv (octave_value::unary_op_as_string (op));
	}
      else
	gripe_unary_op (octave_value::unary_op_as_string (op),
			v.type_name ());
    }

  return retval;
}

static void
gripe_unary_op_conversion_failed (const std::string& op,
				  const std::string& tn)
{
  error ("operator %s: type conversion for `%s' failed",
	 op.c_str (), tn.c_str ());
}

const octave_value&
octave_value::do_non_const_unary_op (unary_op op)
{
  octave_value retval;

  int t = type_id ();

  octave_value_typeinfo::non_const_unary_op_fcn f
    = octave_value_typeinfo::lookup_non_const_unary_op (op, t);

  if (f)
    {
      make_unique ();

      f (*rep);
    }
  else
    {
      octave_base_value::type_conv_fcn cf = numeric_conversion_function ();

      if (cf)
	{
	  octave_base_value *tmp = cf (*rep);

	  if (tmp)
	    {
	      octave_base_value *old_rep = rep;
	      rep = tmp;

	      t = type_id ();

	      f = octave_value_typeinfo::lookup_non_const_unary_op (op, t);

	      if (f)
		{
		  f (*rep);

		  if (old_rep && --old_rep->count == 0)
		    delete old_rep;
		}
	      else
		{
		  if (old_rep)
		    {
		      if (--rep->count == 0)
			delete rep;

		      rep = old_rep;
		    }

		  gripe_unary_op (octave_value::unary_op_as_string (op),
				  type_name ());
		}
	    }
	  else
	    gripe_unary_op_conversion_failed
	      (octave_value::unary_op_as_string (op), type_name ());
	}
      else
	gripe_unary_op (octave_value::unary_op_as_string (op), type_name ());
    }

  return *this;
}

#if 0
static void
gripe_unary_op_failed_or_no_method (const std::string& on,
				    const std::string& tn) 
{
  error ("operator %s: no method, or unable to evaluate for %s operand",
	 on.c_str (), tn.c_str ());
}
#endif

void
octave_value::do_non_const_unary_op (unary_op, const octave_value_list&)
{
  abort ();
}

octave_value
octave_value::do_non_const_unary_op (unary_op op, const std::string& type,
				     const std::list<octave_value_list>& idx)
{
  octave_value retval;

  if (idx.empty ())
    {
      do_non_const_unary_op (op);

      retval = *this;
    }
  else
    {
      // FIXME -- only do the following stuff if we can't find a
      // specific function to call to handle the op= operation for the
      // types we have.

      assign_op assop = unary_op_to_assign_op (op);

      retval = assign (assop, type, idx, 1.0);
    }

  return retval;
}

octave_value::assign_op
octave_value::unary_op_to_assign_op (unary_op op)
{
  assign_op binop = unknown_assign_op;

  switch (op)
    {
    case op_incr:
      binop = op_add_eq;
      break;

    case op_decr:
      binop = op_sub_eq;
      break;

    default:
      {
	std::string on = unary_op_as_string (op);
	error ("operator %s: no assign operator found", on.c_str ());
      }
    }

  return binop;
}

octave_value::binary_op 
octave_value::op_eq_to_binary_op (assign_op op)
{
  binary_op binop = unknown_binary_op;

  switch (op)
    {
    case op_add_eq:
      binop = op_add;
      break;

    case op_sub_eq:
      binop = op_sub;
      break;

    case op_mul_eq:
      binop = op_mul;
      break;

    case op_div_eq:
      binop = op_div;
      break;

    case op_ldiv_eq:
      binop = op_ldiv;
      break;

    case op_pow_eq:
      binop = op_pow;
      break;

    case op_lshift_eq:
      binop = op_lshift;
      break;

    case op_rshift_eq:
      binop = op_rshift;
      break;

    case op_el_mul_eq:
      binop = op_el_mul;
      break;

    case op_el_div_eq:
      binop = op_el_div;
      break;

    case op_el_ldiv_eq:
      binop = op_el_ldiv;
      break;

    case op_el_pow_eq:
      binop = op_el_pow;
      break;

    case op_el_and_eq:
      binop = op_el_and;
      break;

    case op_el_or_eq:
      binop = op_el_or;
      break;

    default:
      {
	std::string on = assign_op_as_string (op);
	error ("operator %s: no binary operator found", on.c_str ());
      }
    }

  return binop;
}

octave_value
octave_value::empty_conv (const std::string& type, const octave_value& rhs)
{
  octave_value retval;

  if (type.length () > 0)
    {
      switch (type[0])
	{
	case '(':
	  {
	    if (type.length () > 1 && type[1] == '.')
	      retval = Octave_map ();
	    else
	      retval = octave_value (rhs.empty_clone ());
	  }
	  break;

	case '{':
	  retval = Cell ();
	  break;

	case '.':
	  retval = Octave_map ();
	  break;

	default:
	  panic_impossible ();
	}
    }
  else
    retval = octave_value (rhs.empty_clone ());

  return retval;
}

void
install_types (void)
{
  octave_base_value::register_type ();
  octave_cell::register_type ();
  octave_scalar::register_type ();
  octave_complex::register_type ();
  octave_matrix::register_type ();
  octave_complex_matrix::register_type ();
  octave_range::register_type ();
  octave_bool::register_type ();
  octave_bool_matrix::register_type ();
  octave_char_matrix::register_type ();
  octave_char_matrix_str::register_type ();
  octave_char_matrix_sq_str::register_type ();
  octave_int8_scalar::register_type ();
  octave_int16_scalar::register_type ();
  octave_int32_scalar::register_type ();
  octave_int64_scalar::register_type ();
  octave_uint8_scalar::register_type ();
  octave_uint16_scalar::register_type ();
  octave_uint32_scalar::register_type ();
  octave_uint64_scalar::register_type ();
  octave_int8_matrix::register_type ();
  octave_int16_matrix::register_type ();
  octave_int32_matrix::register_type ();
  octave_int64_matrix::register_type ();
  octave_uint8_matrix::register_type ();
  octave_uint16_matrix::register_type ();
  octave_uint32_matrix::register_type ();
  octave_uint64_matrix::register_type ();
  octave_sparse_bool_matrix::register_type ();
  octave_sparse_matrix::register_type ();
  octave_sparse_complex_matrix::register_type ();
  octave_struct::register_type ();
  octave_list::register_type ();
  octave_cs_list::register_type ();
  octave_all_va_args::register_type ();
  octave_magic_colon::register_type ();
  octave_builtin::register_type ();
  octave_mapper::register_type ();
  octave_user_function::register_type ();
  octave_dld_function::register_type ();
  octave_fcn_handle::register_type ();
  octave_fcn_inline::register_type ();
  octave_streamoff::register_type ();
}

#if 0
DEFUN (cast, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} cast (@var{val}, @var{type})\n\
Convert @var{val} to the new data type @var{type}.\n\
@seealso{class, typeinfo}\n\
@end deftypefn")
{
  octave_value retval;

  if (args.length () == 2)
    error ("cast: not implemented");
  else
    print_usage ("cast");

  return retval;
}
#endif

DEFUN (sizeof, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} sizeof (@var{val})\n\
Return the size of @var{val} in bytes\n\
@end deftypefn")
{
  octave_value retval;

  if (args.length () == 1)
    retval = args(0).byte_size ();
  else
    print_usage ("sizeof");

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
