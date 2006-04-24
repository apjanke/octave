## Copyright (C) 1993, 1998, 1999 Auburn University.  All rights reserved.
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by the
## Free Software Foundation; either version 2, or (at your option) any
## later version.
##
## Octave is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, write to the Free
## Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
## 02110-1301 USA.

## -*- texinfo -*-
## @deftypefn {Function File} {[@var{u}, @var{h}, @var{nu}] =} krylov (@var{a}, @var{v}, @var{k}, @var{eps1}, @var{pflg})
## Construct an orthogonal basis @var{u} of block Krylov subspace
##
## @example
## [v a*v a^2*v ... a^(k+1)*v]
## @end example
##
## @noindent
## Using Householder reflections to guard against loss of orthogonality.
##
## If @var{v} is a vector, then @var{h} contains the Hessenberg matrix
## such that @code{a*u == u*h}.  Otherwise, @var{h} is meaningless.
##
## The value of @var{nu} is the dimension of the span of the krylov
## subspace (based on @var{eps1}).
##
## If @var{b} is a vector and @var{k} is greater than @var{m-1}, then
## @var{h} contains the Hessenberg decompostion of @var{a}.
##
## The optional parameter @var{eps1} is the threshold for zero.  The
## default value is 1e-12.
##
## If the optional parameter @var{pflg} is nonzero, row pivoting is used
## to improve numerical behavior.  The default value is 0.
##
## Reference: Hodel and Misra, "Partial Pivoting in the Computation of
## Krylov Subspaces", to be submitted to Linear Algebra and its
## Applications
## @end deftypefn

## Author: A. Scottedward Hodel <a.s.hodel@eng.auburn.edu>

function [Uret, H, nu] = krylov (A, V, k, eps1, pflg);

  defeps = 1e-12;

  if (nargin < 3 || nargin > 5)
    usage ("[U, nu] = krylov (A, V, k, eps1, pflg)")
  elseif (nargin < 5)
    pflg = 0;        # default permutation flag
  endif

  if(nargin < 4)
    eps1 = defeps;    # default tolerance parameter
  endif

  if (isempty (eps1))
    eps1 = defeps;
  endif

  na = issquare (A);
  if (! na)
    error ("A(%d x %d) must be square", rows (A), columns (A));
  endif

  [m, kb] = size(V);
  if (m != na)
    error("A(%d x %d), V(%d x %d): argument dimensions do not match",
	  na, na, m, kb)
  endif

  if (! isscalar (k))
    error ("krylov: third argument must be a scalar integer");
  endif

  Vnrm = norm (V, Inf);

  ## check for trivial solution
  if (Vnrm == 0)
    Uret = [];
    H = [];
    nu = 0;
    return;
  endif

  # identify trivial null space
  abm = max (abs ([A, V]'));
  nzidx = find (abm != 0);
  zidx = find (abm == 0);

  # set up vector of pivot points
  pivot_vec = 1:na;

  iter = 0;
  alpha = [];
  nh = 0;
  while (length(alpha) < na) && (columns(V) > 0) && (iter < k)
    iter++;

    ## get orthogonal basis of V
    jj = 1;
    while (jj <= columns (V) && length (alpha) < na)
      ## index of next Householder reflection
      nu = length(alpha)+1;

      short_pv = pivot_vec(nu:na);
      q = V(:,jj);
      short_q = q(short_pv);

      if (norm (short_q) < eps1)
	## insignificant column; delete
        nv = columns (V);
        if (jj != nv)
          [V(:,jj), V(:,nv)] = swap (V(:,jj), V(:,nv));
	  ## FIXME -- H columns should be swapped too.  Not done
	  ## since Block Hessenberg structure is lost anyway.
        endif
        V = V(:,1:(nv-1));
	## one less reflection
        nu--;
      else
	## new householder reflection
        if (pflg)
          ## locate max magnitude element in short_q
          asq = abs (short_q);
          maxv = max (asq);
          maxidx = find (asq == maxv);
          pivot_idx = short_pv(maxidx(1));

	  ## see if need to change the pivot list
          if (pivot_idx != pivot_vec(nu))
            swapidx = maxidx(1) + (nu-1);
            [pivot_vec(nu), pivot_vec(swapidx)] = ...
		swap (pivot_vec(nu), pivot_vec(swapidx));
          endif
        endif

	## isolate portion of vector for reflection
        idx = pivot_vec(nu:na);
        jdx = pivot_vec(1:nu);

        [hv, av, z] = housh (q(idx), 1, 0);
        alpha(nu) = av;
        U(idx,nu) = hv;

        # reduce V per the reflection
        V(idx,:) = V(idx,:) - av*hv*(hv' * V(idx,:));
        if(iter > 1)
	  ## FIXME -- not done correctly for block case
          H(nu,nu-1) = V(pivot_vec(nu),jj);
        endif

        ## advance to next column of V
        jj++;
      endif
    endwhile

    ## check for oversize V (due to full rank)
    if ((columns (V) > na) && (length (alpha) == na))
      ## trim to size
      V = V(:,1:na);
    elseif (columns(V) > na)
      krylov_V = V
      krylov_na = na
      krylov_length_alpha = length (alpha)
      error ("This case should never happen; submit a bug report");
    endif

    if (columns (V) > 0)
      ## construct next Q and multiply
      Q = zeros (size (V));
      for kk = 1:columns (Q)
        Q(pivot_vec(nu-columns(Q)+kk),kk) = 1;
      endfor

      ## apply Householder reflections
      for ii = nu:-1:1
        idx = pivot_vec(ii:na);
        hv = U(idx,ii);
        av = alpha(ii);
        Q(idx,:) = Q(idx,:) - av*hv*(hv'*Q(idx,:));
      endfor
    endif

    ## multiply to get new vector;
    V = A*Q;
    ## project off of previous vectors
    nu = length (alpha);
    for i = 1:nu
      hv = U(:,i);
      av = alpha(i);
      V = V - av*hv*(hv'*V);
      H(i,nu-columns(V)+(1:columns(V))) = V(pivot_vec(i),:);
    end

  endwhile

  ## Back out complete U matrix
  ## back out U matrix ;
  j1 = columns (U);
  for i = j1:-1:1;
    idx = pivot_vec(i:na);
    hv = U(idx,i);
    av = alpha(i);
    U(:,i) = zeros (na, 1);
    U(idx(1),i) = 1;
    U(idx,i:j1) = U(idx,i:j1)-av*hv*(hv'*U(idx,i:j1));
  endfor

  nu = length (alpha);
  Uret = U;
  if (max (max (abs (Uret(zidx,:)))) > 0)
    warning ("krylov: trivial null space corrupted; set pflg = 1 or eps1 > %e",
	     eps1);
  endif

endfunction
