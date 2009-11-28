## Copyright (C) 2009 David Bateman
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; If not, see <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {Function File} {@var{lst} =} dir (@var{path})
## List the current directory in verbose form for the FTP connection 
## @var{f}. @var{f} is an FTP object returned by the ftp function.
## @end deftypefn

function lst = dir (obj)
  if (nargout == 0)
    __ftp_dir__ (obj.curlhandle);
  else
    lst = __ftp_dir__ (obj.curlhandle);
  endif
endfunction
