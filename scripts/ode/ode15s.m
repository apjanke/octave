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
## @deftypefn  {} {[@var{t}, @var{y}] =} ode15s (@var{fun}, @var{trange}, @var{y0})
## @deftypefnx {} {[@var{t}, @var{y}] =} ode15s (@var{fun}, @var{trange}, @var{y0}, @var{ode_opt})
## @deftypefnx {} {[@var{t}, @var{y}, @var{te}, @var{ye}, @var{ie}] =} ode15s (@dots{})
## @deftypefnx {} {@var{solution} =} ode15s (@dots{})
## @deftypefnx {} {} ode15s (@dots{})
##
## Solve a set of stiff Ordinary Differential Equations and stiff semi-explicit
## Differential Algebraic Equations (DAEs) of index 1, with the variable-step,
## variable order BDF (Backward Differentiation Formula) method, which ranges
## from order 1 to 5.
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
## If no output arguments are requested, and no @code{OutputFcn} is
## specified in @var{ode_opt}, then the @code{OutputFcn} is set to
## @code{odeplot} and the results of the solver are plotted immediately.
##
## If using the @qcode{"Events"} option then three additional outputs may
## be returned.  @var{te} holds the time when an Event function returned a
## zero.  @var{ye} holds the value of the solution at time @var{te}.  @var{ie}
## contains an index indicating which Event function was triggered in the case
## of multiple Event functions.
##
## Example: Solve the @nospell{Robetson's} equations:
##
## @example
## @group
## function r = robertsidae (@var{t}, @var{y})
##   r = [-0.04*@var{y}(1) + 1e4*@var{y}(2)*@var{y}(3);
##         0.04*@var{y}(1) - 1e4*@var{y}(2)*@var{y}(3) - 3e7*@var{y}(2)^2;
##         @var{y}(1) + @var{y}(2) + @var{y}(3) - 1];
## endfunction
## opt = odeset ("Mass", [1 0 0; 0 1 0; 0 0 0], "MStateDependence", "none");
## [@var{t},@var{y}] = ode15s (@@robertsidae, [0, 1e3], [1; 0; 0], opt);
## @end group
## @end example
## @seealso{decic, odeset, odeget}
## @end deftypefn

