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
 
function retsys = syssetsignals(sys,opt,names,sig_idx)
# retsys = syssetsignals(sys,opt,names{,sig_idx})
# change the names of selected inputs, outputs and states.
# inputs:
# 	sys: system data structure
#	opt: []: change default name (output)
#	     "out": change selected output names
#	     "in": change selected input names
#	     "st": change selected state names	 
#	     "yd": change selected outputs from discrete to continuous or 
#		   from continuous to discrete.
#    	names: opt = "out", "in", or "st": string or or list of strings
#              opt = "yd": desired output continuous/discrete flag.
#                          names(ii) = 0: output ii is continuous
#                          names(ii) = 1: output ii is discrete
#     	list: vector of indices of outputs, yd, inputs, or
#             states whose respective names should be changed.
#             Default: replace entire signal list/vector value
# outputs:
#    retsys=sys with appropriate signal names changed 
#            (or yd values, where appropriate)

# Written by John Ingram August 1996

  save_val = implicit_str_to_num_ok;	# save for later
  implicit_str_to_num_ok = 1;

  if (nargin < 3 | nargin > 4)
    usage("retsys=syssetsignals(sys,opt,names{,sig_idx})");
  elseif (!is_struct(sys))
    error("sys must be a system data structure");
  elseif (isempty(opt))
    opt = "out";
  elseif( ! isstr(opt)  )
    error("opt must be a string");
  elseif( ! (strcmp(opt,"out") + strcmp(opt,"yd") + ...
    strcmp(opt,"in") + strcmp(opt,"st") ) )
    error("opt must be one of [], ""out"", ""yd"", ""in"", or ""st""");
  elseif(nargin == 4)
    if(min(size(sig_idx)) > 1)
      disp("syssetsignals: sig_idx=")
      disp(sig_idx);
      error("sig_idx must be a vector")
    endif
  endif

  sig_vals = sysgetsignals(sys,opt);

  # make sure it's in state space form if state names are given
  if(strcmp(opt,"st"))    sys = sysupdate(sys,"ss");    endif

  if(strcmp(opt,"yd") == 0)
    # it's a signal name list we're changing
    if(!is_list(names))
      names = list(names);
    endif
    if(!is_signal_list(names))
      if(isstr(nth(names,1)))
        warning("syssetsignals(opt=%s): converting string matrix \"names\" to a list of strings",opt);
        tmpstr = nth(names,1);
        for ii=1:rows(tmpstr)
          names(ii) = deblank(tmpstr(ii,:));
        endfor
      else
        names
        error("parameter \"names\" must be a list of strings");
      endif
    endif
    nsigs = length(sig_vals);

    if(nargin == 3)
      # replace all signal names
      if(length(names) != nsigs)
        error("opt=%s, sig_idx omitted: names(len=%d) should have %d entries ", ...
          opt,length(names),nsigs);
      endif
      sig_idx = 1:nsigs;
    elseif(length(names) != length(sig_idx))
      # replace specified signal names
      error("opt=%s, sig_idx(len=%d), names(len=%d) mismatch",opt, ...
        length(sig_idx), length(names));
    endif

    for ii=1:length(sig_idx)
      jj = sig_idx(ii);
      if(jj < 1 | jj > nsigs | jj != floor(jj+0.5))
        error("opt=%s, sig_idx(%d)=%d, %e: must be an integer between 1 and %d", ...
          opt, ii, jj, jj, nsigs);
      endif
      sig_vals(jj) = nth(names,ii);
    endfor

  else
    nsigs = length(sig_vals);
    if(!is_vector(names))
      error("syssetsignals: opt=yd, names(%dx%d) must be a vector", ...
        rows(names), columns(names));
    endif
    if(nargin == 3)
      if(length(names) != nsigs)
        error("opt=yd, sig_idx omitted: names(%d) should be length(%d)", ...
          length(names), nsigs);
      endif
      sig_idx = 1:nsigs;
    elseif(length(names) != length(sig_idx))
      error("opt=yd: length(names)=%d, length(sig_idx)=%d",length(names), ...
        length(sig_idx) );
    endif

    badidx = find(names != 0 & names != 1);
    if(! isempty(badidx) )
      for ii=1:length(badidx)
        warning("syssetsignals: opt=yd: names(%d)=%e, must be 0 or 1", ...
          badidx(ii), names(badidx(ii)) );
      endfor
      error("opt=yd: illegal values in names");
    endif

    for ii=1:length(sig_idx)
      jj = sig_idx(ii);
      if(jj < 1 | jj > nsigs | jj != floor(jj))
        error("sig_idx(%d)=%d, %e: must be an integer between 1 and %d", ...
          ii,jj, jj, nsigs);
      endif
      sig_vals(jj) = names(ii);
    endfor
    if(any(sig_vals == 1) & sysgettsam(sys) == 0)
      warning("Setting system sampling time to 1");
      printf("syssetsignals: original system sampling time=0 but output(s)\n");
      disp(find(sig_vals==1))
      printf("are digital\n");
      sys = syschtsam(sys,1);
    endif
    
  endif

  if(strcmp(opt,"st"))
    sys.stname = sig_vals;
  elseif(strcmp(opt,"in"))
    sys.inname = sig_vals;
  elseif(strcmp(opt,"out"))
    sys.outname = sig_vals;
  elseif(strcmp(opt,"yd"))
    sys.yd = sig_vals;
  endif

  retsys = sys;
  implicit_str_to_num_ok = save_val;	# restore value

endfunction
