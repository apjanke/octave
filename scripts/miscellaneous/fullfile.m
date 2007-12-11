## Copyright (C) 2003, 2005, 2006, 2007 John W. Eaton
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
## @deftypefn {Function File} {@var{filename} =} fullfile (@var{dir1}, @var{dir2}, @dots{}, @var{file})
## Return a complete filename constructed from the given components.
## @seealso{fileparts}
## @end deftypefn

function filename = fullfile (varargin)

  if (nargin > 0)
    filename = "";
    for first = 1:nargin
      tmp = varargin{first};
      if (! isempty (tmp))
	filename = tmp;
	break;
      endif
    endfor
    for i = first+1:nargin
      tmp = varargin{i};
      if (! isempty (tmp))
	if (strcmp (tmp(1), filesep))
	  tmp(1) = "";
	endif
	if (i < nargin && strcmp (tmp(end), filesep))
	  tmp(end) = "";
        endif
        filename = strcat (filename, filesep, tmp);
      endif
    endfor
  else
    print_usage ();
  endif

endfunction

%!assert (fullfile (""), "")
%!assert (fullfile (filesep ()), filesep ())
%!assert (fullfile ("", filesep ()), filesep ())
%!assert (fullfile (filesep (), ""), filesep ())
%!assert (fullfile ("", filesep ()), filesep ())
%!assert (fullfile ("foo"), "foo")
%!assert (fullfile ("", "foo"), "foo")
%!assert (fullfile ("foo", ""), "foo")
%!assert (fullfile ("", "foo", ""), "foo")
%!assert (fullfile ("foo", "bar"), strcat ("foo", filesep (), "bar"))
%!assert (fullfile ("foo", "", "bar"), strcat ("foo", filesep (), "bar"))
%!assert (fullfile ("foo", "", "bar", ""), strcat ("foo", filesep (), "bar"))
%!assert (fullfile ("", "foo", "", "bar", ""), strcat ("foo", filesep (), "bar"))
