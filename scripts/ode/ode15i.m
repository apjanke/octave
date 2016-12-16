## Copyright (C) 2016, Francesco Faccio <francesco.faccio@mail.polimi.it>
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
## @deftypefn  {} {[@var{t}, @var{y}] =} ode15i (@var{fun}, @var{trange}, @var{y0}, @var{yp0})
## @deftypefnx {} {[@var{t}, @var{y}] =} ode15i (@var{fun}, @var{trange}, @var{y0}, @var{yp0}, @var{ode_opt})
## @deftypefnx {} {[@var{t}, @var{y}, @var{te}, @var{ye}, @var{ie}] =} ode15i (@dots{})
## @deftypefnx {} {@var{solution} =} ode15i (@dots{})
##
## Solve a set of full-implicit Ordinary Differential Equations and
## Differential Algebraic Equations (DAEs) of index 1, with the variable-step,
## variable order BDF (Backward Differentiation Formula) method, which
## ranges from order 1 to 5.
##
## @var{fun} is a function handle, inline function, or string containing the
## name of the function that defines the ODE: @code{f(@var{t},@var{y},@var{yp})}.
## The function must accept three inputs where the first is time @var{t}, the
## second is a column vector of unknowns @var{y} and the third is a column
## vector of unknowns @var{yp}.
##
## @var{trange} specifies the time interval over which the ODE will be
## evaluated.  Typically, it is a two-element vector specifying the initial and
## final times (@code{[tinit, tfinal]}).  If there are more than two elements
## then the solution will also be evaluated at these intermediate time.
##
## @var{y0} and @var{yp0} contain the initial values for the unknowns @var{y}
## and @var{yp}.  If they are row vectors then the solution @var{y} will be a
## matrix in which each column is the solution for the corresponding initial
## value in @var{y0} and @var{yp0}.
##
## @var{y0} and @var{yp0} must be consistent initial conditions, meaning that
## @code{f(@var{t},@var{y0},@var{yp0})=0} is satisfied. You can use function
## decic to compute consistent initial conditions, given initial guesses.
##
## The optional fifth argument @var{ode_opt} specifies non-default options to
## the ODE solver.  It is a structure generated by @code{odeset}.
##
## The function typically returns two outputs.  Variable @var{t} is a
## column vector and contains the times where the solution was found.  The
## output @var{y} is a matrix in which each column refers to a different
## unknown of the problem and each row corresponds to a time in @var{t}.
##
## The output can also be returned as a structure @var{solution} which
## has field @var{x} containing the time where the solution was evaluated and
## field @var{y} containing the solution matrix for the times in @var{x}.
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
## Example: Solve the @nospell{Robetson}'s equations:
##
## @example
## @group
## function res = robertsidae(@var{t}, @var{y}, @var{yp})
## res = [-(@var{yp}(1) + 0.04*@var{y}(1) - 1e4*@var{y}(2)*@var{y}(3));
##        -(@var{yp}(2) - 0.04*@var{y}(1) + 1e4*@var{y}(2)*@var{y}(3) +
##        3e7*@var{y}(2)^2);
##        @var{y}(1) + @var{y}(2) + @var{y}(3) - 1];
## endfunction
## [@var{t},@var{y}] = ode15i (@@robertsidae, [0 1e3], [1; 0; 0],[-1e-4; 1e-4; 0]);
## @end group
## @end example
## @seealso{decic, odeset, odeget}
## @end deftypefn

