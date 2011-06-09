## Copyright (C) 2008-2011 David Bateman
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
## @deftypefn  {Mapping Function} {} spmax (@var{x}, @var{y}, @var{dim})
## @deftypefnx {Mapping Function} {[@var{w}, @var{iw}] =} spmax (@var{x})
## This function has been deprecated.  Use @code{max} instead.
## @end deftypefn

## Deprecated in version 3.2

function varargout = spmax (varargin)
  persistent warned = false;
  if (! warned)
    warned = true;
    warning ("Octave:deprecated-function",
             "spmax is obsolete and will be removed from a future version of Octave; please use max instead");
  endif

  varargout = cell (nargout, 1);
  [ varargout{:} ] = max (varargin{:});

endfunction
