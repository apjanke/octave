## Copyright (C) 2016 Carlo de Falco
## Copyright (C) 2016 Francesco Faccio <francesco.faccio@mail.polimi.it>
## Copyright (C) 2014-2016 Jacopo Corno <jacopo.corno@gmail.com>
## Copyright (C) 2013-2016 Roberto Porcu' <roberto.porcu@polimi.it>
## Copyright (C) 2006-2016 Thomas Treichl <treichl@users.sourceforge.net>
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
## @deftypefn  {} {[@var{t}, @var{y}] =} ode23 (@var{fun}, @var{trange}, @var{init})
## @deftypefnx {} {[@var{t}, @var{y}] =} ode23 (@var{fun}, @var{trange}, @var{init}, @var{ode_opt})
## @deftypefnx {} {[@var{t}, @var{y}, @var{te}, @var{ye}, @var{ie}] =} ode23 (@dots{})
## @deftypefnx {} {@var{solution} =} ode23 (@dots{})
##
## Solve a set of non-stiff Ordinary Differential Equations (non-stiff ODEs)
## with the well known explicit @nospell{Bogacki-Shampine} method of order 3.
## For the definition of this method see
## @url{http://en.wikipedia.org/wiki/List_of_Runge%E2%80%93Kutta_methods}.
##
## @var{fun} is a function handle, inline function, or string containing the
## name of the function that defines the ODE: @code{y' = f(t,y)}.  The function
## must accept two inputs where the first is time @var{t} and the second is a
## column vector of unknowns @var{y}.
##
## @var{trange} specifies the time interval over which the ODE will be
## evaluated.  Typically, it is a two-element vector specifying the initial and
## final times (@code{[tinit, tfinal]}).  If there are more than two elements
## then the solution will also be evaluated at these intermediate time
## instances.
##
## By default, @code{ode23} uses an adaptive timestep with the
## @code{integrate_adaptive} algorithm.  The tolerance for the timestep
## computation may be changed by using the options @qcode{"RelTol"}
## and @qcode{"AbsTol"}.
##
## @var{init} contains the initial value for the unknowns.  If it is a row
## vector then the solution @var{y} will be a matrix in which each column is
## the solution for the corresponding initial value in @var{init}.
##
## The optional fourth argument @var{ode_opt} specifies non-default options to
## the ODE solver.  It is a structure generated by @code{odeset}.
##
## The function typically returns two outputs.  Variable @var{t} is a
## column vector and contains the times where the solution was found.  The
## output @var{y} is a matrix in which each column refers to a different
## unknown of the problem and each row corresponds to a time in @var{t}.
##
## The output can also be returned as a structure @var{solution} which
## has a field @var{x} containing a row vector of times where the solution
## was evaluated and a field @var{y} containing the solution matrix such
## that each column corresponds to a time in @var{x}.
## Use @code{fieldnames (@var{solution})} to see the other fields and
## additional information returned.
##
## If using the @qcode{"Events"} option then three additional outputs may
## be returned.  @var{te} holds the time when an Event function returned a
## zero.  @var{ye} holds the value of the solution at time @var{te}.  @var{ie}
## contains an index indicating which Event function was triggered in the case
## of multiple Event functions.
##
## This function can be called with two output arguments: @var{t} and @var{y}.
## Variable @var{t} is a column vector and contains the time stamps, instead
## @var{y} is a matrix in which each column refers to a different unknown of
## the problem and the rows number is the same of @var{t} rows number so
## that each row of @var{y} contains the values of all unknowns at the time
## value contained in the corresponding row in @var{t}.
##
## Example: Solve the @nospell{Van der Pol} equation
##
## @example
## @group
## fvdp = @@(@var{t},@var{y}) [@var{y}(2); (1 - @var{y}(1)^2) * @var{y}(2) - @var{y}(1)];
## [@var{t},@var{y}] = ode23 (fvdp, [0, 20], [2, 0]);
## @end group
## @end example
## @seealso{odeset, odeget, ode45}
## @end deftypefn

