# Copyright (C) 1996 A. Scottedward Hodel 
#
# This file is part of Octave. 
#
# Octave is free software; you can redistribute it and/or modify it 
# under the terms of the GNU General Public License as published by the 
# Free Software Foundation; either version 2, or (at your option) any 
# later version. 
# 
# Octave is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License 
# for more details.
# 
# You should have received a copy of the GNU General Public License 
# along with Octave; see the file COPYING.  If not, write to the Free 
# Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 
function prompt(str)
# function prompt([str])
# Prompt user to continue
# str: input string. Default value: "\n ---- Press a key to continue ---"
# Written by David Clem August 15, 1994
# Modified A. S. Hodel June 1995


if(nargin > 1)
  usage("prompt([str])");
elseif(nargin == 0)
  str = "\n ---- Press a key to continue ---";
elseif ( !isstr(str) )
  error("prompt: input must be a string");
endif

disp(str);
fflush(stdout);
kbhit;

endfunction
