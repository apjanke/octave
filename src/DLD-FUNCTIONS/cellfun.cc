/*

Copyright (C) 2005, 2006, 2007, 2008, 2009 Mohamed Kamoun
Copyright (C) 2009 Jaroslav Hajek

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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <string>
#include <vector>
#include <list>
#include <memory>

#include "lo-mappers.h"
#include "oct-locbuf.h"

#include "Cell.h"
#include "oct-map.h"
#include "defun-dld.h"
#include "parse.h"
#include "variables.h"
#include "ov-colon.h"
#include "unwind-prot.h"
#include "gripes.h"
#include "utils.h"

// Rationale:
// The octave_base_value::subsasgn method carries too much overhead for
// per-element assignment strategy.
// This class will optimize the most optimistic and most likely case
// when the output really is scalar by defining a hierarchy of virtual
// collectors specialized for some scalar types.

class scalar_col_helper
{
public:
  virtual bool collect (octave_idx_type i, const octave_value& val) = 0;
  virtual octave_value result (void) = 0;
  virtual ~scalar_col_helper (void) { }
};

// The default collector represents what was previously done in the main loop.
// This reuses the existing assignment machinery via octave_value::subsasgn,
// which can perform all sorts of conversions, but is relatively slow.

class scalar_col_helper_def : public scalar_col_helper
{
  std::list<octave_value_list> idx_list;
  octave_value resval;
public:
  scalar_col_helper_def (const octave_value& val, const dim_vector& dims)
    : idx_list (1), resval (val)
    {
      idx_list.front ().resize (1);
      if (resval.dims () != dims)
        resval.resize (dims);
    }
  ~scalar_col_helper_def (void) { }

  bool collect (octave_idx_type i, const octave_value& val)
    {
      if (val.numel () == 1)
        {
          idx_list.front ()(0) = static_cast<double> (i + 1);
          resval = resval.subsasgn ("(", idx_list, val);
        }
      else
        error ("cellfun: expecting all values to be scalars for UniformOutput = true");

      return true;
    }
  octave_value result (void)
    {
      return resval;
    }
};

template <class T>
struct scalar_query_helper { };

#define DEF_QUERY_HELPER(T, TEST, QUERY) \
template <> \
struct scalar_query_helper<T> \
{ \
  static bool has_value (const octave_value& val) \
    { return TEST; } \
  static T get_value (const octave_value& val) \
    { return QUERY; } \
}

DEF_QUERY_HELPER (double, val.is_real_scalar (), val.scalar_value ());
DEF_QUERY_HELPER (Complex, val.is_complex_scalar (), val.complex_value ());
DEF_QUERY_HELPER (float, val.is_single_type () && val.is_real_scalar (), 
                  val.float_scalar_value ());
DEF_QUERY_HELPER (FloatComplex, val.is_single_type () && val.is_complex_scalar (), 
                  val.float_complex_value ());
DEF_QUERY_HELPER (bool, val.is_bool_scalar (), val.bool_value ());
// FIXME: More?

// This specializes for collecting elements of a single type, by accessing
// an array directly. If the scalar is not valid, it returns false.

template <class NDA>
class scalar_col_helper_nda : public scalar_col_helper
{
  NDA arrayval;
  typedef typename NDA::element_type T;
public:
  scalar_col_helper_nda (const octave_value& val, const dim_vector& dims)
    : arrayval (dims)
    {
      arrayval(0) = scalar_query_helper<T>::get_value (val);
    }
  ~scalar_col_helper_nda (void) { }

  bool collect (octave_idx_type i, const octave_value& val)
    {
      bool retval = scalar_query_helper<T>::has_value (val);
      if (retval)
        arrayval(i) = scalar_query_helper<T>::get_value (val);
      return retval;
    }
  octave_value result (void)
    {
      return arrayval;
    }
};

template class scalar_col_helper_nda<NDArray>;
template class scalar_col_helper_nda<FloatNDArray>;
template class scalar_col_helper_nda<ComplexNDArray>;
template class scalar_col_helper_nda<FloatComplexNDArray>;
template class scalar_col_helper_nda<boolNDArray>;

// the virtual constructor.
scalar_col_helper *
make_col_helper (const octave_value& val, const dim_vector& dims)
{
  scalar_col_helper *retval;

  if (val.is_bool_scalar ())
    retval = new scalar_col_helper_nda<boolNDArray> (val, dims);
  else if (val.is_complex_scalar ())
    {
      if (val.is_single_type ())
        retval = new scalar_col_helper_nda<FloatComplexNDArray> (val, dims);
      else
        retval = new scalar_col_helper_nda<ComplexNDArray> (val, dims);
    }
  else if (val.is_real_scalar ())
    {
      if (val.is_single_type ())
        retval = new scalar_col_helper_nda<FloatNDArray> (val, dims);
      else
        retval = new scalar_col_helper_nda<NDArray> (val, dims);
    }
  else
    retval = new scalar_col_helper_def (val, dims);

  return retval;
}

DEFUN_DLD (cellfun, args, nargout,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} cellfun (@var{name}, @var{c})\n\
@deftypefnx {Loadable Function} {} cellfun (\"size\", @var{c}, @var{k})\n\
@deftypefnx {Loadable Function} {} cellfun (\"isclass\", @var{c}, @var{class})\n\
@deftypefnx {Loadable Function} {} cellfun (@var{func}, @var{c})\n\
@deftypefnx {Loadable Function} {} cellfun (@var{func}, @var{c}, @var{d})\n\
@deftypefnx {Loadable Function} {[@var{a}, @var{b}] =} cellfun (@dots{})\n\
@deftypefnx {Loadable Function} {} cellfun (@dots{}, 'ErrorHandler', @var{errfunc})\n\
@deftypefnx {Loadable Function} {} cellfun (@dots{}, 'UniformOutput', @var{val})\n\
\n\
Evaluate the function named @var{name} on the elements of the cell array\n\
@var{c}.  Elements in @var{c} are passed on to the named function\n\
individually.  The function @var{name} can be one of the functions\n\
\n\
@table @code\n\
@item isempty\n\
Return 1 for empty elements.\n\
@item islogical\n\
Return 1 for logical elements.\n\
@item isreal\n\
Return 1 for real elements.\n\
@item length\n\
Return a vector of the lengths of cell elements.\n\
@item ndims\n\
Return the number of dimensions of each element.\n\
@item prodofsize\n\
Return the product of dimensions of each element.\n\
@item size\n\
Return the size along the @var{k}-th dimension.\n\
@item isclass\n\
Return 1 for elements of @var{class}.\n\
@end table\n\
\n\
Additionally, @code{cellfun} accepts an arbitrary function @var{func}\n\
in the form of an inline function, function handle, or the name of a\n\
function (in a character string).  In the case of a character string\n\
argument, the function must accept a single argument named @var{x}, and\n\
it must return a string value.  The function can take one or more arguments,\n\
with the inputs args given by @var{c}, @var{d}, etc.  Equally the function\n\
can return one or more output arguments.  For example\n\
\n\
@example\n\
@group\n\
cellfun (@@atan2, @{1, 0@}, @{0, 1@})\n\
@result{}ans = [1.57080   0.00000]\n\
@end group\n\
@end example\n\
\n\
Note that the default output argument is an array of the same size as the\n\
input arguments.\n\
Input arguments that are singleton (1x1) cells will be automatically expanded\n\
to the size of the other arguments.\n\
\n\
If the parameter 'UniformOutput' is set to true (the default), then the function\n\
must return a single element which will be concatenated into the\n\
return value.  If 'UniformOutput' is false, the outputs are concatenated in\n\
a cell array.  For example\n\
\n\
@example\n\
@group\n\
cellfun (\"tolower(x)\", @{\"Foo\", \"Bar\", \"FooBar\"@},\n\
         \"UniformOutput\",false)\n\
@result{} ans = @{\"foo\", \"bar\", \"foobar\"@}\n\
@end group\n\
@end example\n\
\n\
Given the parameter 'ErrorHandler', then @var{errfunc} defines a function to\n\
call in case @var{func} generates an error.  The form of the function is\n\
\n\
@example\n\
function [@dots{}] = errfunc (@var{s}, @dots{})\n\
@end example\n\
\n\
where there is an additional input argument to @var{errfunc} relative to\n\
@var{func}, given by @var{s}.  This is a structure with the elements\n\
'identifier', 'message' and 'index', giving respectively the error\n\
identifier, the error message, and the index into the input arguments\n\
of the element that caused the error.  For example\n\
\n\
@example\n\
@group\n\
function y = foo (s, x), y = NaN; endfunction\n\
cellfun (@@factorial, @{-1,2@},'ErrorHandler',@@foo)\n\
@result{} ans = [NaN 2]\n\
@end group\n\
@end example\n\
\n\
@seealso{isempty, islogical, isreal, length, ndims, numel, size}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();
  int nargout1 = (nargout < 1 ? 1 : nargout);

  if (nargin < 2)
    {
      error ("cellfun: you must supply at least 2 arguments");
      print_usage ();
      return retval;
    }

  octave_value func = args(0);

  if (! args(1).is_cell ())
    {
      error ("cellfun: second argument must be a cell array");

      return retval;
    }
  
  if (func.is_string ())
    {
      const Cell f_args = args(1).cell_value ();

      octave_idx_type k = f_args.numel ();

      std::string name = func.string_value ();

      if (name == "isempty")
        {      
          boolNDArray result (f_args.dims ());
          for (octave_idx_type count = 0; count < k ; count++)
            result(count) = f_args.elem(count).is_empty ();
          retval(0) = result;
        }
      else if (name == "islogical")
        {
          boolNDArray result (f_args.dims ());
          for (octave_idx_type  count= 0; count < k ; count++)
            result(count) = f_args.elem(count).is_bool_type ();
          retval(0) = result;
        }
      else if (name == "isreal")
        {
          boolNDArray result (f_args.dims ());
          for (octave_idx_type  count= 0; count < k ; count++)
            result(count) = f_args.elem(count).is_real_type ();
          retval(0) = result;
        }
      else if (name == "length")
        {
          NDArray result (f_args.dims ());
          for (octave_idx_type  count= 0; count < k ; count++)
            result(count) = static_cast<double> (f_args.elem(count).length ());
          retval(0) = result;
        }
      else if (name == "ndims")
        {
          NDArray result (f_args.dims ());
          for (octave_idx_type count = 0; count < k ; count++)
            result(count) = static_cast<double> (f_args.elem(count).ndims ());
          retval(0) = result;
        }
      else if (name == "prodofsize" || name == "numel")
        {
          NDArray result (f_args.dims ());
          for (octave_idx_type count = 0; count < k ; count++)
            result(count) = static_cast<double> (f_args.elem(count).numel ());
          retval(0) = result;
        }
      else if (name == "size")
        {
          if (nargin == 3)
            {
              int d = args(2).nint_value () - 1;

              if (d < 0)
                error ("cellfun: third argument must be a positive integer");

              if (! error_state)
                {
                  NDArray result (f_args.dims ());
                  for (octave_idx_type count = 0; count < k ; count++)
                    {
                      dim_vector dv = f_args.elem(count).dims ();
                      if (d < dv.length ())
                        result(count) = static_cast<double> (dv(d));
                      else
                        result(count) = 1.0;
                    }
                  retval(0) = result;
                }
            }
          else
            error ("not enough arguments for `size'");
        }
      else if (name == "isclass")
        {
          if (nargin == 3)
            {
              std::string class_name = args(2).string_value();
              boolNDArray result (f_args.dims ());
              for (octave_idx_type count = 0; count < k ; count++)
                result(count) = (f_args.elem(count).class_name() == class_name);

              retval(0) = result;
            }
          else
            error ("not enough arguments for `isclass'");
        }
      else if (name == "subsref" && nargin == 5 && nargout == 1
               && args(2).numel () == 1 && args(2).is_cell () 
               && args(3).is_string ()
               && args(3).xtoupper ().string_value () == "uniformoutput"
               && args(4).bool_value () && ! error_state)
        {
          // This optimizes the case of applying the same index expression to
          // multiple values. We decode the subscript just once. uniformoutput must
          // be requested as well.

          const Cell tmpc = args(2).cell_value ();
          octave_value subs = tmpc(0);

          std::string type;
          std::list<octave_value_list> idx;
          decode_subscripts ("subsref", subs, type, idx);

          if (! error_state)
            {
              Cell result (f_args.dims ());
              for (octave_idx_type count = 0; count < k && ! error_state; count++)
                result(count) = f_args.elem (count).subsref (type, idx);

              retval(0) = result;
            }
        }
      else
        {
          if (! valid_identifier (name))
            {

              std::string fcn_name = unique_symbol_name ("__cellfun_fcn_");
              std::string fname = "function y = ";
              fname.append (fcn_name);
              fname.append ("(x) y = ");
              octave_function *ptr_func = extract_function (args(0), "cellfun", 
                                                            fcn_name, fname, "; endfunction");
              if (ptr_func && ! error_state)
                func = octave_value (ptr_func, true);
            }

          func = symbol_table::find_function (name);
          if (func.is_undefined ())
            error ("cellfun: invalid function name: %s", name.c_str ());
        }
    }

  if (error_state || ! retval.empty ())
    return retval;

  if (func.is_function_handle () || func.is_inline_function ()
      || func.is_function ())
    {
      unwind_protect::frame_id_t uwp_frame = unwind_protect::begin_frame ();
      unwind_protect::protect_var (buffer_error_messages);

      bool uniform_output = true;
      octave_value error_handler;

      while (nargin > 3 && args(nargin-2).is_string())
        {
          std::string arg = args(nargin-2).string_value();

          std::transform (arg.begin (), arg.end (), 
                          arg.begin (), tolower);

          if (arg == "uniformoutput")
            uniform_output = args(nargin-1).bool_value();
          else if (arg == "errorhandler")
            {
              if (args(nargin-1).is_function_handle () || 
                  args(nargin-1).is_inline_function ())
                {
                  error_handler = args(nargin-1);
                }
              else if (args(nargin-1).is_string ())
                {
                  std::string err_name = args(nargin-1).string_value ();
                  error_handler = symbol_table::find_function (err_name);
                  if (error_handler.is_undefined ())
                    {
                      error ("cellfun: invalid function name: %s", err_name.c_str ());
                      break;
                    }
                }
              else
                {
                  error ("invalid errorhandler value");
                  break;
                }
            }
          else
            {
              error ("cellfun: unrecognized parameter %s", 
                     arg.c_str());
              break;
            }

          nargin -= 2;
        }

      nargin -= 1;

      octave_value_list inputlist (nargin, octave_value ());

      OCTAVE_LOCAL_BUFFER (Cell, inputs, nargin);
      OCTAVE_LOCAL_BUFFER (bool, mask, nargin);

      // This is to prevent copy-on-write.
      const Cell *cinputs = inputs;

      octave_idx_type k = 1;

      dim_vector fdims (1, 1);

      if (error_state)
        goto cellfun_err;

      for (int j = 0; j < nargin; j++)
        {
          if (! args(j+1).is_cell ())
            {
              error ("cellfun: arguments must be cells");
              goto cellfun_err;
            }

          inputs[j] = args(j+1).cell_value ();
          mask[j] = inputs[j].numel () != 1;
          if (! mask[j])
            inputlist(j) = cinputs[j](0);
        }

      for (int j = 0; j < nargin; j++)
        {
          if (mask[j])
            {
              fdims = inputs[j].dims ();
              k = inputs[j].numel ();
              for (int i = j+1; i < nargin; i++)
                {
                  if (mask[i] && inputs[i].dims () != fdims)
                    {
                      error ("cellfun: Dimensions mismatch.");
                      goto cellfun_err;
                    }
                }
              break;
            }
        }

      if (error_handler.is_defined ())
        buffer_error_messages++;

      if (uniform_output)
        {
          OCTAVE_LOCAL_BUFFER (std::auto_ptr<scalar_col_helper>, retptr, nargout1);

          for (octave_idx_type count = 0; count < k ; count++)
            {
              for (int j = 0; j < nargin; j++)
                {
                  if (mask[j])
                    inputlist(j) = cinputs[j](count);
                }

              octave_value_list tmp = func.do_multi_index_op (nargout, inputlist);

              if (error_state && error_handler.is_defined ())
                {
                  Octave_map msg;
                  msg.assign ("identifier", last_error_id ());
                  msg.assign ("message", last_error_message ());
                  msg.assign ("index", octave_value(double (count + static_cast<octave_idx_type>(1))));
                  octave_value_list errlist = inputlist;
                  errlist.prepend (msg);
                  buffer_error_messages--;
                  error_state = 0;
                  tmp = error_handler.do_multi_index_op (nargout, errlist);
                  buffer_error_messages++;

                  if (error_state)
                    goto cellfun_err;
                }

              if (error_state)
                goto cellfun_err;

              if (tmp.length () < nargout1)
                {
                  if (tmp.length () < nargout)
                    {
                      error ("cellfun: too many output arguments");
                      goto cellfun_err;
                    }
                  else
                    nargout1 = 0;
                }

              if (count == 0)
                {
                  for (int j = 0; j < nargout1; j++)
                    {
                      octave_value val = tmp(j);

                      if (val.numel () == 1)
                        retptr[j].reset (make_col_helper (val, fdims));
                      else
                        {
                          error ("cellfun: expecting all values to be scalars for UniformOutput = true");
                          break;
                        }
                    }
                }
              else
                {
                  for (int j = 0; j < nargout1; j++)
                    {
                      octave_value val = tmp(j);

                      if (! retptr[j]->collect (count, val))
                        {
                          // FIXME: A more elaborate structure would allow again a virtual
                          // constructor here.
                          retptr[j].reset (new scalar_col_helper_def (retptr[j]->result (), 
                                                                      fdims));
                          retptr[j]->collect (count, val);
                        }
                    }
                }

              if (error_state)
                break;
            }

          retval.resize (nargout1);
          for (int j = 0; j < nargout1; j++)
            {
              if (retptr[j].get ())
                retval(j) = retptr[j]->result ();
              else
                retval(j) = Matrix ();
            }
        }
      else
        {
          OCTAVE_LOCAL_BUFFER (Cell, results, nargout1);
          for (int j = 0; j < nargout1; j++)
            results[j].resize (fdims);

          for (octave_idx_type count = 0; count < k ; count++)
            {
              for (int j = 0; j < nargin; j++)
                {
                  if (mask[j])
                    inputlist(j) = cinputs[j](count);
                }

              octave_value_list tmp = func.do_multi_index_op (nargout, inputlist);

              if (error_state && error_handler.is_defined ())
                {
                  Octave_map msg;
                  msg.assign ("identifier", last_error_id ());
                  msg.assign ("message", last_error_message ());
                  msg.assign ("index", octave_value(double (count + static_cast<octave_idx_type>(1))));
                  octave_value_list errlist = inputlist;
                  errlist.prepend (msg);
                  buffer_error_messages--;
                  error_state = 0;
                  tmp = error_handler.do_multi_index_op (nargout, errlist);
                  buffer_error_messages++;

                  if (error_state)
                    goto cellfun_err;
                }

              if (error_state)
                goto cellfun_err;

              if (tmp.length () < nargout1)
                {
                  if (tmp.length () < nargout)
                    {
                      error ("cellfun: too many output arguments");
                      goto cellfun_err;
                    }
                  else
                    nargout1 = 0;
                }


              for (int j = 0; j < nargout1; j++)
                results[j](count) = tmp(j);
            }

          retval.resize(nargout1);
          for (int j = 0; j < nargout1; j++)
            retval(j) = results[j];
        }

cellfun_err:
      if (error_state)
        retval = octave_value_list();

      unwind_protect::run_frame (uwp_frame);
    }
  else
    error ("cellfun: first argument must be a string or function handle");

  return retval;
}

/*

%% Test function to check the "Errorhandler" option
%!function [z] = cellfunerror (S, varargin)
%!    z = S;
%!  endfunction

%% First input argument can be a string, an inline function,
%% a function_handle or an anonymous function
%!test
%!  A = cellfun ("islogical", {true, 0.1, false, i*2});
%!  assert (A, [true, false, true, false]);
%!test
%!  A = cellfun (inline ("islogical (x)", "x"), {true, 0.1, false, i*2});
%!  assert (A, [true, false, true, false]);
%!test
%!  A = cellfun (@islogical, {true, 0.1, false, i*2});
%!  assert (A, [true, false, true, false]);
%!test
%!  A = cellfun (@(x) islogical(x), {true, 0.1, false, i*2});
%!  assert (A, [true, false, true, false]);

%% First input argument can be the special string "isreal",
%% "isempty", "islogical", "length", "ndims" or "prodofsize"
%!test
%!  A = cellfun ("isreal", {true, 0.1, false, i*2, [], "abc"});
%!  assert (A, [true, true, true, false, true, false]);
%!test
%!  A = cellfun ("isempty", {true, 0.1, false, i*2, [], "abc"});
%!  assert (A, [false, false, false, false, true, false]);
%!test
%!  A = cellfun ("islogical", {true, 0.1, false, i*2, [], "abc"});
%!  assert (A, [true, false, true, false, false, false]);
%!test
%!  A = cellfun ("length", {true, 0.1, false, i*2, [], "abc"});
%!  assert (A, [1, 1, 1, 1, 0, 3]);
%!test
%!  A = cellfun ("ndims", {[1, 2; 3, 4]; (cell (1,2,3,4))});
%!  assert (A, [2; 4]);
%!test
%!  A = cellfun ("prodofsize", {[1, 2; 3, 4], (cell (1,2,3,4))});
%!  assert (A, [4, 24]);

%% Number of input and output arguments may not be limited to one
%!test
%!  A = cellfun (@(x,y,z) x + y + z, {1, 1, 1}, {2, 2, 2}, {3, 4, 5});
%!  assert (A, [6, 7, 8]);
%!test
%!  A = cellfun (@(x,y,z) x + y + z, {1, 1, 1}, {2, 2, 2}, {3, 4, 5}, \
%!    "UniformOutput", false);
%!  assert (A, {6, 7, 8});
%!test %% Two input arguments of different types
%!  A = cellfun (@(x,y) islogical (x) && ischar (y), {false, true}, {"a", 3});
%!  assert (A, [true, false]);
%!test %% Pass another variable to the anonymous function
%!  y = true; A = cellfun (@(x) islogical (x) && y, {false, 0.3});
%!  assert (A, [true, false]);
%!test %% Three ouptut arguments of different type
%!  [A, B, C] = cellfun (@find, {10, 11; 0, 12}, "UniformOutput", false);
%!  assert (isequal (A, {true, true; [], true}));
%!  assert (isequal (B, {true, true; [], true}));
%!  assert (isequal (C, {10, 11; [], 12}));

%% Input arguments can be of type cell array of logical
%!test
%!  A = cellfun (@(x,y) x == y, {false, true}, {true, true});
%!  assert (A, [false, true]);
%!test
%!  A = cellfun (@(x,y) x == y, {false; true}, {true; true}, \
%!    "UniformOutput", true);
%!  assert (A, [false; true]);
%!test
%!  A = cellfun (@(x) x, {false, true; false, true}, "UniformOutput", false);
%!  assert (A, {false, true; false, true});
%!test %% Three ouptut arguments of same type
%!  [A, B, C] = cellfun (@find, {true, false; false, true}, \
%!    "UniformOutput", false);
%!  assert (isequal (A, {true, []; [], true}));
%!  assert (isequal (B, {true, []; [], true}));
%!  assert (isequal (C, {true, []; [], true}));
%!test
%!  A = cellfun (@(x,y) cell2str (x,y), {true}, {true}, \
%!    "ErrorHandler", @cellfunerror);
%!  assert (isfield (A, "identifier"), true);
%!  assert (isfield (A, "message"), true);
%!  assert (isfield (A, "index"), true);
%!  assert (isempty (A.message), false);
%!  assert (A.index, 1);
%!test %% Overwriting setting of "UniformOutput" true
%!  A = cellfun (@(x,y) cell2str (x,y), {true}, {true}, \
%!    "UniformOutput", true, "ErrorHandler", @cellfunerror);
%!  assert (isfield (A, "identifier"), true);
%!  assert (isfield (A, "message"), true);
%!  assert (isfield (A, "index"), true);
%!  assert (isempty (A.message), false);
%!  assert (A.index, 1);

%% Input arguments can be of type cell array of numeric
%!test
%!  A = cellfun (@(x,y) x>y, {1.1, 4.2}, {3.1, 2+3*i});
%!  assert (A, [false, true]);
%!test
%!  A = cellfun (@(x,y) x>y, {1.1, 4.2; 2, 4}, {3.1, 2; 2, 4+2*i}, \
%!    "UniformOutput", true);
%!  assert (A, [false, true; false, false]);
%!test
%!  A = cellfun (@(x,y) x:y, {1.1, 4}, {3.1, 6}, "UniformOutput", false);
%!  assert (isequal (A{1}, [1.1, 2.1, 3.1]));
%!  assert (isequal (A{2}, [4, 5, 6]));
%!test %% Three ouptut arguments of different type
%!  [A, B, C] = cellfun (@find, {10, 11; 0, 12}, "UniformOutput", false);
%!  assert (isequal (A, {true, true; [], true}));
%!  assert (isequal (B, {true, true; [], true}));
%!  assert (isequal (C, {10, 11; [], 12}));
%!test
%!  A = cellfun (@(x,y) cell2str(x,y), {1.1, 4}, {3.1, 6}, \
%!    "ErrorHandler", @cellfunerror);
%!  B = isfield (A(1), "message") && isfield (A(1), "index");
%!  assert ([(isfield (A(1), "identifier")), (isfield (A(2), "identifier"))], [true, true]);
%!  assert ([(isfield (A(1), "message")), (isfield (A(2), "message"))], [true, true]);
%!  assert ([(isfield (A(1), "index")), (isfield (A(2), "index"))], [true, true]);
%!  assert ([(isempty (A(1).message)), (isempty (A(2).message))], [false, false]);
%!  assert ([A(1).index, A(2).index], [1, 2]);
%!test %% Overwriting setting of "UniformOutput" true
%!  A = cellfun (@(x,y) cell2str(x,y), {1.1, 4}, {3.1, 6}, \
%!    "UniformOutput", true, "ErrorHandler", @cellfunerror);
%!  B = isfield (A(1), "message") && isfield (A(1), "index");
%!  assert ([(isfield (A(1), "identifier")), (isfield (A(2), "identifier"))], [true, true]);
%!  assert ([(isfield (A(1), "message")), (isfield (A(2), "message"))], [true, true]);
%!  assert ([(isfield (A(1), "index")), (isfield (A(2), "index"))], [true, true]);
%!  assert ([(isempty (A(1).message)), (isempty (A(2).message))], [false, false]);
%!  assert ([A(1).index, A(2).index], [1, 2]);

%% Input arguments can be of type cell arrays of character or strings
%!error %% "UniformOutput" false should be used
%!  A = cellfun (@(x,y) x>y, {"ad", "c", "ghi"}, {"cc", "d", "fgh"});
%!test
%!  A = cellfun (@(x,y) x>y, {"a"; "f"}, {"c"; "d"}, "UniformOutput", true);
%!  assert (A, [false; true]);
%!test
%!  A = cellfun (@(x,y) x:y, {"a", "d"}, {"c", "f"}, "UniformOutput", false);
%!  assert (A, {"abc", "def"});
%!test
%!  A = cellfun (@(x,y) cell2str(x,y), {"a", "d"}, {"c", "f"}, \
%!    "ErrorHandler", @cellfunerror);
%!  assert ([(isfield (A(1), "identifier")), (isfield (A(2), "identifier"))], [true, true]);
%!  assert ([(isfield (A(1), "message")), (isfield (A(2), "message"))], [true, true]);
%!  assert ([(isfield (A(1), "index")), (isfield (A(2), "index"))], [true, true]);
%!  assert ([(isempty (A(1).message)), (isempty (A(2).message))], [false, false]);
%!  assert ([A(1).index, A(2).index], [1, 2]);
%!test %% Overwriting setting of "UniformOutput" true
%!  A = cellfun (@(x,y) cell2str(x,y), {"a", "d"}, {"c", "f"}, \
%!    "UniformOutput", true, "ErrorHandler", @cellfunerror);
%!  assert ([(isfield (A(1), "identifier")), (isfield (A(2), "identifier"))], [true, true]);
%!  assert ([(isfield (A(1), "message")), (isfield (A(2), "message"))], [true, true]);
%!  assert ([(isfield (A(1), "index")), (isfield (A(2), "index"))], [true, true]);
%!  assert ([(isempty (A(1).message)), (isempty (A(2).message))], [false, false]);
%!  assert ([A(1).index, A(2).index], [1, 2]);

%% Structures cannot be handled by cellfun
%!error
%!  vst1.a = 1.1; vst1.b = 4.2; vst2.a = 3.1; vst2.b = 2;
%!  A = cellfun (@(x,y) (x.a < y.a) && (x.b > y.b), vst1, vst2);

%% Input arguments can be of type cell array of cell arrays
%!test
%!  A = cellfun (@(x,y) x{1} < y{1}, {{1.1}, {4.2}}, {{3.1}, {2}});
%!  assert (A, [1, 0], 1e-16);
%!test
%!  A = cellfun (@(x,y) x{1} < y{1}, {{1.1}; {4.2}}, {{3.1}; {2}}, \
%!    "UniformOutput", true);
%!  assert (A, [1; 0], 1e-16);
%!test
%!  A = cellfun (@(x,y) x{1} < y{1}, {{1.1}, {4.2}}, {{3.1}, {2}}, \
%!    "UniformOutput", false);
%!  assert (A, {true, false});
%!test
%!  A = cellfun (@(x,y) mat2str(x,y), {{1.1}, {4.2}}, {{3.1}, {2}}, \
%!    "ErrorHandler", @cellfunerror);
%!  assert ([(isfield (A(1), "identifier")), (isfield (A(2), "identifier"))], [true, true]);
%!  assert ([(isfield (A(1), "message")), (isfield (A(2), "message"))], [true, true]);
%!  assert ([(isfield (A(1), "index")), (isfield (A(2), "index"))], [true, true]);
%!  assert ([(isempty (A(1).message)), (isempty (A(2).message))], [false, false]);
%!  assert ([A(1).index, A(2).index], [1, 2]);
%!test %% Overwriting setting of "UniformOutput" true
%!  A = cellfun (@(x,y) mat2str(x,y), {{1.1}, {4.2}}, {{3.1}, {2}}, \
%!    "UniformOutput", true, "ErrorHandler", @cellfunerror);
%!  assert ([(isfield (A(1), "identifier")), (isfield (A(2), "identifier"))], [true, true]);
%!  assert ([(isfield (A(1), "message")), (isfield (A(2), "message"))], [true, true]);
%!  assert ([(isfield (A(1), "index")), (isfield (A(2), "index"))], [true, true]);
%!  assert ([(isempty (A(1).message)), (isempty (A(2).message))], [false, false]);
%!  assert ([A(1).index, A(2).index], [1, 2]);

%% Input arguments can be of type cell array of structure arrays
%!test
%!  a = struct ("a", 1, "b", 2); b = struct ("a", 1, "b", 3);
%!  A = cellfun (@(x,y) (x.a == y.a) && (x.b < y.b), {a}, {b});
%!  assert (A, true);
%!test
%!  a = struct ("a", 1, "b", 2); b = struct ("a", 1, "b", 3);
%!  A = cellfun (@(x,y) (x.a == y.a) && (x.b < y.b) , {a}, {b}, \
%!    "UniformOutput", true);
%!  assert (A, true);
%!test
%!  a = struct ("a", 1, "b", 2); b = struct ("a", 1, "b", 3);
%!  A = cellfun (@(x,y) (x.a == y.a) && (x.b < y.b) , {a}, {b}, \
%!    "UniformOutput", false);
%!  assert (A, {true});
%!test
%!  a = struct ("a", 1, "b", 2); b = struct ("a", 1, "b", 3);
%!  A = cellfun (@(x,y) cell2str (x.a, y.a), {a}, {b}, \
%!    "ErrorHandler", @cellfunerror);
%!  assert (isfield (A, "identifier"), true);
%!  assert (isfield (A, "message"), true);
%!  assert (isfield (A, "index"), true);
%!  assert (isempty (A.message), false);
%!  assert (A.index, 1);
%!test %% Overwriting setting of "UniformOutput" true
%!  a = struct ("a", 1, "b", 2); b = struct ("a", 1, "b", 3);
%!  A = cellfun (@(x,y) cell2str (x.a, y.a), {a}, {b}, \
%!    "UniformOutput", true, "ErrorHandler", @cellfunerror);
%!  assert (isfield (A, "identifier"), true);
%!  assert (isfield (A, "message"), true);
%!  assert (isfield (A, "index"), true);
%!  assert (isempty (A.message), false);
%!  assert (A.index, 1);

%% A lot of other tests
%!error(cellfun(1))
%!error(cellfun('isclass',1))
%!error(cellfun('size',1))
%!error(cellfun(@sin,{[]},'BadParam',false))
%!error(cellfun(@sin,{[]},'UniformOuput'))
%!error(cellfun(@sin,{[]},'ErrorHandler'))
%!assert(cellfun(@sin,{0,1}),sin([0,1]))
%!assert(cellfun(inline('sin(x)'),{0,1}),sin([0,1]))
%!assert(cellfun('sin',{0,1}),sin([0,1]))
%!assert(cellfun('isempty',{1,[]}),[false,true])
%!assert(cellfun('islogical',{false,pi}),[true,false])
%!assert(cellfun('isreal',{1i,1}),[false,true])
%!assert(cellfun('length',{zeros(2,2),1}),[2,1])
%!assert(cellfun('prodofsize',{zeros(2,2),1}),[4,1])
%!assert(cellfun('ndims',{zeros([2,2,2]),1}),[3,2])
%!assert(cellfun('isclass',{zeros([2,2,2]),'test'},'double'),[true,false])
%!assert(cellfun('size',{zeros([1,2,3]),1},1),[1,1])
%!assert(cellfun('size',{zeros([1,2,3]),1},2),[2,1])
%!assert(cellfun('size',{zeros([1,2,3]),1},3),[3,1])
%!assert(cellfun(@atan2,{1,1},{1,2}),[atan2(1,1),atan2(1,2)])
%!assert(cellfun(@atan2,{1,1},{1,2},'UniformOutput',false),{atan2(1,1),atan2(1,2)})
%!assert(cellfun(@sin,{1,2;3,4}),sin([1,2;3,4]))
%!assert(cellfun(@atan2,{1,1;1,1},{1,2;1,2}),atan2([1,1;1,1],[1,2;1,2]))
%!error(cellfun(@factorial,{-1,3}))
%!assert(cellfun(@factorial,{-1,3},'ErrorHandler',@(x,y) NaN),[NaN,6])
%!test
%! [a,b,c]=cellfun(@fileparts,{fullfile("a","b","c.d"),fullfile("e","f","g.h")},'UniformOutput',false);
%! assert(a,{fullfile("a","b"),fullfile("e","f")})
%! assert(b,{'c','g'})
%! assert(c,{'.d','.h'})

*/

