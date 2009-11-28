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

function b = loadobj (a)
  b = a;
  b.curlhandle = tmpnam ("ftp-");
  __ftp__ (b.curlhandle, b.host, b.user, b.pass);
  if (! isempty (b.dir))
    __ftp_cwd__ (b.curlhandle, b.dir);
  endif
  b = rmfield (b, "dir")
endfunction