function varargout = ode15i (fun, trange, y0, yp0, varargin)

  solver = 'ode15i';

  if (nargin < 4)
    print_usage ();
  endif

  n = numel (y0);

  if (nargin > 4)
   options = varargin{1};
  else
   options = odeset ();
  endif

  ## Check fun, trange, y0, yp0
  fun = check_default_input (fun, trange, solver, y0, yp0);

  if (! isempty (options.Jacobian))
    if (ischar (options.Jacobian))
      try
        options.Jacobian = str2func (options.Jacobian);
      catch
        warning (lasterr);
      end_try_catch
      if (! isa (options.Jacobian, "function_handle"))
        error ("Octave:invalid-input-arg",
               [solver ": invalid value assigned to field '%s'"], "Jacobian");
      endif
    endif
  endif

  if (! isempty (options.OutputFcn))
    if (ischar (options.OutputFcn))
      try
        options.OutputFcn = str2func (options.OutputFcn);
      catch
        warning (lasterr);
      end_try_catch
      if (! isa (options.OutputFcn, "function_handle"))
        error ("Octave:invalid-input-arg",
               [solver ": invalid value assigned to field '%s'"], "OutputFcn");
      endif
    endif
  endif

  if (! isempty (options.Events))
    if (ischar (options.Events))
      try
        options.Events = str2func (options.Events);
      catch
        warning (lasterr);
      end_try_catch
      if (! isa (options.Events, "function_handle") && ! ismatrix (options.Events))
        error ("Octave:invalid-input-arg",
               [solver ": invalid value assigned to field '%s'"], "Events");
      endif
    endif
  endif

  [defaults, classes, attributes] = ...
  odedefaults (n, trange(1), trange(end));

  persistent ignorefields = {"NonNegative", "Mass", ...
                             "MStateDependence", "MvPattern", ...
                             "MassSingular", "InitialSlope", "BDF"};

  defaults   = rmfield (defaults, ignorefields);
  classes    = rmfield (classes, ignorefields);
  attributes = rmfield (attributes, ignorefields);

  classes        = odeset (classes, 'Vectorized', {});
  attributes     = ...
  odeset (attributes, 'Jacobian', {}, 'Vectorized', {});

  options = ...
  odemergeopts ("ode15i", options, defaults,
                classes, attributes, solver);

  ## Jacobian
  options.havejac       = false;
  options.havejacsparse = false;
  options.havejacfun    = false;

  if (! isempty (options.Jacobian))
    options.havejac = true;
    if (iscell (options.Jacobian))
      if (numel (options.Jacobian) == 2)
        if (issparse (options.Jacobian{1}) && issparse (options.Jacobian{2})) ## Jac is sparse cell
          options.havejacsparse = true;
        endif

        if (any (size (options.Jacobian{1}) != [n n])
            || any (size (options.Jacobian{2}) != [n n])
            || ! isnumeric (options.Jacobian{1})
            || ! isnumeric (options.Jacobian{2})
            || ! isreal (options.Jacobian{1})
            || ! isreal (options.Jacobian{2}))
          error ("Octave:invalid-input-arg",
               [solver ": invalid value assigned to field '%s'"], "Jacobian");
        endif
      else
        error ("Octave:invalid-input-arg",
               [solver ": invalid value assigned to field '%s'"], "Jacobian");
      endif

    elseif (isa (options.Jacobian, "function_handle"))
      options.havejacfun = true;
      if (nargin (options.Jacobian) == 3)
        [A, B] = options.Jacobian (trange(1), y0, yp0);
        if (issparse (A) && issparse (B))
          options.havejacsparse = true;  ## Jac is sparse fun
        endif

        if (any (size (A) != [n n]) || any (size (B) != [n n])
            || ! isnumeric (A) || ! isnumeric (B) || ! isreal (A)
            || ! isreal (B))
          error ("Octave:invalid-input-arg",
               [solver ": invalid value assigned to field '%s'"], "Jacobian");
        endif
      else
        error ("Octave:invalid-input-arg",
               [solver ": invalid value assigned to field '%s'"], "Jacobian");
      endif
    else
        error ("Octave:invalid-input-arg",
               [solver ": invalid value assigned to field '%s'"], "Jacobian");
    endif
  endif

  ## Abstol and Reltol

  options.haveabstolvec = false;

  if (numel (options.AbsTol) != 1 && numel (options.AbsTol) != n)
    error ("Octave:invalid-input-arg",
           [solver ": invalid value assigned to field '%s'"], "AbsTol");

  elseif (numel (options.AbsTol) == n)
    options.haveabstolvec = true;
  endif

  ## Stats
  options.havestats = false;
  if (strcmp (options.Stats, "on"))
    options.havestats = true;
  endif

  ## Don't use Refine when the output is a structure
  if (nargout == 1)
    options.Refine = 1;
  endif

  ## OutputFcn and OutputSel
  if (isempty (options.OutputFcn) && nargout == 0)
    options.OutputFcn = @odeplot;
    options.haveoutputfunction = true;
  else
    options.haveoutputfunction = ! isempty (options.OutputFcn);
  endif

  options.haveoutputselection = ! isempty (options.OutputSel);
  if (options.haveoutputselection)
    options.OutputSel = options.OutputSel - 1;
  endif

  ## Events
  options.haveeventfunction = ! isempty (options.Events);


  [t, y, te, ye, ie] = __ode15__ (fun, trange, y0, yp0, options);


  if (nargout == 2)
    varargout{1} = t;
    varargout{2} = y;
  elseif (nargout == 1)
    varargout{1}.x = t;    # Time stamps are saved in field x
    varargout{1}.y = y;    # Results are saved in field y
    varargout{1}.solver = solver;
    if (options.haveeventfunction)
      varargout{1}.xe = te;  # Time info when an event occurred
      varargout{1}.ye = ye;  # Results when an event occurred
      varargout{1}.ie = ie;  # Index info which event occurred
    endif
  elseif (nargout == 5)
    varargout = cell (1,5);
    varargout{1} = t;
    varargout{2} = y;
    if (options.haveeventfunction)
      varargout{3} = te;  # Time info when an event occurred
      varargout{4} = ye;  # Results when an event occurred
      varargout{5} = ie;  # Index info which event occurred
    endif
  endif