static void
do_num2cell_helper (const dim_vector& dv,
                    const Array<int>& dimv,
                    dim_vector& celldv, dim_vector& arraydv,
                    Array<int>& perm)
{
  int dvl = dimv.length ();
  int maxd = dv.length ();
  celldv = dv;
  for (int i = 0; i < dvl; i++)
    maxd = std::max (maxd, dimv(i));
  if (maxd > dv.length ())
    celldv.resize (maxd, 1);
  arraydv = celldv;

  OCTAVE_LOCAL_BUFFER_INIT (bool, sing, maxd, false);

  perm.clear (maxd);
  for (int i = 0; i < dvl; i++)
    {
      int k = dimv(i) - 1;
      if (k < 0)
        {
          error ("num2cell: dimension indices must be positive");
          return;
        }
      else if (i > 0 && k < dimv(i-1) - 1)
        {
          error ("num2cell: dimension indices must be strictly increasing");
          return;
        }

      sing[k] = true;
      perm(i) = k;
    }

  for (int k = 0, i = dvl; k < maxd; k++)
    if (! sing[k])
      perm(i++) = k;

  for (int i = 0; i < maxd; i++)
    if (sing[i])
      celldv(i) = 1;
    else
      arraydv(i) = 1;
}

template<class NDA>
static Cell
do_num2cell (const NDA& array, const Array<int>& dimv)
{
  if (dimv.is_empty ())
    {
      Cell retval (array.dims ());
      octave_idx_type nel = array.numel ();
      for (octave_idx_type i = 0; i < nel; i++)
        retval.xelem (i) = array(i);

      return retval;
    }
  else
    {
      dim_vector celldv, arraydv;
      Array<int> perm;
      do_num2cell_helper (array.dims (), dimv, celldv, arraydv, perm);
      if (error_state)
        return Cell ();

      NDA parray = array.permute (perm);

      octave_idx_type nela = arraydv.numel (), nelc = celldv.numel ();
      parray = parray.reshape (dim_vector (nela, nelc));

      Cell retval (celldv);
      for (octave_idx_type i = 0; i < nelc; i++)
        {
          retval.xelem (i) = NDA (parray.column (i).reshape (arraydv));
        }

      return retval;
    }
}


