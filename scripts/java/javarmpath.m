## Copyright (C) 2007 Michael Goffioul
## Copyright (C) 2010 Martin Hepperle
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
## @deftypefn {Function file} {} javarmpath (@var{path})
## Remove @var{path} from the dynamic class path of the Java virtual
## machine.  @var{path} may be either a directory where @file{.class}
## files are found, or a @file{.jar} file containing Java classes.
## @seealso{javaaddpath, javaclasspath}
## @end deftypefn

function javarmpath (class_path)

  if (nargin != 1)
    print_usage ();
  else
    old_path = canonicalize_file_name (tilde_expand (class_path));
    if (exist (old_path, "dir"))
      if (! strcmp (old_path (end), filesep))
        old_path = [old_path, filesep];
      endif
    endif

    success = java_invoke ("org.octave.ClassHelper", "removeClassPath",
                           old_path);

    if (! success)
      warning ("javarmpath: %s: not found in Java classpath", old_path);
    endif
  endif

endfunction