function varargout = ode23 (fun, trange, init, varargin)

  if (nargin < 3)
    print_usage ();
  endif

  order  = 3;
  solver = "ode23";

  if (nargin >= 4)
    if (! isstruct (varargin{1}))
      ## varargin{1:len} are parameters for fun
      odeopts = odeset ();
      funarguments = varargin;
    elseif (numel (varargin) > 1)
      ## varargin{1} is an ODE options structure opt
      odeopts = varargin{1};
      funarguments = {varargin{2:numel (varargin)}};
    else  # if (isstruct (varargin{1}))
      odeopts = varargin{1};
      funarguments = {};
    endif
  else  # nargin == 3
    odeopts = odeset ();
    funarguments = {};
  endif

  if (! isnumeric (trange) || ! isvector (trange))
    error ("Octave:invalid-input-arg",
           "ode23: TRANGE must be a numeric vector");
  endif

  if (numel (trange) < 2)
    error ("Octave:invalid-input-arg",
           "ode23: TRANGE must contain at least 2 elements");
  elseif (trange(2) == trange(1))
    error ("Octave:invalid-input-arg",
           "ode23: invalid time span, TRANGE(1) == TRANGE(2)");
  else
    direction = sign (trange(2) - trange(1));
  endif
  trange = trange(:);

  if (! isnumeric (init) || ! isvector (init))
    error ("Octave:invalid-input-arg",
           "ode23: INIT must be a numeric vector");
  endif
  init = init(:);

  if (ischar (fun))
    try
      fun = str2func (fun);
    catch
      warning (lasterr);
    end_try_catch
  endif
  if (! isa (fun, "function_handle"))
    error ("Octave:invalid-input-arg",
           "ode23: FUN must be a valid function handle");
  endif

  ## Start preprocessing, have a look which options are set in odeopts,
  ## check if an invalid or unused option is set.
  [defaults, classes, attributes] = odedefaults (numel (init),
                                                 trange(1), trange(end));

  persistent ode23_ignore_options = ...
    {"BDF", "InitialSlope", "Jacobian", "JPattern",
     "MassSingular", "MaxOrder", "MvPattern", "Vectorized"};

  defaults   = rmfield (defaults, ode23_ignore_options);
  classes    = rmfield (classes, ode23_ignore_options);
  attributes = rmfield (attributes, ode23_ignore_options);

  odeopts = odemergeopts ("ode23", odeopts, defaults, classes, attributes);

  odeopts.funarguments = funarguments;
  odeopts.direction    = direction;

  if (! isempty (odeopts.NonNegative))
    if (isempty (odeopts.Mass))
      odeopts.havenonnegative = true;
    else
      odeopts.havenonnegative = false;
      warning ("Octave:invalid-input-arg",
               ['ode23: option "NonNegative" is ignored', ...
                " when mass matrix is set\n"]);
    endif
  else
    odeopts.havenonnegative = false;
  endif

  if (isempty (odeopts.OutputFcn) && nargout == 0)
    odeopts.OutputFcn = @odeplot;
    odeopts.haveoutputfunction = true;
  else
    odeopts.haveoutputfunction = ! isempty (odeopts.OutputFcn);
  endif

  if (isempty (odeopts.InitialStep))
    odeopts.InitialStep = odeopts.direction * ...
                          starting_stepsize (order, fun, trange(1), init,
                                             odeopts.AbsTol, odeopts.RelTol,
                                             strcmp (odeopts.NormControl, "on"),
                                             odeopts.funarguments);
  endif

  if (! isempty (odeopts.Mass) && isnumeric (odeopts.Mass))
    havemasshandle = false;
    mass = odeopts.Mass;    # constant mass
  elseif (isa (odeopts.Mass, "function_handle"))
    havemasshandle = true;  # mass defined by a function handle
  else  # no mass matrix - creating a diag-matrix of ones for mass
    havemasshandle = false; # mass = diag (ones (length (init), 1), 0);
  endif

  ## Starting the initialization of the core solver ode23

  if (havemasshandle)   # Handle only the dynamic mass matrix,
    if (! strcmp (odeopts.MStateDependence, "none"))
      ## constant mass matrices have already been handled
      mass = @(t,x) odeopts.Mass (t, x, odeopts.funarguments{:});
      fun = @(t,x) mass (t, x, odeopts.funarguments{:}) ...
                   \ fun (t, x, odeopts.funarguments{:});
    else
      mass = @(t) odeopts.Mass (t, odeopts.funarguments{:});
      fun = @(t,x) mass (t, odeopts.funarguments{:}) ...
                   \ fun (t, x, odeopts.funarguments{:});
    endif
  endif

  if (nargout == 1)
    ## Single output requires auto-selected intermediate times,
    ## which is obtained by NOT specifying specific solution times.
    trange = [trange(1); trange(end)];
    odeopts.Refine = [];  # disable Refine when single output requested
  elseif (numel (trange) > 2)
    odeopts.Refine = [];  # disable Refine when specific times requested
  endif

  solution = integrate_adaptive (@runge_kutta_23,
                                 order, fun, trange, init, odeopts);

  ## Postprocessing, do whatever when terminating integration algorithm
  if (odeopts.haveoutputfunction)  # Cleanup plotter
    feval (odeopts.OutputFcn, [], [], "done", odeopts.funarguments{:});
  endif
  if (! isempty (odeopts.Events))   # Cleanup event function handling
    ode_event_handler (odeopts.Events, solution.t(end),
                       solution.x(end,:).', "done", odeopts.funarguments{:});
  endif

  ## Print additional information if option Stats is set
  if (strcmp (odeopts.Stats, "on"))
    nsteps    = solution.cntloop;             # cntloop from 2..end
    nfailed   = solution.cntcycles - nsteps;  # cntcycl from 1..end
    nfevals   = 3 * solution.cntcycles + 1;   # number of ode evaluations
    ndecomps  = 0;  # number of LU decompositions
    npds      = 0;  # number of partial derivatives
    nlinsols  = 0;  # no. of solutions of linear systems
    ## Print cost statistics if no output argument is given
    if (nargout == 0)
      printf ("Number of successful steps: %d\n", nsteps);
      printf ("Number of failed attempts:  %d\n", nfailed);
      printf ("Number of function calls:   %d\n", nfevals);
    endif
  endif

  if (nargout == 2)
    varargout{1} = solution.t;      # Time stamps are first output argument
    varargout{2} = solution.x;      # Results are second output argument
  elseif (nargout == 1)
    varargout{1}.x = solution.t.';   # Time stamps are saved in field x (row vector)
    varargout{1}.y = solution.x.';   # Results are saved in field y (row vector)
    varargout{1}.solver = solver;   # Solver name is saved in field solver
    if (! isempty (odeopts.Events))
      varargout{1}.ie = solution.event{2};  # Index info which event occurred
      varargout{1}.xe = solution.event{3};  # Time info when an event occurred
      varargout{1}.ye = solution.event{4};  # Results when an event occurred
    endif
    if (strcmp (odeopts.Stats, "on"))
      varargout{1}.stats = struct ();
      varargout{1}.stats.nsteps   = nsteps;
      varargout{1}.stats.nfailed  = nfailed;
      varargout{1}.stats.nfevals  = nfevals;
      varargout{1}.stats.npds     = npds;
      varargout{1}.stats.ndecomps = ndecomps;
      varargout{1}.stats.nlinsols = nlinsols;
    endif
  elseif (nargout == 5)
    varargout = cell (1,5);
    varargout{1} = solution.t;
    varargout{2} = solution.x;
    if (! isempty (odeopts.Events))
      varargout{3} = solution.event{3};  # Time info when an event occurred
      varargout{4} = solution.event{4};  # Results when an event occurred
      varargout{5} = solution.event{2};  # Index info which event occurred
    endif
  endif

endfunction


%!demo
%! ## Demonstrate convergence order for ode23
%! tol = 1e-5 ./ 10.^[0:8];
%! for i = 1 : numel (tol)
%!   opt = odeset ("RelTol", tol(i), "AbsTol", realmin);
%!   [t, y] = ode23 (@(t, y) -y, [0, 1], 1, opt);
%!   h(i) = 1 / (numel (t) - 1);
%!   err(i) = norm (y .* exp (t) - 1, Inf);
%! endfor
%!
%! ## Estimate order visually
%! loglog (h, tol, "-ob",
%!         h, err, "-b",
%!         h, (h/h(end)) .^ 2 .* tol(end), "k--",
%!         h, (h/h(end)) .^ 3 .* tol(end), "k-");
%! axis tight
%! xlabel ("h");
%! ylabel ("err(h)");
%! title ("Convergence plot for ode23");
%! legend ("imposed tolerance", "ode23 (relative) error",
%!         "order 2", "order 3", "location", "northwest");
%!
%! ## Estimate order numerically
%! p = diff (log (err)) ./ diff (log (h))

## We are using the "Van der Pol" implementation for all tests that are done
## for this function.
## For further tests we also define a reference solution (computed at high
## accuracy)
%!function ydot = fpol (t, y)  # The Van der Pol ODE
%!  ydot = [y(2); (1 - y(1)^2) * y(2) - y(1)];
%!endfunction
%!function ref = fref ()       # The computed reference sol
%!  ref = [0.32331666704577, -1.83297456798624];
%!endfunction
%!function [val, trm, dir] = feve (t, y, varargin)
%!  val = fpol (t, y, varargin);    # We use the derivatives
%!  trm = zeros (2,1);              # that's why component 2
%!  dir = ones (2,1);               # does not seem to be exact
%!endfunction
%!function [val, trm, dir] = fevn (t, y, varargin)
%!  val = fpol (t, y, varargin);    # We use the derivatives
%!  trm = ones (2,1);               # that's why component 2
%!  dir = ones (2,1);               # does not seem to be exact
%!endfunction
%!function mas = fmas (t, y, varargin)
%!  mas = [1, 0; 0, 1];             # Dummy mass matrix for tests
%!endfunction
%!function mas = fmsa (t, y, varargin)
%!  mas = sparse ([1, 0; 0, 1]);    # A sparse dummy matrix
%!endfunction
%!function out = fout (t, y, flag, varargin)
%!  out = false;
%!  if (strcmp (flag, "init"))
%!    if (! isequal (size (t), [2, 1]))
%!      error ('fout: step "init"');
%!    endif
%!  elseif (isempty (flag))
%!    if (! isequal (size (t), [1, 1]))
%!      error ('fout: step "calc"');
%!    endif
%!  elseif (strcmp (flag, "done"))
%!    if (! isempty (t))
%!      warning ('fout: step "done"');
%!    endif
%!  else
%!    error ("fout: invalid flag <%s>", flag);
%!  endif
%!endfunction
%!
%!test  # two output arguments
%! [t, y] = ode23 (@fpol, [0 2], [2 0]);
%! assert ([t(end), y(end,:)], [2, fref], 1e-3);
%!test  # anonymous function instead of real function
%! fvdp = @(t,y) [y(2); (1 - y(1)^2) * y(2) - y(1)];
%! [t, y] = ode23 (fvdp, [0 2], [2 0]);
%! assert ([t(end), y(end,:)], [2, fref], 1e-3);
%!test  # extra input arguments passed through
%! [t, y] = ode23 (@fpol, [0 2], [2 0], 12, 13, "KL");
%! assert ([t(end), y(end,:)], [2, fref], 1e-3);
%!test  # empty OdePkg structure *but* extra input arguments
%! opt = odeset;
%! [t, y] = ode23 (@fpol, [0 2], [2 0], opt, 12, 13, "KL");
%! assert ([t(end), y(end,:)], [2, fref], 1e-2);
%!test  # Solve another anonymous function below zero
%! ref = [0, 14.77810590694212];
%! [t, y] = ode23 (@(t,y) y, [-2 0], 2);
%! assert ([t(end), y(end,:)], ref, 1e-2);
%!test  # InitialStep option
%! opt = odeset ("InitialStep", 1e-8);
%! [t, y] = ode23 (@fpol, [0 0.2], [2 0], opt);
%! assert ([t(2)-t(1)], [1e-8], 1e-9);
%!test  # MaxStep option
%! opt = odeset ("MaxStep", 1e-3);
%! sol = ode23 (@fpol, [0 0.2], [2 0], opt);
%! assert ([sol.x(5)-sol.x(4)], [1e-3], 1e-4);
%!test  # Solve in backward direction starting at t=0
%! ref = [-1.205364552835178, 0.951542399860817];
%! sol = ode23 (@fpol, [0 -2], [2 0]);
%! assert ([sol.x(end); sol.y(:,end)], [-2; ref'], 5e-3);
%!test  # Solve in backward direction starting at t=2
%! ref = [-1.205364552835178, 0.951542399860817];
%! sol = ode23 (@fpol, [2 0 -2], fref);
%! assert ([sol.x(end); sol.y(:,end)], [-2; ref'], 2e-2);
%!test  # Solve another anonymous function in backward direction
%! ref = [-1, 0.367879437558975];
%! sol = ode23 (@(t,y) y, [0 -1], 1);
%! assert ([sol.x(end); sol.y(:,end)], ref', 1e-2);
%!test  # Solve another anonymous function below zero
%! ref = [0, 14.77810590694212];
%! sol = ode23 (@(t,y) y, [-2 0], 2);
%! assert ([sol.x(end); sol.y(:,end)], ref', 1e-2);
%!test  # Solve in backward direction starting at t=0 with MaxStep option
%! ref = [-1.205364552835178, 0.951542399860817];
%! opt = odeset ("MaxStep", 1e-3);
%! sol = ode23 (@fpol, [0 -2], [2 0], opt);
%! assert ([abs(sol.x(8)-sol.x(7))], [1e-3], 1e-3);
%! assert ([sol.x(end); sol.y(:,end)], [-2; ref'], 1e-3);
%!test  # AbsTol option
%! opt = odeset ("AbsTol", 1e-5);
%! sol = ode23 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end); sol.y(:,end)], [2; fref'], 1e-3);
%!test  # AbsTol and RelTol option
%! opt = odeset ("AbsTol", 1e-8, "RelTol", 1e-8);
%! sol = ode23 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end); sol.y(:,end)], [2; fref'], 1e-3);
%!test  # RelTol and NormControl option -- higher accuracy
%! opt = odeset ("RelTol", 1e-8, "NormControl", "on");
%! sol = ode23 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end); sol.y(:,end)], [2; fref'], 1e-4);
%!test  # Keeps initial values while integrating
%! opt = odeset ("NonNegative", 2);
%! sol = ode23 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end); sol.y(:,end)], [2; 2; 0], 1e-1);
%!test  # Details of OutputSel and Refine can't be tested
%! opt = odeset ("OutputFcn", @fout, "OutputSel", 1, "Refine", 5);
%! sol = ode23 (@fpol, [0 2], [2 0], opt);
%!test  # Stats must add further elements in sol
%! opt = odeset ("Stats", "on");
%! sol = ode23 (@fpol, [0 2], [2 0], opt);
%! assert (isfield (sol, "stats"));
%! assert (isfield (sol.stats, "nsteps"));
%!test  # Events option add further elements in sol
%! opt = odeset ("Events", @feve);
%! sol = ode23 (@fpol, [0 10], [2 0], opt);
%! assert (isfield (sol, "ie"));
%! assert (sol.ie(1), 2);
%! assert (isfield (sol, "xe"));
%! assert (isfield (sol, "ye"));
%!test  # Events option, now stop integration
%! warning ("off", "integrate_adaptive:unexpected_termination", "local");
%! opt = odeset ("Events", @fevn, "NormControl", "on");
%! sol = ode23 (@fpol, [0 10], [2 0], opt);
%! assert ([sol.ie, sol.xe, sol.ye],
%!         [2.0, 2.496110, -0.830550, -2.677589], .5e-1);
%!test  # Events option, five output arguments
%! warning ("off", "integrate_adaptive:unexpected_termination", "local");
%! opt = odeset ("Events", @fevn, "NormControl", "on");
%! [t, y, vxe, ye, vie] = ode23 (@fpol, [0 10], [2 0], opt);
%! assert ([vie, vxe, ye], [2.0, 2.496110, -0.830550, -2.677589], 1e-1);
%!test  # Mass option as function
%! opt = odeset ("Mass", @fmas);
%! sol = ode23 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end); sol.y(:,end)], [2; fref'], 1e-3);
%!test  # Mass option as matrix
%! opt = odeset ("Mass", eye (2,2));
%! sol = ode23 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end); sol.y(:,end)], [2; fref'], 1e-3);
%!test  # Mass option as sparse matrix
%! opt = odeset ("Mass", sparse (eye (2,2)));
%! sol = ode23 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end); sol.y(:,end)], [2; fref'], 1e-3);
%!test  # Mass option as function and sparse matrix
%! opt = odeset ("Mass", @fmsa);
%! sol = ode23 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end); sol.y(:,end)], [2; fref'], 1e-3);
%!test  # Mass option as function and MStateDependence
%! opt = odeset ("Mass", @fmas, "MStateDependence", "strong");
%! sol = ode23 (@fpol, [0 2], [2 0], opt);
%! assert ([sol.x(end); sol.y(:,end)], [2; fref'], 1e-3);

## Note: The following options have no effect on this solver
##       therefore it makes no sense to test them here:
##
## "BDF"
## "InitialSlope"
## "JPattern"
## "Jacobian"
## "MassSingular"
## "MaxOrder"
## "MvPattern"
## "Vectorized"

%!test # Check that imaginary part of solution does not get inverted
%! sol = ode23 (@(x,y) 1, [0 1], 1i);
%! assert (imag (sol.y), ones (size (sol.y)))
%! [x, y] = ode23 (@(x,y) 1, [0 1], 1i);
%! assert (imag (y), ones (size (y)))

## Test input validation
%!error ode23 ()
%!error ode23 (1)
%!error ode23 (1,2)
%!error <TRANGE must be a numeric> ode23 (@fpol, {[0 25]}, [3 15 1])
%!error <TRANGE must be a .* vector> ode23 (@fpol, [0 25; 25 0], [3 15 1])
%!error <TRANGE must contain at least 2 elements> ode23 (@fpol, [1], [3 15 1])
%!error <invalid time span>  ode23 (@fpol, [1 1], [3 15 1])
%!error <INIT must be a numeric> ode23 (@fpol, [0 25], {[3 15 1]})
%!error <INIT must be a .* vector> ode23 (@fpol, [0 25], [3 15 1; 3 15 1])
%!error <FUN must be a valid function handle> ode23 (1, [0 25], [3 15 1])