DEFUN_DLD (num2cell, args, ,
  "-*- texinfo -*-\n\
@deftypefn  {Loadable Function} {@var{c} =} num2cell (@var{m})\n\
@deftypefnx {Loadable Function} {@var{c} =} num2cell (@var{m}, @var{dim})\n\
Convert the matrix @var{m} to a cell array.  If @var{dim} is defined, the\n\
value @var{c} is of dimension 1 in this dimension and the elements of\n\
@var{m} are placed in slices in @var{c}.\n\
@seealso{mat2cell}\n\
@end deftypefn") 
{
  int nargin =  args.length();
  octave_value retval;

  if (nargin < 1 || nargin > 2)
    print_usage ();
  else
    {
      octave_value array = args(0);
      Array<int> dimv;
      if (nargin > 1)
        dimv = args (1).int_vector_value (true);

      if (error_state)
        ;
      else if (array.is_bool_type ())
        retval = do_num2cell (array.bool_array_value (), dimv);
      else if (array.is_char_matrix ())
        retval = do_num2cell (array.char_array_value (), dimv);
      else if (array.is_numeric_type ())
        {
          if (array.is_integer_type ())
            {
              if (array.is_int8_type ())
                retval = do_num2cell (array.int8_array_value (), dimv);
              else if (array.is_int16_type ())
                retval = do_num2cell (array.int16_array_value (), dimv);
              else if (array.is_int32_type ())
                retval = do_num2cell (array.int32_array_value (), dimv);
              else if (array.is_int64_type ())
                retval = do_num2cell (array.int64_array_value (), dimv);
              else if (array.is_uint8_type ())
                retval = do_num2cell (array.uint8_array_value (), dimv);
              else if (array.is_uint16_type ())
                retval = do_num2cell (array.uint16_array_value (), dimv);
              else if (array.is_uint32_type ())
                retval = do_num2cell (array.uint32_array_value (), dimv);
              else if (array.is_uint64_type ())
                retval = do_num2cell (array.uint64_array_value (), dimv);
            }
          else if (array.is_complex_type ())
            {
              if (array.is_single_type ())
                retval = do_num2cell (array.float_complex_array_value (), dimv);
              else
                retval = do_num2cell (array.complex_array_value (), dimv);
            }
          else
            {
              if (array.is_single_type ())
                retval = do_num2cell (array.float_array_value (), dimv);
              else
                retval = do_num2cell (array.array_value (), dimv);
            }
        }
      else if (array.is_cell () || array.is_map ())
        {
          dim_vector celldv, arraydv;
          Array<int> perm;
          do_num2cell_helper (array.dims (), dimv, celldv, arraydv, perm);

          if (! error_state)
            {
              // FIXME: this operation may be rather inefficient.
              octave_value parray = array.permute (perm);

              octave_idx_type nela = arraydv.numel (), nelc = celldv.numel ();
              parray = parray.reshape (dim_vector (nela, nelc));

              Cell retcell (celldv);
              octave_value_list idx (2);
              idx(0) = octave_value::magic_colon_t;

              for (octave_idx_type i = 0; i < nelc; i++)
                {
                  idx(1) = i + 1;
                  octave_value tmp = parray.do_index_op (idx);
                  retcell(i) = tmp.reshape (arraydv);
                }

              retval = retcell;
            }
        }
      else
        gripe_wrong_type_arg ("num2cell", array);
    }

  return retval;
}

