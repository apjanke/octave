## Copyright (C) 2017 Rik Wehbring
## Copyright (C) 2000-2017 Paul Kienzle
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {} {} __isequal__ (@var{nans_compare_equal}, @var{x}, @var{y}, @dots{})
## Internal function.
##
## Return true if @var{x}, @var{y}, @dots{} are all equal and
## @var{nans_compare_equal} evaluates to false.
##
## If @var{nans_compare_equal} evaluates to true, then assume NaN == NaN.
## @end deftypefn

## Algorithm:
##
## 1. Verify the class of x.
##    a. All objects are of the same class
##    b. All objects are of a generic "numeric" class which includes
##       numeric, logical, and character arrays
## 2. Verify size of all objects match.
## 3. Convert objects to struct, and then compare as stated below.
## 4. For each argument after x, compare it for equality with x:
##    a. char       compare each member with strcmp
##    b. numeric    compare each member with '==', and assume NaN == NaN
##                  if nans_compare_equal is nonzero.
##    c. struct     compare number of fieldnames, value of fieldnames,
##                  and then each field with __isequal__ (recursive)
##    d. cellstr    compare each cellstr member with strcmp
##    e. cell       compare each member with __isequal__ (recursive)
##    f. fcn_handle compare using overloaded "eq" operator