function varargout = ode15s (fun, trange, y0, varargin)

  solver = "ode15s";

  if (nargin < 3)
    print_usage ();
  endif

  ## Check fun, trange, y0, yp0
  fun = check_default_input (fun, trange, solver, y0);

  n = numel (y0);

  if (nargin > 3)
   options = varargin{1};
  else
   options = odeset ();
  endif

  if (! isempty (options.Mass))
    if (ischar (options.Mass))
      try
        options.Mass = str2func (options.Mass);
      catch
        warning (lasterr);
      end_try_catch
      if (! isa (options.Mass, "function_handle"))
        error ("Octave:invalid-input-arg",
               [solver ": invalid value assigned to field 'Mass'"]);
      endif
    endif
  endif

  if (! isempty (options.Jacobian))
    if (ischar (options.Jacobian))
      try
        options.Jacobian = str2func (options.Jacobian);
      catch
        warning (lasterr);
      end_try_catch
      if (! isa (options.Jacobian, "function_handle"))
        error ("Octave:invalid-input-arg",
               [solver ": invalid value assigned to field 'Jacobian'"]);
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
      if (! isa (options.Events, "function_handle"))
        error ("Octave:invalid-input-arg",
               [solver ": invalid value assigned to field 'Events'"]);
      endif
    endif
  endif

  [defaults, classes, attributes] = odedefaults (n, trange(1), trange(end));

  classes    = odeset (classes, "Vectorized", {});
  attributes = odeset (attributes, "Jacobian", {}, "Vectorized", {});

  options = odemergeopts ("ode15s", options, defaults,
                          classes, attributes, solver);

  ## Mass
  options.havemassfun    = false;
  options.havestatedep   = false;
  options.havetimedep    = false;
  options.havemasssparse = false;

  if (! isempty (options.Mass))
    if (isa (options.Mass, "function_handle"))
      options.havemassfun = true;
      if (nargin (options.Mass) == 2)
        options.havestatedep = true;
        M = options.Mass (trange(1), y0);
        options.havemasssparse = issparse (M);
        if (any (size (M) != [n n]) || ! isnumeric (M) || ! isreal (M))
          error ("Octave:invalid-input-arg",
                 [solver ": invalid value assigned to field 'Mass'"]);
        endif
      elseif (nargin (options.Mass) == 1)
        options.havetimedep = true;
        M = options.Mass (trange(1));
        options.havemasssparse = issparse (M);
        if (any (size (M) != [n n]) || ! isnumeric (M) || ! isreal (M))
          error ("Octave:invalid-input-arg",
                 [solver ": invalid value assigned to field 'Mass'"]);
        endif
      else
        error ("Octave:invalid-input-arg",
               [solver ": invalid value assigned to field 'Mass'"]);
      endif
    elseif (ismatrix (options.Mass))
      options.havemasssparse = issparse (options.Mass);
      if (any (size (options.Mass) != [n n]) ||
          ! isnumeric (options.Mass) || ! isreal (options.Mass))
        error ("Octave:invalid-input-arg",
               [solver ": invalid value assigned to field 'Mass'"]);
      endif
    else
      error ("Octave:invalid-input-arg",
             [solver ": invalid value assigned to field 'Mass'"]);
    endif
  endif

  ## Jacobian
  options.havejac       = false;
  options.havejacsparse = false;
  options.havejacfun    = false;

  if (! isempty (options.Jacobian))
    options.havejac = true;
    if (isa (options.Jacobian, "function_handle"))
      options.havejacfun = true;
      if (nargin (options.Jacobian) == 2)
        [A] = options.Jacobian (trange(1), y0);
        if (issparse (A))
          options.havejacsparse = true;  # Jac is sparse fun
        endif
        if (any (size (A) != [n n]) || ! isnumeric (A) || ! isreal (A))
          error ("Octave:invalid-input-arg",
                 [solver ": invalid value assigned to field 'Jacobian'"]);
        endif
      else
        error ("Octave:invalid-input-arg",
               [solver ": invalid value assigned to field 'Jacobian'"]);
      endif
    elseif (ismatrix (options.Jacobian))
      if (issparse (options.Jacobian))
        options.havejacsparse = true;    # Jac is sparse matrix
      endif
      if (! issquare (options.Jacobian))
        error ("Octave:invalid-input-arg",
               [solver ": invalid value assigned to field 'Jacobian'"]);
      endif
    else
      error ("Octave:invalid-input-arg",
             [solver ": invalid value assigned to field 'Jacobian'"]);
    endif
  endif

  ## Derivative of M(t,y) for implicit problem not implemented yet
  if (! isempty (options.Mass) && ! isempty (options.Jacobian))
    if (options.MStateDependence != "none" || options.havestatedep == true)
      options.havejac = false;
      options.Jacobian = [];
      warning ("ode15s:mass_state_dependent_provided",
              ["with MStateDependence != 'none' an internal", ...
               " approximation of Jacobian Matrix will be used.", ...
               " Set MStateDependence equal to 'none' if you want ", ...
               " to provide a constant or time-dependent Jacobian"]);
    endif
  endif

  ## Use sparse methods only if all matrices are sparse
  if (! options.havemasssparse)
    options.havejacsparse = false;
  endif

  ## If Mass or Jacobian is fun, then new Jacobian is fun
  if (options.havejac)
    if (options.havejacfun || options.havetimedep)
      options.Jacobian = @ (t, y, yp) wrapjacfun (t, y, yp,
                                                  options.Jacobian,
                                                  options.Mass,
                                                  options.havetimedep,
                                                  options.havejacfun);
      options.havejacfun = true;
    else   ## All matrices are constant
      options.Jacobian = {[- options.Jacobian], [options.Mass]};

    endif
  endif

  ## Abstol and Reltol
  options.haveabstolvec = false;

  if (numel (options.AbsTol) != 1 && numel (options.AbsTol) != n)
    error ("Octave:invalid-input-arg",
           [solver ": invalid value assigned to field 'AbsTol'"]);
  elseif (numel (options.AbsTol) == n)
    options.haveabstolvec = true;
  endif

  ## Stats
  options.havestats = strcmpi (options.Stats, "on");

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

  yp0 = options.InitialSlope;

  [t, y, te, ye, ie] = __ode15__ (@ (t, y, yp) wrap (t, y, yp, options.Mass,
                                                     options.havetimedep,
                                                     options.havestatedep,
                                                     fun),
                                  trange, y0, yp0, options);

  if (nargout == 2)
    varargout{1} = t;
    varargout{2} = y;
  elseif (nargout == 1)
    varargout{1}.x = t;  # Time stamps are saved in field x
    varargout{1}.y = y;  # Results are saved in field y
    varargout{1}.solver = solver;
    if (options.haveeventfunction)
      varargout{1}.xe = te;  # Time info when an event occurred
      varargout{1}.ye = ye;  # Results when an event occurred
      varargout{1}.ie = ie;  # Index info which event occurred
    endif
  elseif (nargout > 2)
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

