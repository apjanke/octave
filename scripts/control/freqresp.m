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
 
function [ff,w] = freqresp(sys,USEW,w);
  # function [ff,w] = freqresp(sys,USEW{,w});
  # Frequency response function - used internally by bode, nyquist.
  # minimal argument checking; "do not attempt to do this at home"
  # USEW returned by freqchkw 
  # w: optional, must be present if USEW is given
  #
  # returns: ff = vector of finite G(j*w) entries (or || G(j*w) || for MIMO)
  #          w = vector of frequencies used
  #      ff and w are both returned as row vectors

  #  Written by: R. Bruce Tenison July 11, 1994
  # $Revision: 1.6 $
  # SYS_INTERNAL accesses members of system data structure

  save_val = empty_list_elements_ok;
  empty_list_elements_ok = 1;

  # Check Args
  if( (nargin < 2) || (nargin > 4) )
    usage ("[ff,w] = freqresp(sys,USEW{,w})");
  elseif( USEW & (nargin < 3) )
    error("USEW=1 but w was not passed.");
  elseif( USEW & isempty(w))
    warning("USEW=1 but w is empty; setting USEW=0");
    USEW=0;
  endif

  DIGITAL = is_digital(sys);

  # compute default w if needed
  if(!USEW)
    if(is_siso(sys))
      sys = sysupdate(sys,"zp");
      [zer,pol] = sys2zp(sys);
    else
      zer = tzero(sys);
      pol = eig(sys2ss(sys));
    endif

    # get default frequency range
    [wmin,wmax] = bode_bounds(zer,pol,DIGITAL,sysgettsam(sys));
    w = logspace(wmin,wmax,50);
  else
    w = reshape(w,1,length(w)); 	# make sure it's a row vector
  endif

  # now get complex values of s or z
  if(DIGITAL)
    jw = exp(i*w*sysgettsam(sys));
  else
    jw = i*w;
  endif

  [nn,nz,mm,pp] = sysdimensions(sys);

  # now compute the frequency response - divide by zero yields a warning
  if (strcmp(sysgettype(sys),"zp"))
    # zero-pole form (preferred)
    [zer,pol,sysk] = sys2zp(sys);
    ff = ones(size(jw));
    l1 = min(length(zer)*(1-isempty(zer)),length(pol)*(1-isempty(pol)));
    for ii=1:l1
      ff = ff .* (jw - zer(ii)) ./ (jw - pol(ii));
    endfor

    # require proper  transfer function, so now just get poles.
    for ii=(l1+1):length(pol)
      ff = ff ./ (jw - pol(ii));
    endfor
    ff = ff*sysk;

  elseif (strcmp(sysgettype(sys),"tf"))
    # transfer function form 
    [num,den] = sys2tf(sys);
    ff = polyval(num,jw)./polyval(den,jw);
  elseif (mm==pp)
    # The system is square; do state-space form bode plot
    [sysa,sysb,sysc,sysd,tsam,sysn,sysnz] = sys2ss(sys);
    n = sysn + sysnz;
    for ii=1:length(jw);
      ff(ii) = det(sysc*((jw(ii).*eye(n)-sysa)\sysb)+sysd);
    endfor;
  else
    # Must be state space... bode                            
    [sysa,sysb,sysc,sysd,tsam,sysn,sysnz] = sys2ss(sys);
    n = sysn + sysnz;
    for ii=1:length(jw);
      ff(ii) = norm(sysc*((jw(ii)*eye(n)-sysa)\sysb)+sysd);
    endfor
    
  endif

  w = reshape(w,1,length(w));
  ff = reshape(ff,1,length(ff));

  # restore global variable
  empty_list_elements_ok = save_val;
endfunction