endfunction

%!demo
%!
%! ##Solve Robertson's equations with ode15i
%! fun = @ (t, y, yp) [-(yp(1) + 0.04*y(1) - 1e4*y(2)*y(3));
%!                     -(yp(2) - 0.04*y(1) + 1e4*y(2)*y(3) + 3e7*y(2)^2);
%!                     y(1) + y(2) + y(3) - 1];
%!
%! opt = odeset ('RelTol',1e-4, 'AbsTol', [1e-8, 1e-14, 1e-6]);
%! y0 = [1; 0; 0];
%! yp0 = [-1e-4; 1e-4; 0];
%! tspan = [0 4*logspace(-6, 6)];
%!
%! [t, y] = ode15i (fun, tspan, y0, yp0, opt);
%!
%! y (:,2) = 1e4 * y (:, 2);
%! figure (2);
%! semilogx (t, y, 'o')
%! xlabel ('time');
%! ylabel ('species concentration');
%! title ('Robertson DAE problem with a Conservation Law');
%! legend ('y1', 'y2', 'y3');

%!function res = rob (t, y, yp)
%! res =[-(yp(1) + 0.04*y(1) - 1e4*y(2)*y(3));
%!       -(yp(2) - 0.04*y(1) + 1e4*y(2)*y(3) + 3e7*y(2)^2);
%!         y(1) + y(2) + y(3) - 1];
%!endfunction
%!
%!function ref = fref()
%! ref = [100, 0.617234887614937, 0.000006153591397, 0.382758958793666];
%!endfunction
%!
%!function ref2 = fref2()
%! ref2 = [4e6 0 0 1];
%!endfunction
%!
%!function [DFDY, DFDYP] = jacfundense(t, y, yp)
%!  DFDY = [-0.04,           1e4*y(3),  1e4*y(2);
%!           0.04, -1e4*y(3)-6e7*y(2), -1e4*y(2);
%!              1,                  1,         1];
%!  DFDYP = [-1,  0, 0;
%!            0, -1, 0;
%!            0,  0, 0];
%!endfunction
%!
%!function [DFDY, DFDYP] = jacfunsparse(t, y, yp)
%!  DFDY = sparse ([-0.04,           1e4*y(3),  1e4*y(2);
%!                   0.04, -1e4*y(3)-6e7*y(2), -1e4*y(2);
%!                      1,                  1,         1]);
%!  DFDYP = sparse ([-1,  0, 0;
%!                    0, -1, 0;
%!                    0,  0, 0]);
%!endfunction
%!function [DFDY, DFDYP] = jacwrong(t, y, yp)
%!  DFDY = [-0.04,           1e4*y(3);
%!           0.04, -1e4*y(3)-6e7*y(2)];
%!  DFDYP = [-1,  0;
%!            0, -1];
%!endfunction
%!function [DFDY, DFDYP, A] = jacwrong2(t, y, yp)
%!  DFDY = [-0.04,           1e4*y(3),  1e4*y(2);
%!           0.04, -1e4*y(3)-6e7*y(2), -1e4*y(2);
%!              1,                  1,         1];
%!  DFDYP = [-1,  0, 0;
%!            0, -1, 0;
%!            0,  0, 0];
%!  A = DFDY;
%!endfunction
%!function [val, isterminal, direction] = ff (t, y, yp)
%!  isterminal = [0 1];
%!  if (t < 1e1)
%!    val = [-1, -2];
%!  else
%!    val = [1 3];
%!  endif
%!
%!  direction = [1 0];
%!endfunction