function res = wrap (t, y, yp, Mass, havetimedep, havestatedep, fun)

  if (! isempty (Mass) && havestatedep)
    res = Mass (t, y) * yp - fun (t, y);
  elseif (! isempty (Mass) && havetimedep)
    res = Mass (t) * yp - fun (t, y);
  elseif (! isempty (Mass))
    res = Mass * yp - fun (t, y);
  else
    res = yp - fun (t, y);
  endif

endfunction

function [jac, jact] = wrapjacfun (t, y, yp, Jac, Mass,
                                   havetimedep, havejacfun)
  if (havejacfun)
    jac = - Jac (t, y);
  else
    jac = - Jac;
  endif

  if (! isempty (Mass) && havetimedep)
    jact = Mass (t);
  elseif (! isempty (Mass))
    jact = Mass;
  else
    jact = speye (numel (y));
  endif

endfunction


%!demo
%! ## Solve Robertson's equations with ode15s
%! fun = @ (t, y) [-0.04*y(1) + 1e4*y(2).*y(3);
%!                  0.04*y(1) - 1e4*y(2).*y(3) - 3e7*y(2).^2;
%!                  y(1) + y(2) + y(3) - 1];
%!
%! y0 = [1; 0; 0];
%! tspan = [0, 4*logspace(-6, 6)];
%! M = [1, 0, 0; 0, 1, 0; 0, 0, 0];
%!
%! options = odeset ("RelTol", 1e-4, "AbsTol", [1e-6, 1e-10, 1e-6],
%!                   "MStateDependence", "none", "Mass", M);
%!
%! [t, y] = ode15s (fun, tspan, y0, options);
%!
%! y(:,2) = 1e4 * y(:,2);
%! figure (2);
%! semilogx (t, y, "o");
%! xlabel ("time");
%! ylabel ("species concentration");
%! title ("Robertson DAE problem with a Conservation Law");
%! legend ("y1", "y2", "y3");

%!function ydot = fpol (t, y)  # Van der Pol equation
%!  ydot = [y(2); (1 - y(1)^2) * y(2) - y(1)];
%!endfunction
%!
%!function ref = fref ()       # The computed reference sol
%!  ref = [0.32331666704577, -1.83297456798624];
%!endfunction
%!
%!function jac = fjac (t, y)   # its Jacobian
%!  jac = [0, 1; -1 - 2 * y(1) * y(2), 1 - y(1)^2];
%!endfunction
%!
%!function jac = fjcc (t, y)   # sparse type
%!  jac = sparse ([0, 1; -1 - 2 * y(1) * y(2), 1 - y(1)^2]);
%!endfunction
%!
%!function mas = fmas (t, y)
%!  mas = [1, 0; 0, 1];           # Dummy mass matrix for tests
%!endfunction
%!
%!function mas = fmsa (t, y)
%!  mas = sparse ([1, 0; 0, 1]);  # A sparse dummy matrix
%!endfunction
%!
%!function res = rob (t, y)
%!  res = [-0.04*y(1) + 1e4*y(2).*y(3);
%!          0.04*y(1) - 1e4*y(2).*y(3) - 3e7*y(2).^2;
%!          y(1) + y(2) + y(3) - 1];
%!endfunction
%!
%!function refrob = frefrob ()
%!  refrob = [100, 0.617234887614937, 0.000006153591397, 0.382758958793666];
%!endfunction
%!
%!function [val, isterminal, direction] = feve (t, y)
%!  isterminal = [0, 1];
%!  if (t < 1e1)
%!    val = [-1, -2];
%!  else
%!    val = [1, 3];
%!  endif
%!
%!  direction = [1, 0];
%!endfunction
%!
%!function masrob = massdensefunstate (t, y)
%!  masrob = [1, 0, 0; 0, 1, 0; 0, 0, 0];
%!endfunction
%!
%!function masrob = masssparsefunstate (t, y)
%!  masrob = sparse ([1, 0, 0; 0, 1, 0; 0, 0, 0]);
%!endfunction
%!
%!function masrob = massdensefuntime (t)
%!  masrob = [1, 0, 0; 0, 1, 0; 0, 0, 0];
%!endfunction
%!
%!function masrob = masssparsefuntime (t)
%!  masrob = sparse ([1, 0, 0; 0, 1, 0; 0, 0, 0]);
%!endfunction
%!
%!function jac = jacfundense (t, y)
%!  jac = [-0.04,           1e4*y(3),  1e4*y(2);
%!          0.04, -1e4*y(3)-6e7*y(2), -1e4*y(2);
%!             1,                  1,         1];
%!endfunction
%!
%!function jac = jacfunsparse (t, y)
%!  jac = sparse([-0.04,           1e4*y(3),  1e4*y(2);
%!                 0.04, -1e4*y(3)-6e7*y(2), -1e4*y(2);
%!                    1,                  1,         1]);
%!endfunction