function t = __isequal__ (nans_compare_equal, x, varargin)

  nvarargin = nargin - 2;
  two_args = (nvarargin == 1);  # Optimization for base case of just 2 args

  if (two_args)
    y = varargin{1};  # alias y to second input for comparison
  endif

  ############################################################
  ## Generic tests for equality

  ## All arguments must either be of the same class,
  ##  or they must be "numeric" values.
  if (two_args)
    t = (strcmp (class (x), class (y))
         || ((isreal (x) || iscomplex (x)) && (isreal (y) || iscomplex (y))));
  else
    t = (all (cellfun ("isclass", varargin, class (x)))
         || ((isreal (x) || iscomplex (x))
             && all (cellfun ("isreal", varargin)
                     | cellfun ("isnumeric", varargin))));
  endif

  ## Test that everything is the same size (which also tests dimensions)
  if (t)
    t = size_equal (x, varargin{:});
  endif

  ## From here on, compare any objects as if they were structures.
  if (t && isobject (x))
    ## Locally suppress class-to-struct warning.  We know what we are doing.
    warning ("off", "Octave:classdef-to-struct", "local");
    x = builtin ("struct", x);
    for i = 1:nvarargin
      varargin(i) = builtin ("struct", varargin{i});
    endfor
  endif

  ############################################################
  ## Check individual classes.

  if (t)
    if (two_args)

      if (ischar (x) && ischar (y))
        ## char type.  Optimization, strcmp is ~35% faster than '==' operator.
        t = strcmp (x, y);

      elseif (isreal (x) || iscomplex (x))
        ## general "numeric" type.  Use '==' operator.
        m = (x == y);
        t = all (m(:));

        if (! t && nans_compare_equal && isfloat (x) && isfloat (y))
          t = isnan (x(! m)) && isnan (y(! m));
        endif

      elseif (isstruct (x))
        ## struct type.  Compare # of fields, fieldnames, then field values.

        ## Test number of fields are equal.
        t = (numfields (x) == numfields (y));

        ## Test that all the field names are equal.
        if (t)
          s_fnm_x = sort (fieldnames (x));
          t = all (strcmp (s_fnm_x, sort (fieldnames (y))));
        endif

        ## Test that all field values are equal.  Slow because of recursion.
        if (t)
          for fldnm = s_fnm_x.'
            t = __isequal__ (nans_compare_equal, x.(fldnm{1}), y.(fldnm{1}));
            if (! t)
              break;
            endif
          endfor
        endif

      elseif (iscellstr (x) && iscellstr (y))
        ## cellstr type.  Optimization over cell type by using strcmp.
        ## FIXME: It would be faster to use strcmp on whole cellstr arrays,
        ## but bug #51412 needs to be fixed.  Instead, time/space trade-off.
        ## Convert to char (space) for faster processing with strcmp (time).
        t = strcmp (char (x), char (y));

      elseif (iscell (x))
        ## cell type.  Check that each element of a cell is equal.  Slow.
        n = numel (x);
        idx = 1;
        while (t && idx <= n)
          t = __isequal__ (nans_compare_equal, x{idx}, y{idx});
          idx += 1;
        endwhile

      elseif (isa (x, "function_handle"))
        ## function type.  Use '==' operator which is overloaded.
        t = (x == y);

      else
        error ("__isequal__: Impossible to reach code.  File a bug report."); 

      endif

    else  ## More than two args.  This is going to be slower in general.

      if (ischar (x) && all (cellfun ("isclass", varargin, "char")))
        ## char type.  Optimization, strcmp is ~35% faster than '==' operator.
        idx = 1;
        while (t && idx <= nvarargin)
          t = strcmp (x, varargin{idx});
          idx += 1;
        endwhile

      elseif (isreal (x) || iscomplex (x))
        ## general "numeric" type.  Use '==' operator.

        idx = 1;
        while (t && idx <= nvarargin)
          y = varargin{idx};
          m = (x == y);
          t = all (m(:));

          if (! t && nans_compare_equal && isfloat (x) && isfloat (y))
            t = isnan (x(! m)) && isnan (y(! m));
          endif

          idx += 1;
        endwhile

      elseif (isstruct (x))
        ## struct type.  Compare # of fields, fieldnames, then field values.

        ## Test number of fields are equal.
        fnm_x = fieldnames (x);
        n = numel (fnm_x);
        fnm_v = cellfun ("fieldnames", varargin, "uniformoutput", false);
        t = all (n == cellfun ("numel", fnm_v));

        ## Test that all the field names are equal.
        if (t)
          fnm_x = sort (fnm_x);
          idx = 1;
          while (t && idx <= nvarargin)
            ## Allow the fieldnames to be in a different order.
            t = all (strcmp (fnm_x, sort (fnm_v{idx})));
            idx += 1;
          endwhile
        endif

        ## Test that all field values are equal.  Slow because of recursion.
        if (t)
          args = cell (1, 2 + nvarargin);
          args(1) = nans_compare_equal;
          for fldnm = fnm_x.'
            args(2) = x.(fldnm{1});
            for argn = 1:nvarargin
              args(argn+2) = varargin{argn}.(fldnm{1});
            endfor

            t = __isequal__ (args{:});

            if (! t)
              break;
            endif
          endfor
        endif

      elseif (iscellstr (x) && all (cellfun (@iscellstr, varargin)))
        ## cellstr type.  Optimization over cell type by using strcmp.
        ## FIXME: It would be faster to use strcmp on whole cellstr arrays,
        ## but bug #51412 needs to be fixed.  Instead, time/space trade-off.
        ## Convert to char (space) for faster processing with strcmp (time).
        idx = 1;
        x = char (x);
        while (t && idx <= nvarargin) 
          t = strcmp (x, char (varargin{idx}));
          idx += 1;
        endwhile

      elseif (iscell (x))
        ## cell type.  Check that each element of a cell is equal.  Slow.
        n = numel (x);
        args = cell (1, 2 + nvarargin);
        args(1) = nans_compare_equal;
        idx = 1;
        while (t && idx <= n)
          args(2) = x{idx};
          args(3:end) = [cellindexmat(varargin, idx){:}];

          t = __isequal__ (args{:});

          idx += 1;
        endwhile

      elseif (isa (x, "function_handle"))
        ## function type.  Use '==' operator which is overloaded.
        t = all (cellfun ("eq", {x}, varargin));

      else
        error ("__isequal__: Impossible to reach code.  File a bug report."); 

      endif

    endif
  endif

endfunction
