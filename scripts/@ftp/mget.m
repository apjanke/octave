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
## @deftypefn {Function File} {} mget (@var{f}, @var{file})
## @deftypefnx {Function File} {} mget (@var{f}, @var{dir})
## @deftypefnx {Function File} {} mget (@dots{}, @var{target})
## Downloads a remote file @var{file} or directory @var{dir} to the local
## directory on the FTP connection @var{f}.  @var{f} is an FTP object
## returned by the ftp function. 
##
## The arguments @var{file} and @var{dir} can include wildcards and any
## files or directories on the remote server that match will be downloaded.
##
## If a third argument @var{target} is given, then a single file or
## directory will be downloaded with the name @var{target} to the local
## directory.
## @end deftypefn

function mget (obj, file)
  __ftp_mget__ (obj.curlhandle, file);
endfunction