%!testif HAVE_SUNDIALS
%! opt = odeset ("MStateDependence", "none",
%!               "Mass", [1, 0, 0; 0, 1, 0; 0, 0, 0]);
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), y(end,:)], frefrob, 1e-3);

%!testif HAVE_SUNDIALS
%! opt = odeset ("MStateDependence", "none",
%!               "Mass", sparse ([1, 0, 0; 0, 1, 0; 0, 0, 0]));
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), y(end,:)], frefrob, 1e-3);

%!testif HAVE_SUNDIALS
%! opt = odeset ("MStateDependence", "none",
%!               "Mass", @massdensefunstate);
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), y(end,:)], frefrob, 1e-3);

%!testif HAVE_SUNDIALS
%! opt = odeset ("MStateDependence", "none",
%!               "Mass", @masssparsefunstate);
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), y(end,:)], frefrob, 1e-3);

%!testif HAVE_SUNDIALS
%! opt = odeset ("MStateDependence", "none",
%!               "Mass", "massdensefuntime");
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), y(end,:)], frefrob, 1e-3);

%!testif HAVE_SUNDIALS
%! opt = odeset ("MStateDependence", "none",
%!               "Mass", [1, 0, 0; 0, 1, 0; 0, 0, 0],
%!               "Jacobian", "jacfundense");
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), y(end,:)], frefrob, 1e-3);

%!testif HAVE_SUNDIALS
%! opt = odeset ("MStateDependence", "none",
%!               "Mass", sparse ([1, 0, 0; 0, 1, 0; 0, 0, 0]),
%!               "Jacobian", @jacfundense);
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), y(end,:)], frefrob, 1e-3);

%!testif HAVE_SUNDIALS
%! warning ("off", "ode15s:mass_state_dependent_provided", "local");
%! opt = odeset ("MStateDependence", "none",
%!               "Mass", @massdensefunstate,
%!               "Jacobian", @jacfundense);
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), y(end,:)], frefrob, 1e-3);

%!testif HAVE_SUNDIALS
%! warning ("off", "ode15s:mass_state_dependent_provided", "local");
%! opt = odeset ("MStateDependence", "none",
%!               "Mass", @masssparsefunstate,
%!               "Jacobian", @jacfundense);
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), y(end,:)], frefrob, 1e-3);

%!testif HAVE_SUNDIALS
%! opt = odeset ("MStateDependence", "none",
%!               "Mass", @massdensefuntime,
%!               "Jacobian", @jacfundense);
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), y(end,:)], frefrob, 1e-3);
%! opt = odeset ("MStateDependence", "none",
%!               "Mass", "masssparsefuntime",
%!               "Jacobian", "jacfundense");
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), y(end,:)], frefrob, 1e-3);