/*

%!assert(num2cell([1,2;3,4]),{1,2;3,4})
%!assert(num2cell([1,2;3,4],1),{[1;3],[2;4]})
%!assert(num2cell([1,2;3,4],2),{[1,2];[3,4]})

*/

DEFUN_DLD (mat2cell, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{b} =} mat2cell (@var{a}, @var{m}, @var{n})\n\
@deftypefnx {Loadable Function} {@var{b} =} mat2cell (@var{a}, @var{d1}, @var{d2}, @dots{})\n\
@deftypefnx {Loadable Function} {@var{b} =} mat2cell (@var{a}, @var{r})\n\
Convert the matrix @var{a} to a cell array.  If @var{a} is 2-D, then\n\
it is required that @code{sum (@var{m}) == size (@var{a}, 1)} and\n\
@code{sum (@var{n}) == size (@var{a}, 2)}.  Similarly, if @var{a} is\n\
a multi-dimensional and the number of dimensional arguments is equal\n\
to the dimensions of @var{a}, then it is required that @code{sum (@var{di})\n\
== size (@var{a}, i)}.\n\
\n\
Given a single dimensional argument @var{r}, the other dimensional\n\
arguments are assumed to equal @code{size (@var{a},@var{i})}.\n\
\n\
An example of the use of mat2cell is\n\
\n\
@example\n\
mat2cell (reshape(1:16,4,4),[3,1],[3,1])\n\
@result{} @{\n\
  [1,1] =\n\
\n\
     1   5   9\n\
     2   6  10\n\
     3   7  11\n\
\n\
  [2,1] =\n\
\n\
     4   8  12\n\
\n\
  [1,2] =\n\
\n\
    13\n\
    14\n\
    15\n\
\n\
  [2,2] = 16\n\
@}\n\
@end example\n\
@seealso{num2cell, cell2mat}\n\
@end deftypefn")
{
  int nargin = args.length();
  octave_value retval;

  if (nargin < 2)
    print_usage ();
  else
    {
      dim_vector dv = args(0).dims();
      dim_vector new_dv;
      new_dv.resize(dv.length());
      
      if (nargin > 2)
	{
	  octave_idx_type nmax = -1;

	  if (nargin - 1 != dv.length())
	    error ("mat2cell: Incorrect number of dimensions");
	  else
	    {
	      for (octave_idx_type j = 0; j < dv.length(); j++)
		{
		  ColumnVector d = ColumnVector (args(j+1).vector_value 
						 (false, true));

		  if (d.length() < 1)
		    {
		      error ("mat2cell: dimension can not be empty");
		      break;
		    }
		  else
		    {
		      if (nmax < d.length())
			nmax = d.length();

		      for (octave_idx_type i = 1; i < d.length(); i++)
			{
			  OCTAVE_QUIT;

			  if (d(i) >= 0)
			    d(i) += d(i-1);
			  else
			    {
			      error ("mat2cell: invalid dimensional argument");
			      break;
			    }
			}

		      if (d(0) < 0)
			error ("mat2cell: invalid dimensional argument");
		      
		      if (d(d.length() - 1) != dv(j))
			error ("mat2cell: inconsistent dimensions");

		      if (error_state)
			break;

		      new_dv(j) = d.length();
		    }
		}
	    }

	  if (! error_state)
	    {
	      // Construct a matrix with the index values
	      Matrix dimargs(nmax, new_dv.length());
	      for (octave_idx_type j = 0; j < new_dv.length(); j++)
		{
		  OCTAVE_QUIT;

		  ColumnVector d = ColumnVector (args(j+1).vector_value 
						 (false, true));

		  dimargs(0,j) = d(0);
		  for (octave_idx_type i = 1; i < d.length(); i++)
		    dimargs(i,j) = dimargs(i-1,j) + d(i);
		}


	      octave_value_list lst (new_dv.length(), octave_value());
	      Cell ret (new_dv);
	      octave_idx_type nel = new_dv.numel();
	      octave_idx_type ntot = 1;

	      for (int j = 0; j < new_dv.length()-1; j++)
		ntot *= new_dv(j);

	      for (octave_idx_type i = 0; i <  nel; i++)
		{
		  octave_idx_type n = ntot;
		  octave_idx_type ii = i;
		  for (octave_idx_type j =  new_dv.length() - 1;  j >= 0; j--)
		    {
		      OCTAVE_QUIT;
		  
		      octave_idx_type idx = ii / n;
		      lst (j) = Range((idx == 0 ? 1. : dimargs(idx-1,j)+1.),
				      dimargs(idx,j));
		      ii = ii % n;
		      if (j != 0)
			n /= new_dv(j-1);
		    }
		  ret(i) = args(0).do_index_op(lst, 0);
		  if (error_state)
		    break;
		}
	  
	      if (!error_state)
		retval = ret;
	    }
	}
      else
	{
	  ColumnVector d = ColumnVector (args(1).vector_value 
					 (false, true));

	  double sumd = 0.;
	  for (octave_idx_type i = 0; i < d.length(); i++)
	    {
	      OCTAVE_QUIT;

	      if (d(i) >= 0)
		sumd += d(i);
	      else
		{
		  error ("mat2cell: invalid dimensional argument");
		  break;
		}
	    }

	  if (sumd != dv(0))
	    error ("mat2cell: inconsistent dimensions");

	  new_dv(0) = d.length();
	  for (octave_idx_type i = 1; i < dv.length(); i++)
	    new_dv(i) = 1;

	  if (! error_state)
	    {
	      octave_value_list lst (new_dv.length(), octave_value());
	      Cell ret (new_dv);

	      for (octave_idx_type i = 1; i < new_dv.length(); i++)
		lst (i) = Range (1., static_cast<double>(dv(i)));
	      
	      double idx = 0.;
	      for (octave_idx_type i = 0; i <  new_dv(0); i++)
		{
		  OCTAVE_QUIT;

		  lst(0) = Range(idx + 1., idx + d(i));
		  ret(i) = args(0).do_index_op(lst, 0);
		  idx += d(i);
		  if (error_state)
		    break;
		}
	  
	      if (!error_state)
		retval = ret;
	    }
	}
    }

  return retval;
}

