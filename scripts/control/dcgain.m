# Copyright (C) 1993, 1994, 1995 John W. Eaton
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

function gm = dcgain(sys, tol)
# Usage: gm = dcgain(sys[, tol])
#      Returns dc-gain matrix. If dc-gain is infinity
#      an empty matrix is returned.
#      The argument tol is an optional tolerance for the condition
#      number of A-Matrix in sys (default tol = 1.0e-10)
#      Prints a warning message of the system is unstable.
#

# Written by Kai P Mueller (mueller@ifr.ing.tu-bs.de) October 1, 1997
# $Revision: 2.0.0.0 $

  if((nargin < 1) || (nargin > 2) || (nargout > 1))
    usage("[gm, ok] = dcgain(sys[, tol])");
  endif
  if(!is_struct(sys))
    error("dcgain: first argument is not a system data structure.")
  endif
  sys = sysupdate(sys, "ss");
  [aa,bb,cc,dd] = sys2ss(sys);
  if (is_digital(sys))  aa = aa - eye(size(aa));  endif
  if (nargin == 1)  tol = 1.0e-10;  endif
  r = rank(aa, tol);
  if (r < rows(aa))
    gm = [];
  else
    gm = -cc / aa * bb + dd;
  endif
  if(!is_stable(sys))
    [nn,nz,mm,pp] = sysdimensions(sys);
    warning("dcgain: unstable system; dimensions [nc=%d,nz=%d,mm=%d,pp=%d]", ...
      nn,nz,mm,pp);
  endif
endfunction
