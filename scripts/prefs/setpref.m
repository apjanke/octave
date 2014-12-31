## Copyright (C) 2012-2013 John W. Eaton
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or (at
## your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {Function File} {} setpref (@var{group}, @var{pref}, @var{val})
## Set a preference @var{pref} to the given @var{val} in the named
## preference group @var{group}.
##
## The named preference group must be a character string.
##
## The preference @var{pref} may be a character string or a cell array
## of character strings.  The corresponding value @var{val} may be any
## value, or, if @var{pref} is a cell array of strings, @var{val}
## must be a cell array of values with the same size as @var{pref}.
##
## If the named preference or group does not exist, it is added.
## @seealso{addpref, getpref, ispref, rmpref}
## @end deftypefn

## Author: jwe

function setpref (group, pref, val)

  if (nargin == 3)
    if (ischar (group))
      prefs = loadprefs ();
      if (ischar (pref))
        prefs.(group).(pref) = val;
      elseif (iscellstr (pref))
        if (size_equal (pref, val))
          for i = 1:numel (pref)
            prefs.(group).(pref{i}) = val;
          endfor
        else
          error ("size mismatch for pref and val");
        endif
      else
        error ("expecting pref to be a character string or cellstr");
      endif
      saveprefs (prefs);
    else
      error ("expecting group to be a character string");
    endif
  else
    print_usage ();
  endif

endfunction


%% Testing these functions will require some care to avoid wiping out
%% existing (or creating unwanted) preferences for the user running the
%% tests.

