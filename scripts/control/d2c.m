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
 
function csys = d2c(sys,opt)
# csys = d2c(sys[,tol])
# csys = d2c(sys,opt)
#
# inputs: 
#   sys: system data structure with discrete components
#   tol: tolerance for convergence of default "log" option (see below)
#
#   opt: conversion option.  Choose from:
#        "log":  (default) Conversion is performed via a matrix logarithm.
#                Due to some problems with this computation, it is
#                followed by a steepest descent to identify continuous time 
#                A, B, to get a better fit to the original data.  
#
#                If called as d2c(sys,tol), tol=positive scalar, the log
#                option is used.  The default value for tol is 1e-8.
#        "bi": Conversion is performed via bilinear transform
#                  1 + s T/2
#              z = ---------
#                  1 - s T/2
#              where T is the system sampling time (see syschtsam).
#
#              FIXME: exits with an error if sys is not purely discrete
#
# D2C converts the real-coefficient discrete time state space system
#
#        x(k+1) = A x(k) + B u(k)
#
# to a continuous time state space system
#        .
#        x = A1 x + B1 u
#
# The sample time used is that of the system. (see syschtsam).
  
# Written by R. Bruce Tenison August 23, 1994
# Updated by John Ingram for system data structure  August 1996
# SYS_INTERNAL accesses members of system data structure
# $Revision: 2.0.0.0 $ 

  save_val = implicit_str_to_num_ok;	# save for later
  implicit_str_to_num_ok = 1;

  if( (nargin != 1) & (nargin != 2) )
    usage("csys = d2c(sys[,tol]), csys = d2c(sys,opt)");
  elseif (!is_struct(sys))
    error("sys must be in system data structure");
  elseif(nargin == 1)
    opt = "log";
    tol = 1e-12;
  elseif(isstr(opt))   # all remaining cases are for nargin == 2
    tol = 1e-12;
    if( !(strcmp(opt,"log") | strcmp(opt,"bi") ) )
      error(["d2c: illegal opt passed=",opt]);
    endif
  elseif(!is_sample(opt))
    error("tol must be a postive scalar")
  elseif(opt > 1e-2)
    warning(["d2c: ridiculous error tolerance passed=",num2str(opt); ...
	", intended c2d call?"])
  else
    tol = opt;
    opt = "log";
  endif
  T = sysgettsam(sys);

  if(strcmp(opt,"bi"))
    # bilinear transform
    # convert with bilinear transform
    if (! is_digital(sys) )
       error("d2c requires a discrete time system for input")
    endif
    [a,b,c,d,tsam,n,nz,stname,inname,outname,yd] = sys2ss(sys);

    poles = eig(a);
    if( find(abs(poles-1) < 200*(n+nz)*eps) )
      warning("d2c: some poles very close to one.  May get bad results.");
    endif

    I = eye(size(a));
    tk = 2/sqrt(T);
    A = (2/T)*(a-I)/(a+I);
    iab = (I+a)\b;
    B = tk*iab;
    C = tk*(c/(I+a));
    D = d- (c*iab);
    stnamec = strappend(stname,"_c");
    csys = ss2sys(A,B,C,D,0,rows(A),0,stnamec,inname,outname);
  elseif(strcmp(opt,"log"))
    sys = sysupdate(sys,"ss");
    [n,nz,m,p] = sysdimensions(sys);
  
    if(nz == 0)
      warning("d2c: all states continuous; setting outputs to agree");
      csys = syssetsignals(sys,"yd",zeros(1,1:p));
      return;
    elseif(n != 0)
      warning(["d2c: n=",num2str(n),">0; performing c2d first"]);
      sys = c2d(sys,T);
    endif
    [a,b] = sys2ss(sys);
  
    [ma,na] = size(a);
    [mb,nb] = size(b);
  
    if(isempty(b) )
      warning("d2c: empty b matrix");
      Amat = a;
    else
      Amat = [a, b; zeros(nb, na) eye(nb)];
    endif
  
    poles = eig(a);
    if( find(abs(poles) < 200*(n+nz)*eps) )
      warning("d2c: some poles very close to zero.  logm not performed");
      Mtop = zeros(ma, na+nb);
    elseif( find(abs(poles-1) < 200*(n+nz)*eps) )
      warning("d2c: some poles very close to one.  May get bad results.");
      logmat = real(logm(Amat)/T);
      Mtop = logmat(1:na,:);
    else
      logmat = real(logm(Amat)/T);
      Mtop = logmat(1:na,:);
    endif
  
    # perform simplistic, stupid optimization approach.
    # should re-write with a Davidson-Fletcher CG approach
    mxthresh = norm(Mtop);
    if(mxthresh == 0)
      mxthresh = 1;
    endif
    eps1 = mxthresh;	#gradient descent step size
    cnt = max(20,(n*nz)*4);	#max number of iterations
    newgrad=1;	#signal for new gradient
    while( (eps1/mxthresh > tol) & cnt)
      cnt = cnt-1;
      # calculate the gradient of error with respect to Amat...
      geps = norm(Mtop)*1e-8;
      if(geps == 0)
        geps = 1e-8;
      endif
      DMtop = Mtop;
      if(isempty(b))
        Mall = Mtop;
        DMall = DMtop;
      else
        Mall = [Mtop; zeros(nb, na+nb)];
        DMall = [DMtop; zeros(nb, na+nb) ];
      endif
  
      if(newgrad)
        GrMall = zeros(size(Mall));
        for ii=1:rows(Mtop)
          for jj=1:columns(Mtop)
  	  DMall(ii,jj) = Mall(ii,jj) + geps;
            GrMall(ii,jj) = norm(Amat - expm(DMall*T),'fro') ...
  	    - norm(Amat-expm(Mall*T),'fro');
      	  DMall(ii,jj) = Mall(ii,jj);
          endfor
        endfor
        GrMall = GrMall/norm(GrMall,1);
        newgrad = 0;
      endif
  
      #got a gradient, now try to use it
      DMall = Mall-eps1*GrMall;
  
      FMall = expm(Mall*T);
      FDMall = expm(DMall*T);
      FmallErr = norm(Amat - FMall);
      FdmallErr = norm(Amat - FDMall);
      if( FdmallErr < FmallErr)
        Mtop = DMall(1:na,:);
        eps1 = min(eps1*2,1e12);
        newgrad = 1;
      else
        eps1 = eps1/2;
      endif
  
      if(FmallErr == 0)
        eps1 = 0;
      endif
      
    endwhile
  
    [aa,bb,cc,dd,tsam,nn,nz,stnam,innam,outnam,yd] = sys2ss(sys);
    aa = Mall(1:na,1:na);
    if(!isempty(b))
      bb = Mall(1:na,(na+1):(na+nb));
    endif
    csys = ss2sys(aa,bb,cc,dd,0,na,0,stnam,innam,outnam);
    
    # update names
    nn = sysdimensions(sys);
    for ii = (nn+1):na
      strval = sprintf("%s_c",sysgetsignals(csys,"st",ii,1));
      csys = syssetsignals(csys,"st",strval,ii);
    endfor
  endif

  implicit_str_to_num_ok = save_val;	# restore value
endfunction