%!testif HAVE_SUNDIALS_IDAKLU
%! opt = odeset ("MStateDependence", "none",
%!               "Mass", [1, 0, 0; 0, 1, 0; 0, 0, 0],
%!               "Jacobian", @jacfunsparse);
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), y(end,:)], frefrob, 1e-3);

%!testif HAVE_SUNDIALS_IDAKLU
%! opt = odeset ("MStateDependence", "none",
%!               "Mass", sparse ([1, 0, 0; 0, 1, 0; 0, 0, 0]),
%!               "Jacobian", @jacfunsparse);
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), y(end,:)], frefrob, 1e-3);

%!testif HAVE_SUNDIALS_IDAKLU
%! warning ("off", "ode15s:mass_state_dependent_provided", "local");
%! opt = odeset ("MStateDependence", "none",
%!               "Mass", @massdensefunstate,
%!               "Jacobian", @jacfunsparse);
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), y(end,:)], frefrob, 1e-3);

%!testif HAVE_SUNDIALS
%! warning ("off", "ode15s:mass_state_dependent_provided", "local");
%! opt = odeset ("MStateDependence", "none",
%!               "Mass", @masssparsefunstate,
%!               "Jacobian", @jacfunsparse);
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), y(end,:)], frefrob, 1e-3);

%!testif HAVE_SUNDIALS_IDAKLU
%! opt = odeset ("MStateDependence", "none",
%!               "Mass", @massdensefuntime,
%!               "Jacobian", @jacfunsparse);
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), y(end,:)], frefrob, 1e-3);

%!testif HAVE_SUNDIALS_IDAKLU
%! opt = odeset ("MStateDependence", "none",
%!               "Mass", @masssparsefuntime,
%!               "Jacobian", @jacfunsparse);
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), y(end,:)], frefrob, 1e-3);

## two output arguments
%!testif HAVE_SUNDIALS
%! [t, y] = ode15s (@fpol, [0, 2], [2, 0]);
%! assert ([t(end), y(end,:)], [2, fref], 1e-2);

## anonymous function instead of real function
%!testif HAVE_SUNDIALS
%! fvdb = @(t,y) [y(2); (1 - y(1)^2) * y(2) - y(1)];
%! [t, y] = ode15s (fvdb, [0, 2], [2, 0]);
%! assert ([t(end), y(end,:)], [2, fref], 1e-2);

## Solve another anonymous function below zero
%!testif HAVE_SUNDIALS
%! ref = [0, 14.77810590694212];
%! [t, y] = ode15s (@(t,y) y, [-2, 0], 2);
%! assert ([t(end), y(end,:)], ref, 5e-2);

## InitialStep option
%!testif HAVE_SUNDIALS
%! opt = odeset ("InitialStep", 1e-8);
%! [t, y] = ode15s (@fpol, [0, 0.2], [2, 0], opt);
%! assert (t(2)-t(1), 1e-8, 1e-9);

## MaxStep option
%!testif HAVE_SUNDIALS
%! opt = odeset ("MaxStep", 1e-3);
%! sol = ode15s (@fpol, [0, 0.2], [2, 0], opt);
%! assert (sol.x(5)-sol.x(4), 1e-3, 1e-3);

## Solve in backward direction starting at t=0
%!testif HAVE_SUNDIALS
%! ref = [-1.205364552835178, 0.951542399860817];
%! sol = ode15s (@fpol, [0 -2], [2, 0]);
%! assert ([sol.x(end), sol.y(end,:)], [-2, ref], 5e-3);

## Solve in backward direction starting at t=2
%!testif HAVE_SUNDIALS
%! ref = [-1.205364552835178, 0.951542399860817];
%! sol = ode15s (@fpol, [2, 0 -2], fref);
%! assert ([sol.x(end), sol.y(end,:)], [-2, ref], 3e-2);

## Solve another anonymous function in backward direction
%!testif HAVE_SUNDIALS
%! ref = [-1, 0.367879437558975];
%! sol = ode15s (@(t,y) y, [0 -1], 1);
%! assert ([sol.x(end), sol.y(end,:)], ref, 1e-2);

## Solve another anonymous function below zero
%!testif HAVE_SUNDIALS
%! ref = [0, 14.77810590694212];
%! sol = ode15s (@(t,y) y, [-2, 0], 2);
%! assert ([sol.x(end), sol.y(end,:)], ref, 5e-2);