/*

%!test
%! x = reshape(1:20,5,4);
%! c = mat2cell(x,[3,2],[3,1]);
%! assert(c,{[1,6,11;2,7,12;3,8,13],[16;17;18];[4,9,14;5,10,15],[19;20]})

%!test
%! x = 'abcdefghij';
%! c = mat2cell(x,1,[0,4,2,0,4,0]);
%! empty1by0str = resize('',1,0);
%! assert(c,{empty1by0str,'abcd','ef',empty1by0str,'ghij',empty1by0str})

*/

// FIXME: it would be nice to allow ranges being handled without a conversion.
template <class NDA>
static Cell 
do_cellslices_nda (const NDA& array, 
                   const Array<octave_idx_type>& lb, 
                   const Array<octave_idx_type>& ub)
{
  octave_idx_type n = lb.length ();
  Cell retval (1, n);
  if (array.is_vector ())
    {
      for (octave_idx_type i = 0; i < n && ! error_state; i++)
        retval(i) = array.index (idx_vector (lb(i) - 1, ub(i)));
    }
  else
    {
      dim_vector dv = array.dims ();
      octave_idx_type nl = 1;
      for (int i = 0; i < dv.length () - 1; i++) nl *= dv(i);
      for (octave_idx_type i = 0; i < n && ! error_state; i++)
        {
          // Do it with a single index to speed things up.
          dv = array.dims ();
          dv(dv.length () - 1) = ub(i) + 1 - lb(i);
          dv.chop_trailing_singletons ();
          retval(i) = array.index (idx_vector (nl*(lb(i) - 1), nl*ub(i))).reshape (dv);
        }
    }

  return retval;
}