%!test  # anonymous function instead of real function
%! ref = [0.049787079136413];
%! ff = @(t, u, udot)  udot + 3 * u;
%! [t, y] = ode15i (ff, 0:1, 1, -3);
%! assert ([t(end), y(end)], [1, ref], 1e-3);
%!test  # function passed as string
%! [t, y] = ode15i ('rob',[0 100 200], [1;0;0], [-1e-4;1e-4;0]);
%! assert ([t(2), y(2,:)], fref, 1e-3);
%!test  #  solve in intermidiate step
%! [t, y] = ode15i (@rob,[0 100 200], [1;0;0], [-1e-4;1e-4;0]);
%! assert ([t(2), y(2,:)], fref, 1e-3);
%!test  # numel(trange) = 2 final value
%! [t, y] = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0]);
%! assert ([t(end), y(end,:)], fref, 1e-5);
%!test  # With empty options
%! opt = odeset();
%! [t, y] = ode15i (@rob,[0 1e6 2e6 3e6 4e6], [1;0;0], [-1e-4;1e-4;0], opt);
%! assert ([t(end), y(end,:)], fref2, 1e-3);
%! opt = odeset();
%!test  # Without options
%! [t, y] = ode15i (@rob,[0 1e6 2e6 3e6 4e6], [1;0;0], [-1e-4;1e-4;0]);
%! assert ([t(end), y(end,:)], fref2, 1e-3);
%!test  # InitialStep option
%! opt = odeset ("InitialStep", 1e-8);
%! [t, y] = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0], opt);
%! assert ([t(2)-t(1)], [1e-8], 1e-9);
%!test  # MaxStep option
%! opt = odeset ("MaxStep", 1e-3);
%! [t, y] = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0]);
%! assert ([t(5)-t(4)], [1e-3], 1e-3);
%!test  # AbsTol scalar option
%! opt = odeset ("AbsTol", 1e-8);
%! [t, y] = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0], opt);
%! assert ([t(end), y(end,:)], fref, 1e-3);
%!test  # AbsTol scalar and RelTol option
%! opt = odeset ("AbsTol", 1e-8, "RelTol", 1e-6);
%! [t, y] = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0], opt);
%! assert ([t(end), y(end,:)], fref, 1e-3);
%!test  # AbsTol vector option
%! opt = odeset ("AbsTol", [1e-8, 1e-14,1e-6]);
%! [t, y] = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0], opt);
%! assert ([t(end), y(end,:)], fref, 1e-3);
%!test  # AbsTol vector and RelTol option
%! opt = odeset ("AbsTol", [1e-8, 1e-14,1e-6], "RelTol", 1e-6);
%! [t, y] = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0], opt);
%! assert ([t(end), y(end,:)], fref, 1e-3);
%!test  # RelTol option
%! opt = odeset ("RelTol", 1e-6);
%! [t, y] = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0], opt);
%! assert ([t(end), y(end,:)], fref, 1e-3);
%!test  # Jacobian fun dense
%! opt = odeset ("Jacobian", @jacfundense);
%! [t, y] = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0], opt);
%! assert ([t(end), y(end,:)], fref, 1e-3);
%!test  # Jacobian fun dense as string
%! opt = odeset ("Jacobian", 'jacfundense');
%! [t, y] = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0], opt);
%! assert ([t(end), y(end,:)], fref, 1e-3);
%!test  # Jacobian fun sparse
%! opt = odeset ("Jacobian", @jacfunsparse, "AbsTol", 1e-7, "RelTol", 1e-7);
%! [t, y] = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0], opt);
%! assert ([t(end), y(end,:)], fref, 1e-3);
%!test  # Solve in backward direction starting at t=100
%! YPref = [-0.001135972751027; -0.000000027483627; 0.001136000234654];
%! Yref = [0.617234887614937, 0.000006153591397, 0.382758958793666];
%! [t, y] = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0]);
%! [t2, y2] = ode15i (@rob,[100 0], Yref', YPref);
%! assert ([t2(end), y2(end,:)], [0 1 0 0], 2e-2);
#%!test  # Solve in backward direction with MaxStep option
%! YPref = [-0.001135972751027; -0.000000027483627; 0.001136000234654];
%! Yref = [0.617234887614937, 0.000006153591397, 0.382758958793666];
%! opt = odeset ('MaxStep', 1e-2);
%! [t, y] = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0]);
%! [t2, y2] = ode15i (@rob,[100 0], Yref', YPref, opt);
%! assert ([t2(end), y2(end,:)], [0 1 0 0], 2e-2);
%! assert ([t2(9)-t2(10)], [1e-2], 1e-2);
#%!test  # Solve in backward direction starting  with intermidiate step
%! YPref = [-0.001135972751027; -0.000000027483627; 0.001136000234654];
%! Yref = [0.617234887614937, 0.000006153591397, 0.382758958793666];
%! [t, y] = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0]);
%! [t2, y2] = ode15i (@rob,[100 5 0], Yref', YPref);
%! assert ([t2(end), y2(end,:)], [0 1 0 0], 2e-2);
%!test  # Refine
%! opt = odeset ("Refine", 3);
%! [t, y] = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0]);
%! [t2, y2] = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0], opt);
%! assert ([numel(t2)], numel(t)*3, 3);
%!test  # Refine ignored if numel (trange) > 2
%! opt = odeset ("Refine", 3);
%! [t, y] = ode15i (@rob,[0 10 100], [1;0;0], [-1e-4;1e-4;0]);
%! [t2, y2] = ode15i (@rob,[0 10 100], [1;0;0], [-1e-4;1e-4;0], opt);
%! assert ([numel(t2)], numel(t));
%!test  # Events option add further elements in sol
%! opt = odeset ("Events", @ff);
%! sol = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0], opt);
%! assert (isfield (sol, "ie"));
%! assert (sol.ie, [0;1]);
%! assert (isfield (sol, "xe"));
%! assert (isfield (sol, "ye"));
%! assert (sol.x(end), 10, 1);
%!test  # Events option, five output arguments
%! opt = odeset ("Events", @ff);
%! [t, y, te, ye, ie] = ode15i (@rob,[0 100], [1;0;0], [-1e-4;1e-4;0], opt);
%! assert ([t(end), te', ie'], [10, 10, 10, 0, 1], [1, 0.2, 0.2, 0, 0]);

