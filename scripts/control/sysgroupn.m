# Copyright (C) 1996,1998 A. Scottedward Hodel 
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
 
function names = sysgroupn(names,kind)
# names = sysgroupn(names)
# locate and mark duplicate names
# inputs:
#   names: list of signal names
#   kind: kind of signal name (used for diagnostic message purposes only)
# outputs:
#   returns names with unique suffixes added; diagnostic warning
#      message is printed to inform the user of the new signal name
#
#  used internally in sysgroup

  # check for duplicate names
  l = length(names);
  ii = 1;
  while(ii <= l-1)
    st1 = nth(names,ii);
    jj = ii+1;
    while ( jj <= l)
      st2 = nth(names,jj);
      if(strcmp(st1,st2))
        suffix = ["_",num2str(jj)];
        warning("sysgroup: %s name(%d) = %s name(%d) = %s", ...
	  kind,ii,kind,jj,st1);
        strval = sprintf("%s%s",st2,suffix);
        names(jj) = strval;
        warning("sysgroup:     changed %s name %d to %s",kind,jj,strval);
        # restart the check (just to be sure there's no further duplications)
        ii = 0; jj = l;
      endif
      jj = jj+1;
    endwhile
    ii = ii+1;
  endwhile
endfunction
