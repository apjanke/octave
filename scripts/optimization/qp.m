## Copyright (C) 2000, 2001, 2004, 2005 Gabriele Pannocchia.
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, write to the Free
## Software Foundation, 59 Temple Place - Suite 330, Boston, MA
## 02111-1307, USA.

## -*- texinfo -*-
## @deftypefn {Function File} {[@var{x}, @var{obj}, @var{info}, @var{lambda}] =} qp (@var{x0}, @var{H}, @var{q}, @var{A}, @var{b}, @var{lb}, @var{ub}, @var{A_lb}, @var{Ain}, @var{A_ub})
## Solve the quadratic program
## @ifinfo
##
## @example
##      min 0.5 x'*H*x + x'*q
##       x
## @end example
##
## @end ifinfo
## @iftex
## @tex
## @end tex
## @end iftex
## subject to
## @ifinfo
##
## @example
##      A*x = b
##      lb <= x <= ub
##      A_lb <= Ain*x <= A_ub
## @end example
## @end ifinfo
## @iftex
## @tex
## @end tex
## @end iftex
##
## @noindent
## using a null-space active-set method.
##
## Any bound (@var{A}, @var{b}, @var{lb}, @var{ub}, @var{A_lb},
## @var{A_ub}) may be set to the empty matrix (@code{[]}) if not
## present.  If the initial guess is feasible the algorithm is faster.
##
## The value @var{info} is a structure with the following fields:
## @table @code
## @item solveiter
## The number of iterations required to find the solution.
## @item info
## An integer indicating the status of the solution, as follows:
## @table @asis
## @item 0
## The problem is feasible and convex.  Global solution found.
## @item 1
## The problem is not convex.  Local solution found.
## @item 2
## The problem is not convex and unbounded.
## @item 3
## Maximum number of iterations reached.
## @item 6
## The problem is infeasible.
## @end table
## @end table
## @end deftypefn

