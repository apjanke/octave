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
 
function sys = sysadd(Gsys,Hsys)
# 
# [sys] = sysadd(Gsys,Hsys)
#
#
# returns transfer function sys = Gsys + Hsys
#
# Method: Gsys and Hsys are connected in parallel
# The vector are connected to both systems; the outputs will be 
# added.  The names given to the system will be the G systems names.
#
#                  ________
#             ----|  Gsys  |---
#        u   |    ----------  +|         
#        -----                (_)----> y
#            |     ________   +|
#             ----|  Hsys  |---
#                  --------

# Written by John Ingram July 1996

  save_val = implicit_str_to_num_ok;	# save for later
  implicit_str_to_num_ok = 1;

  if(nargin != 2)
    usage("sysadd:  [sys] = sysysadd(Gsys,Hsys)");
  endif

  # check inputs
  if(!is_struct(Gsys) | !is_struct(Hsys))
    error("Both Gsys and Hsys must be in system data structure form");
  endif

  # check for compatibility
  [n,nz,mg,pg] = sysdimensions(Gsys);
  [n,nz,mh,ph] = sysdimensions(Hsys);
  if(mg != mh)
    error(sprintf("Gsys inputs(%d) != Hsys inputs (%d)",mg,mh));
  elseif(pg != ph)
    error(sprintf("Gsys outputs(%d) != Hsys outputs (%d)",pg,ph));
  endif

  [Gst, Gin, Gout, Gyd] = sysgetsignals(Gsys);
  [Hst, Hin, Hout, Hyd] = sysgetsignals(Hsys);

  # check for digital to continuous addition
  if (Gyd != Hyd)
    error("can not add a discrete output to a continuous output");
  endif

  if( strcmp(sysgettype(Gsys),"tf") | strcmp(sysgettype(Hsys),"tf") )
    # see if adding  transfer functions with identical denominators
    [Gnum,Gden,GT,Gin,Gout] = sys2tf(Gsys);
    [Hnum,Hden,HT,Hin,Hout] = sys2tf(Hsys);
    if( (Hden == Gden) & (HT == GT) )
      sys = tf2sys(Gnum+Hnum,Gden,GT,Gin,Gout);
      return
    endif
    # if not, we go on and do the usual thing...
  endif

  # make sure in ss form
  Gsys = sysupdate(Gsys,"ss");
  Hsys = sysupdate(Hsys,"ss");

  # change signal names to avoid warning messages from sysgroup
  Gsys = syssetsignals(Gsys,"in",sysdefioname(length(Gin),"Gin_u"));
  Gsys = syssetsignals(Gsys,"out",sysdefioname(length(Gout),"Gout_u"));
  Hsys = syssetsignals(Hsys,"in",sysdefioname(length(Hin),"Hin_u"));
  Hsys = syssetsignals(Hsys,"out",sysdefioname(length(Hout),"Hout_u"));
  
  sys = sysgroup(Gsys,Hsys);

  eyin = eye(mg);
  eyout = eye(pg);

  sys = sysscale(sys,[eyout, eyout],[eyin;eyin],Gout,Gin);

endfunction
