## Copyright (C) 2008-2016 John W. Eaton
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
## @deftypefn  {} {[@var{prog}, @var{args}] =} gnuplot_binary ()
## @deftypefnx {} {[@var{old_prog}, @var{old_args}] =} gnuplot_binary (@var{new_prog}, @var{arg1}, @dots{})
## Query or set the name of the program invoked by the plot command when the
## graphics toolkit is set to @qcode{"gnuplot"}.
##
## Additional arguments to pass to the external plotting program may also be
## given.  The default value is @qcode{"gnuplot"} with no additional arguments.
## @xref{Installation}.
## @seealso{graphics_toolkit}
## @end deftypefn

## Author: jwe

function [prog, args] = gnuplot_binary (new_prog, varargin)

  mlock ()
  persistent gp_binary = %OCTAVE_CONF_GNUPLOT%;
  persistent gp_args = {};

  if (nargout > 0 || nargin == 0)
    prog = gp_binary;
    args = gp_args;
  endif

  if (nargin == 1)
    if (! ischar (new_prog) || isempty (new_prog))
      error ("gnuplot_binary: NEW_PROG must be a non-empty string");
    endif
    gp_binary = new_prog;
    __gnuplot_has_feature__ ("__reset__");
  endif

  if (nargin > 1)
    if (! iscellstr (varargin))
      error ("gnuplot_binary: arguments must be character strings");
    endif
    gp_args = varargin;
  endif

endfunction


%!test
%! orig_val = gnuplot_binary ();
%! old_val = gnuplot_binary ("X");
%! assert (orig_val, old_val);
%! assert (gnuplot_binary (), "X");
%! gnuplot_binary (orig_val);
%! assert (gnuplot_binary (), orig_val);
