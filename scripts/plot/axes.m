## Copyright (C) 2005, 2006, 2007 John W. Eaton
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
## @deftypefn {Function File} {} axes ()
## @deftypefnx {Function File} {} axes (@var{property}, @var{value}, @dots{})
## @deftypefnx {Function File} {} axes (@var{h})
## Create an axes object and return a handle to it.
## @end deftypefn

## Author: jwe

function h = axes (varargin)

  if (nargin == 0 || nargin > 1)
    ## make default axes object, and make it the current axes for the
    ## current figure.
    idx = find (strcmpi (varargin(1:2:end), "parent"), 1, "first");
    if (! isempty (idx) && length (varargin) >= 2*idx)
      cf = varargin{2*idx};
      varargin([2*idx-1, 2*idx]) = [];
    else
      cf = gcf ();
    endif
    tmp = __go_axes__ (cf, varargin{:});
    set (ancestor (cf, "figure"), "currentaxes", tmp);
  else
    ## arg is axes handle, make it the current axes for the current
    ## figure.
    tmp = varargin{1};
    if (ishandle (tmp) && strcmp (get (tmp, "type"), "axes"))
      parent = ancestor (tmp, "figure");
      set (0, "currentfigure", parent);
      set (parent, "currentaxes", tmp);
    else
      error ("axes: expecting argument to be axes handle");
    endif
  endif

  if (nargout > 0)
    h = tmp;
  endif

endfunction