## Solve in backward direction starting at t=0 with MaxStep option
%!testif HAVE_SUNDIALS
%! ref = [-1.205364552835178, 0.951542399860817];
%! opt = odeset ("MaxStep", 1e-3);
%! sol = ode15s (@fpol, [0 -2], [2, 0], opt);
%! assert (abs (sol.x(8)-sol.x(7)), 1e-3, 1e-3);
%! assert ([sol.x(end), sol.y(end,:)], [-2, ref], 1e-3);

## AbsTol option
%!testif HAVE_SUNDIALS
%! opt = odeset ("AbsTol", 1e-5);
%! sol = ode15s (@fpol, [0, 2], [2, 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 4e-3);

## AbsTol and RelTol option
%!testif HAVE_SUNDIALS
%! opt = odeset ("AbsTol", 1e-8, "RelTol", 1e-8);
%! sol = ode15s (@fpol, [0, 2], [2, 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 1e-3);

## RelTol option -- higher accuracy
%!testif HAVE_SUNDIALS
%! opt = odeset ("RelTol", 1e-8);
%! sol = ode15s (@fpol, [0, 2], [2, 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 1e-4);

## Mass option as function
%!testif HAVE_SUNDIALS
%! opt = odeset ("Mass", @fmas, "MStateDependence", "none");
%! sol = ode15s (@fpol, [0, 2], [2, 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 3e-3);

## Mass option as matrix
%!testif HAVE_SUNDIALS
%! opt = odeset ("Mass", eye (2,2), "MStateDependence", "none");
%! sol = ode15s (@fpol, [0, 2], [2, 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 3e-3);

## Mass option as sparse matrix
%!testif HAVE_SUNDIALS
%! opt = odeset ("Mass", speye (2), "MStateDependence", "none");
%! sol = ode15s (@fpol, [0, 2], [2, 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 3e-3);

## Mass option as function and sparse matrix
%!testif HAVE_SUNDIALS
%! opt = odeset ("Mass", "fmsa", "MStateDependence", "none");
%! sol = ode15s (@fpol, [0, 2], [2, 0], opt);
%! assert ([sol.x(end), sol.y(end,:)], [2, fref], 3e-3);

## Refine
%!testif HAVE_SUNDIALS
%! opt2 = odeset ("Refine", 3, "Mass", @massdensefunstate,
%!                "MStateDependence", "none");
%! opt1 = odeset ("Mass", @massdensefunstate, "MStateDependence", "none");
%! [t, y] = ode15s (@rob, [0, 100], [1; 0; 0], opt1);
%! [t2, y2] = ode15s (@rob, [0, 100], [1; 0; 0], opt2);
%! assert (numel (t2), numel (t) * 3, 3);

## Refine ignored if numel (trange) > 2
%!testif HAVE_SUNDIALS
%! opt2 = odeset ("Refine", 3, "Mass", "massdensefunstate",
%!                "MStateDependence", "none");
%! opt1 = odeset ("Mass", @massdensefunstate, "MStateDependence", "none");
%! [t, y] = ode15s ("rob", [0, 10, 100], [1; 0; 0], opt1);
%! [t2, y2] = ode15s ("rob", [0, 10, 100], [1; 0; 0], opt2);
%! assert (numel (t2), numel (t));

## Events option add further elements in sol
%!testif HAVE_SUNDIALS
%! opt = odeset ("Events", @feve, "Mass", @massdensefunstate,
%!               "MStateDependence", "none");
%! sol = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert (isfield (sol, "ie"));
%! assert (sol.ie, [0;1]);
%! assert (isfield (sol, "xe"));
%! assert (isfield (sol, "ye"));
%! assert (sol.x(end), 10, 1);

## Events option, five output arguments
%!testif HAVE_SUNDIALS
%! opt = odeset ("Events", @feve, "Mass", @massdensefunstate,
%!               "MStateDependence", "none");
%! [t, y, te, ye, ie] = ode15s (@rob, [0, 100], [1; 0; 0], opt);
%! assert ([t(end), te', ie'], [10, 10, 10, 0, 1], [1, 0.5, 0.5, 0, 0]);