DEFUN_DLD (cellslices, args, ,
  "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{sl} =} cellslices (@var{x}, @var{lb}, @var{ub})\n\
Given a vector @var{x}, this function produces a cell array of slices from the vector\n\
determined by the index vectors @var{lb}, @var{ub}, for lower and upper bounds, respectively.\n\
In other words, it is equivalent to the following code:\n\
\n\
@example\n\
@group\n\
n = length (lb);\n\
sl = cell (1, n);\n\
for i = 1:length (lb)\n\
  sl@{i@} = x(lb(i):ub(i));\n\
endfor\n\
@end group\n\
@end example\n\
\n\
If @var{X} is a matrix or array, indexing is done along the last dimension.\n\
@seealso{mat2cell}\n\
@end deftypefn")
{
  octave_value retval;
  int nargin = args.length ();
  if (nargin == 3)
    {
      octave_value x = args(0);
      Array<octave_idx_type> lb = args(1).octave_idx_type_vector_value ();
      Array<octave_idx_type> ub = args(2).octave_idx_type_vector_value ();
      if (! error_state)
        {
          if (lb.length () != ub.length ())
            error ("cellslices: the lengths of lb and ub must match");
          else
            {
              Cell retcell;
              if (! x.is_sparse_type () && x.is_matrix_type ())
                {
                  // specialize for some dense arrays.
                  if (x.is_bool_type ())
                    retcell = do_cellslices_nda (x.bool_array_value (), lb, ub);
                  else if (x.is_char_matrix ())
                    retcell = do_cellslices_nda (x.char_array_value (), lb, ub);
                  else if (x.is_integer_type ())
                    {
                      if (x.is_int8_type ())
                        retcell = do_cellslices_nda (x.int8_array_value (), lb, ub);
                      else if (x.is_int16_type ())
                        retcell = do_cellslices_nda (x.int16_array_value (), lb, ub);
                      else if (x.is_int32_type ())
                        retcell = do_cellslices_nda (x.int32_array_value (), lb, ub);
                      else if (x.is_int64_type ())
                        retcell = do_cellslices_nda (x.int64_array_value (), lb, ub);
                      else if (x.is_uint8_type ())
                        retcell = do_cellslices_nda (x.uint8_array_value (), lb, ub);
                      else if (x.is_uint16_type ())
                        retcell = do_cellslices_nda (x.uint16_array_value (), lb, ub);
                      else if (x.is_uint32_type ())
                        retcell = do_cellslices_nda (x.uint32_array_value (), lb, ub);
                      else if (x.is_uint64_type ())
                        retcell = do_cellslices_nda (x.uint64_array_value (), lb, ub);
                    }
                  else if (x.is_complex_type ())
                    {
                      if (x.is_single_type ())
                        retcell = do_cellslices_nda (x.float_complex_array_value (), lb, ub);
                      else
                        retcell = do_cellslices_nda (x.complex_array_value (), lb, ub);
                    }
                  else
                    {
                      if (x.is_single_type ())
                        retcell = do_cellslices_nda (x.float_array_value (), lb, ub);
                      else
                        retcell = do_cellslices_nda (x.array_value (), lb, ub);
                    }
                }
              else
                {
                  // generic code.
                  octave_idx_type n = lb.length ();
                  retcell = Cell (1, n);
                  octave_idx_type nind = x.dims ().is_vector () ? 1 : x.ndims ();
                  octave_value_list idx (nind, octave_value::magic_colon_t);
                  for (octave_idx_type i = 0; i < n && ! error_state; i++)
                    {
                      idx(nind-1) = Range (lb(i), ub(i));
                      retcell(i) = x.do_index_op (idx);
                    }
                }
              if (! error_state)
                retval = retcell;
            }
        }
    }
  else
    print_usage ();

  return retval;
}
	  
/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