%!error  # Jacobian fun wrong dimension
%! opt = odeset ("Jacobian", @jacwrong);
%! [t, y] = ode15i (@rob,[0 4e6], [1;0;0], [-1e-4;1e-4;0], opt);
%!error  # Jacobian cell dense wrong dimension
%!  DFDY = [-0.04, 1;
%!           0.04, 1];
%!  DFDYP = [-1,  0, 0;
%!            0, -1, 0;
%!            0,  0, 0];
%! opt = odeset ("Jacobian", {DFDY, DFDYP});
%! [t, y] = ode15i (@rob,[0 4e6], [1;0;0], [-1e-4;1e-4;0], opt);
%!error  # Jacobian cell sparse wrong dimension
%!  DFDY = sparse ([-0.04, 1;
%!                   0.04, 1]);
%!  DFDYP = sparse ([-1,  0, 0;
%!                    0, -1, 0;
%!                    0,  0, 0]);
%! opt = odeset ("Jacobian", {DFDY, DFDYP});
%! [t, y] = ode15i (@rob,[0 4e6], [1;0;0], [-1e-4;1e-4;0], opt);
%!error  # Jacobian cell wrong number of matrices
%! A = [1 2 3; 4 5 6; 7 8 9];
%! opt = odeset ("Jacobian", {A,A,A});
%! [t, y] = ode15i (@rob,[0 4e6], [1;0;0], [-1e-4;1e-4;0], opt);
%!error  # Jacobian single matrix
%! opt = odeset ("Jacobian", [1 2 3; 4 5 6; 7 8 9]);
%! [t, y] = ode15i (@rob,[0 4e6], [1;0;0], [-1e-4;1e-4;0], opt);
%!error  # Jacobian single matrix wrong dimension
%! opt = odeset ("Jacobian", [1 2 3; 4 5 6]);
%! [t, y] = ode15i (@rob,[0 4e6], [1;0;0], [-1e-4;1e-4;0], opt);
%!error  # Jacobian strange field
%! opt = odeset ("Jacobian", "foo");
%! [t, y] = ode15i (@rob,[0 4e6], [1;0;0], [-1e-4;1e-4;0], opt);
%!function ydot = fun (t, y, yp)
%! ydot = [y - yp];
%!endfunction
%!error ode15i ();
%!error ode15i (1);
%!error ode15i (1, 1, 1);
%!error ode15i (1, 1, 1);
%!error ode15i (1, 1, 1, 1);
%!error ode15i (1, 1, 1, 1, 1);
%!error ode15i (1, 1, 1, 1, 1, 1);
%!error ode15i (@fun, 1, 1, 1);
%!error ode15i (@fun, [1, 1], 1, 1);
%!error ode15i (@fun, [1, 2], [1], [1, 2]);
%!error
%!  opt = odeset ('RelTol', "foo");
%!  [t, y] = ode15i (@fun, [0 2], [2], [2], opt);
%!error
%!  opt = odeset ('RelTol', [1, 2]);
%!  [t, y] = ode15i (@fun, [0 2], [2], [2], opt);
%!error
%!  opt = odeset ('RelTol', -2);
%!  [t, y] = ode15i (@fun, [0 2], [2], [2], opt);
%!error
%!  opt = odeset ('AbsTol', "foo");
%!  [t, y] = ode15i (@fun, [0 2], [2], [2], opt);
%!error
%!  opt = odeset ('AbsTol', -1);
%!  [t, y] = ode15i (@fun, [0 2], [2], [2], opt);
%!error
%!  opt = odeset ('AbsTol', [1, 1, 1]);
%!  [t, y] = ode15i (@fun, [0 2], [2], [2], opt);