function [x, obj, INFO, lambda] = qp (x0, H, q, A, b, lb, ub, A_lb, A_in, A_ub)

  if (nargin == 5 || nargin == 7 || nargin == 10)

    ## Checking the quadratic penalty
    n = issquare (H);
    if (n == 0)
      error ("qp: quadratic penalty matrix not square");
    endif

    n1 = issymmetric (H);
    if (n1 == 0)
      ## warning ("qp: quadratic penalty matrix not symmetric");
      H = (H + H')/2;
    endif

    ## Checking the initial guess (if empty it is resized to the
    ## right dimension and filled with 0)
    if (isempty (x0))
      x0 = zeros (n, 1);
    elseif (length (x0) != n)
      error ("qp: the initial guess has incorrect length");
    endif

    ## Linear penalty.
    if (length (q) != n)
      error ("qp: the linear term has incorrect length");
    endif

    ## Equality constraint matrices
    if (isempty (A) || isempty(b))
      n_eq = 0;
      A = zeros (n_eq, n);
      b = zeros (n_eq, 1);
    else
      [n_eq, n1] = size (A);
      if (n1 != n)
	error ("qp: equality constraint matrix has incorrect column dimension");
      endif
      if (length (b) != n_eq)
	error ("qp: equality constraint matrix and vector have inconsistent dimension");
      endif
    endif

    ## Bound constraints
    Ain = zeros (0, n);
    bin = zeros (0, 1);
    n_in = 0;
    if (nargin > 5)
      if (! isempty (lb))
	if (length(lb) != n)
	  error ("qp: lower bound has incorrect length");
	else
	  Ain = [Ain; eye(n)];
	  bin = [bin; lb];
	endif
      endif

      if (! isempty (ub))
	if (length (ub) != n)
	  error ("qp: upper bound has incorrect length");
	else
	  Ain = [Ain; -eye(n)];
	  bin = [bin; -ub];
	endif
      endif
    endif

    ## Inequality constraints
    if (nargin > 7)
      [dimA_in, n1] = size (A_in);
      if (n1 != n)
	error ("qp: inequality constraint matrix has incorrect column dimension");
      else
	if (! isempty (A_lb))
	  if (length (A_lb) != dimA_in)
	    error ("qp: inequality constraint matrix and lower bound vector inconsistent");
	  else
	    Ain = [Ain; A_in];
	    bin = [bin; A_lb];
	  endif
	endif
	if (! isempty (A_ub))
	  if (length (A_ub) != dimA_in)
	    error ("qp: inequality constraint matrix and upper bound vector inconsistent");
	  else
	    Ain = [Ain; -A_in];
	    bin = [bin; -A_ub];
	  endif
	endif
      endif
    endif
    n_in = length (bin);

    ## Now we should have the following QP:
    ##
    ##   min_x  0.5*x'*H*x + x'*q
    ##   s.t.   A*x = b
    ##          Ain*x >= bin

    ## Discard inequality constraints that have -Inf bounds since those
    ## will never be active.
    idx = isinf (bin) & bin < 0;
    bin(idx) = [];
    Ain(idx,:) = [];

    ## Check if the initial guess is feasible.
    rtol = sqrt (eps);

    eq_infeasible = (n_eq > 0 && norm (A*x0-b) > rtol*(1+norm (b)));
    in_infeasible = (n_in > 0 && any (Ain*x0-bin < -rtol*(1+norm (bin))));

    info = 0;
    if (eq_infeasible || in_infeasible)
      ## The initial guess is not feasible.
      ## First define xbar that is feasible with respect to the equality
      ## constraints.
      if (eq_infeasible)
	if (rank (A) < n_eq)
	  error ("qp: equality constraint matrix must be full row rank")
	endif
	xbar = pinv (A) * b;
      else
	xbar = x0;
      endif

      ## Check if xbar is feasible with respect to the inequality
      ## constraints also.
      if (n_in > 0)
	res = Ain * xbar - bin;
	if (any (res < -rtol * (1 + norm (bin))))
	  ## xbar is not feasible with respect to the inequality
	  ## constraints.  Compute a step in the null space of the
	  ## equality constraints, by solving a QP.  If the slack is
	  ## small, we have a feasible initial guess.  Otherwise, the
	  ## problem is infeasible.
	  if (n_eq > 0)
	    Z = null (A);
	    if (isempty (Z))
	      ## The problem is infeasible because A is square and full
	      ## rank, but xbar is not feasible.
	      info = 6;
	    endif
	  endif

	  if (info != 6)
            ## Solve an LP with additional slack variables to find
	    ## a feasible starting point.
	    gamma = eye (n_in);
	    if (n_eq > 0)
	      Atmp = [Ain*Z, gamma];
	      btmp = -res;
	    else
	      Atmp = [Ain, gamma];
	      btmp = bin;
	    endif
	    ctmp = [zeros(n-n_eq, 1); ones(n_in, 1)];
	    lb = [-Inf*ones(n-n_eq,1); zeros(n_in,1)];
	    ub = [];
	    ctype = repmat ("L", n_in, 1);
	    [P, dummy, status] = glpk (ctmp, Atmp, btmp, lb, ub, ctype);

	    if ((status == 180 || status == 181 || status == 151)
		&& all (abs (P(n-n_eq+1:end)) < rtol * (1 + norm (btmp))))
	      ## We found a feasible starting point
	      if (n_eq > 0)
		x0 = xbar + Z*P(1:n-n_eq);
	      else
		x0 = P(1:n);
              endif
	    else
	      ## The problem is infeasible
	      info = 6;
	    endif
	  endif
	else
	  ## xbar is feasible.  We use it a starting point.
	  x0 = xbar;
	endif
      else
	## xbar is feasible.  We use it a starting point.
	x0 = xbar;
      endif
    endif

    if (info == 0)
      ## The initial (or computed) guess is feasible.
      ## We call the solver.
      maxit = 100;
      [x, lambda, info, iter] = __qp__ (x0, H, q, A, b, Ain, bin, maxit);
    else
      iter = 0;
      x = x0;
      lambda = [];
    endif
    obj = 0.5 * x' * H * x + q' * x;
    INFO.solveiter = iter;
    INFO.info = info;

  else
    usage ("[x, obj, info, lambda] = qp (x0, H, q, A, b, lb, ub, A_lb, Ain, A_ub)");
  endif

endfunction
