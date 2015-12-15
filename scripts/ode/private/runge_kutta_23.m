## Copyright (C) 2013-2015, Jacopo Corno <jacopo.corno@gmail.com>
## Copyright (C) 2013-2015, Roberto Porcu' <roberto.porcu@polimi.it>
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or (at
## your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn  {Function File} {[@var{t_next}, @var{x_next}] =} runge_kutta_23 (@var{@fun}, @var{t}, @var{x}, @var{dt})
## @deftypefnx {Function File} {[@var{t_next}, @var{x_next}] =} runge_kutta_23 (@var{@fun}, @var{t}, @var{x}, @var{dt}, @var{options})
## @deftypefnx {Function File} {[@var{t_next}, @var{x_next}] =} runge_kutta_23 (@var{@fun}, @var{t}, @var{x}, @var{dt}, @var{options}, @var{k_vals_in})
## @deftypefnx {Function File} {[@var{t_next}, @var{x_next}] =} runge_kutta_23 (@var{@fun}, @var{t}, @var{x}, @var{dt}, @var{options}, @var{k_vals_in}, @var{t_next})
## @deftypefnx {Function File} {[@var{t_next}, @var{x_next}, @var{x_est}] =} runge_kutta_23 (@dots{})
## @deftypefnx {Function File} {[@var{t_next}, @var{x_next}, @var{x_est}, @var{k_vals_out}] =} runge_kutta_23 (@dots{})
##
## This function can be used to integrate a system of ODEs with a given initial
## condition @var{x} from @var{t} to @var{t+dt}, with the Bogacki-Shampine
## method of third order. For the definition of this method see
## @url{http://en.wikipedia.org/wiki/List_of_Runge%E2%80%93Kutta_methods}.
##
## @var{@fun} is a function handle that defines the ODE: @code{y' = f(tau,y)}.
## The function must accept two inputs where the first is time @var{tau} and the
## second is a column vector of unknowns @var{y}.
##
## @var{t} is the first extreme of integration interval.
##
## @var{x} is the initial condition of the system..
##
## @var{dt} is the timestep, that is the length of the integration interval.
##
## The optional fourth argument @var{options} specifies options for the ODE
## solver. It is a structure generated by @code{odeset}. In particular it
## contains the field @var{funarguments} with the optional arguments to be used
## in the evaluation of @var{fun}.
##
## The optional fifth argument @var{k_vals_in} contains the Runge-Kutta
## evaluations of the previous step to use in a FSAL scheme.
##
## The optional sixth argument @var{t_next} (@code{t_next = t + dt}) specifies
## the end of the integration interval. The output @var{x_next} s the higher
## order computed solution at time @var{t_next} (local extrapolation is
## performed).
##
## Optionally the functions can also return @var{x_est}, a lower order solution
## for the estimation of the error, and @var{k_vals_out}, a matrix containing
## the Runge-Kutta evaluations to use in a FSAL scheme or for dense output.
##
## @seealso{runge_kutta_45_dorpri}
## @end deftypefn

function [t_next, x_next, x_est, k] = runge_kutta_23 (f, t, x, dt, options = [],
                                                      k_vals = [],
                                                      t_next = t + dt)

  if (! isempty (options))  # extra arguments for function evaluator
    args = options.funarguments;
  else
    args = {};
  endif
  
  if (! isempty (k_vals))    # k values from previous step are passed
    k(:,1) = k_vals(:,end);  # FSAL property
  else
    k(:,1) = feval (f, t, x, args{:});
  endif

  k(:,2) = feval (f, t + (1/2)*dt, x + dt*(1/2)*k(:,1), args{:});
  k(:,3) = feval (f, t + (3/4)*dt, x + dt*(3/4)*k(:,2), args{:});

  ## compute new time and new values for the unkwnowns
  ## t_next = t + dt;
  x_next = x + dt.*((2/9)*k(:,1) + (1/3)*k(:,2) + (4/9)*k(:,3)); # 3rd order approximation

  ## if the estimation of the error is required
  if (nargout >= 3)
    ## new solution to be compared with the previous one
    k(:,4) = feval (f, t_next, x_next, args{:});
    x_est = x + dt.*((7/24)*k(:,1) + (1/4)*k(:,2) ...
                     + (1/3)*k(:,3) + (1/8)*k(:,4));
  endif

endfunction


%! # We are using the "Van der Pol" implementation.
%!function [ydot] = fpol (t, y) ## The Van der Pol
%!  ydot = [y(2); (1 - y(1)^2) * y(2) - y(1)];
%!endfunction
%!
%!shared opts
%!  opts = odeset;
%!  opts.funarguments = {};
%!
%!test
%!  [t, y] = runge_kutta_23 (@fpol, 0, [2;0], 0.05, opts);
%!test
%!  [t, y, x] = runge_kutta_23 (@fpol, 0, [2;0], 0.1, opts);
