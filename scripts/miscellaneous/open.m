## Copyright (C) 2014-2018 John W. Eaton
##
## This file is part of Octave.
##
## Octave is free software: you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <https://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn  {} {} open @var{file}
## @deftypefnx {} {@var{output} =} open (@var{file})
## Open the file @var{file} in Octave or in an external application based on
## the file type as determined by the filename extension.
##
## By default, recognized file types are
##
## @table @code
## @item .m
## Open file in the editor. No @var{output} value is returned.
##
## @item .mat
## @item octave-workspace
## Open the data file with @code{load}. If no return value @var{output}
## is requested, variables are loaded in the base workspace. Otherwise
## @var{output} will be a structure containing loaded data.
## @xref{XREFload, , load function}.
##
## @item .ofig
## Open the figure with hgload.  @xref{XREFhgload, , hgload function}.
##
## @item .exe
## Execute the program (on Windows systems only). No @var{output} value
## is returned.
## @end table
##
## Custom file extensions may also be handled if a function @code{openxxx},
## where @code{xxx} is the extension, is found in the load path.  The function
## must accept the file name as input.  For example, in order to load ".dat"
## data files in the base workspace, as is done by default for ".mat" files, one
## may define "opendat.m" with the following contents:
## @example
## function retval = opendat (fname)
##   evalin ("base", sprintf ("load ('%s');", fname));
## endfunction
## @end example
##
## Other file types are opened in the appropriate external application.
## @end deftypefn

function output = open (file)

  if (nargin != 1)
    print_usage ();
  elseif (! exist (file, "file"))
    error ("open: unable to find file %s", file);
  endif

  if (! ischar (file))
    error ("open: FILE must be a string");
  endif

  [~, fname, ext] = fileparts (file);

  if (! isempty (ext)
      && any (exist (["open" tolower(ext(2:end))]) == [2 3 5 103]))
    try
      feval (["open" tolower(ext(2:end))], file)
    catch
      error ("open: %s", lasterr);
    end_try_catch
  elseif (strcmpi (ext, ".m"))
    edit (file);
  elseif (strcmpi (ext, ".mat") || strcmp (fname, "octave-workspace"))
    if (nargout > 0)
      output = load (file);
    else
      evalin ("base", sprintf ("load ('%s');", file));
    endif
  elseif (strcmpi (ext, ".ofig"))
    output = hgload (file);
    drawnow ();
  elseif (any (strcmpi (ext, {".mdl", ".slx", ".prj"})))
    error ("open: opening file type '%s' is not supported", ext);
  elseif (strcmpi (ext, ".exe"))
    if (ispc ())
      dos (file);
    else
      error ("open: executing .exe files is only supported on Windows systems");
    endif
  else
    __open_with_system_app__ (file);
  endif

endfunction


## Test input validation
%!error open ()
%!error open ("abc", "def")
%!error <FILE must be a string> open (1)
