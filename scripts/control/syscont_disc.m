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
 
function [n_tot,st_c,st_d,y_c,y_d] = syscont_disc(sys)
# function [n_tot,st_c,st_d,y_c,y_d] = syscont_disc(sys)
# Used internally in syscont and sysdisc.
#
# inputs: sys is a system data structure
# outputs: n_tot: total number of states
#	   st_c: vector of continuous state indices (empty if none)
#	   st_d: vector of discrete state indices (empty if none)
#	   y_c: vector of continuous output indices
#	   y_d: vector of discrete output indices

# Written by A. S. Hodel (a.s.hodel@eng.auburn.edu) Feb 1997
# $Log: syscont_disc.m,v $
# Revision 1.2  1998/07/15 12:29:13  hodelas
# Updated to use sysdimensions.  Removed extraneous if commands (find now
# returns empty matrix if none found)
#

  # get ranges for discrete/continuous states and outputs
  [nn,nz,mm,pp,yd] = sysdimensions(sys);
  n_tot = nn + nz;
  st_c = 1:(nn);
  st_d = nn + (1:nz);
  y_c = find(yd == 0);		# y_c, y_d will be empty if there are none.
  y_d = find(yd == 1);

endfunction
