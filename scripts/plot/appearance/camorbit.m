## Copyright (C) 2016 Colin B. Macdonald
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or (at
## your option) any later version.
##
## This software is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty
## of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public
## License along with this software; see the file COPYING.
## If not, see <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn  {} {} camorbit (@var{theta}, @var{phi})
## @deftypefnx {} {} camorbit (@var{theta}, @var{phi}, @var{coorsys})
## @deftypefnx {} {} camorbit (@var{theta}, @var{phi}, @var{coorsys}, @var{dir})
## @deftypefnx {} {} camorbit (@var{theta}, @var{phi}, "data")
## @deftypefnx {} {} camorbit (@var{theta}, @var{phi}, "data", "z")
## @deftypefnx {} {} camorbit (@var{theta}, @var{phi}, "data", "x")
## @deftypefnx {} {} camorbit (@var{theta}, @var{phi}, "data", "y")
## @deftypefnx {} {} camorbit (@var{theta}, @var{phi}, "data", [@var{x} @var{y} @var{z}])
## @deftypefnx {} {} camorbit (@var{theta}, @var{phi}, "camera")
## @deftypefnx {} {} camorbit (@var{hax}, @dots{})
## Rotate the camera up/down and left/right around its target.
##
## Move the camera @var{phi} degrees up and @var{theta} degrees to the right,
## as if it were in an orbit around its target.
## Example:
## @example
## @group
## @c doctest: +SKIP
## sphere ()
## camorbit (30, 20)
## @end group
## @end example
##
## These rotations are centered around the camera target
## (@pxref{XREFcamtarget,,camtarget}).
## First the camera position is pitched up or down by rotating it @var{phi}
## degrees around an axis orthogonal to both the viewing direction (specifically
## @code{camtarget() - campos()}) and the camera ``up vector''
## (@pxref{XREFcamup,,camup}).
## Example:
## @example
## @group
## @c doctest: +SKIP
## camorbit (0, 20)
## @end group
## @end example
##
## The second rotation depends on the coordinate system @var{coorsys} and
## direction @var{dir} inputs.
## The default for @var{coorsys} is @qcode{"data"}.  In this case, the camera is
## yawed left or right by rotating it @var{theta} degrees around an axis
## specified by @var{dir}.
## The default for @var{dir} is @qcode{"z"}, corresponding to the vector
## @code{[0, 0, 1]}.
## Example:
## @example
## @group
## @c doctest: +SKIP
## camorbit (30, 0)
## @end group
## @end example
##
##
## When @var{coorsys} is set to @qcode{"camera"}, the camera is moved left or
## right by rotating it around an axis parallel to the camera up vector
## (@pxref{XREFcamup,,camup}).
## The input @var{dir} should not be specified in this case.
## Example:
## @example
## @group
## @c doctest: +SKIP
## camorbit (30, 0, "camera")
## @end group
## @end example
## (Note: the rotation by @var{phi} is unaffected by @qcode{"camera"}.)
##
## The @code{camorbit} command modifies two camera properties:
## @pxref{XREFcampos,,campos} and @pxref{XREFcamup,,camup}.
##
## By default, this command affects the current axis; alternatively, an axis
## can be specified by the optional argument @var{hax}.
##
## @seealso{camzoom, camroll, camlookat}
## @end deftypefn


function camorbit (varargin)

  [hax, varargin, nargin] = __plt_get_axis_arg__ ("camorbit", varargin{:});

  if (nargin < 2 || nargin > 4)
    print_usage ();
  endif

  theta = varargin{1};
  phi = varargin{2};
  if (! (isnumeric (theta) && isscalar (theta)
         && isnumeric (phi) && isscalar (phi)))
    error ("camorbit: THETA and PHI must be numeric scalars")
  endif

  if (nargin < 3)
    coorsys = "data";
  else
    coorsys = varargin{3};
    if (! any (strcmpi (coorsys, {"data" "camera"})))
      error ("camorbit: COORSYS must be 'data' or 'camera'")
    endif
  endif

  if (nargin < 4)
    dir = "z";
  else
    if (strcmpi (coorsys, "camera"))
      error ("camorbit: DIR must not be used with 'camera' COORSYS.");
    endif
    dir = varargin{4};
  endif

  if (ischar (dir))
    switch tolower (dir)
      case "x"
        dir = [1 0 0];
      case "y"
        dir = [0 1 0];
      case "z"
        dir = [0 0 1];
      otherwise
        error ("camorbit: DIR must be 'x', 'y', 'z' or a numeric 3-element \
                vector.");
    endswitch
  endif
  if (! (isnumeric (dir) && numel (dir) == 3))
    error ("camorbit: DIR must be 'x', 'y', 'z' or a numeric 3-element \
            vector.");
  endif

  if (isempty (hax))
    hax = gca ();
  else
    hax = hax(1);
  endif

  view_ax = camtarget (hax) - campos (hax);
  view_ax /= norm (view_ax);
  ## orthogonalize the camup vector
  up = camup (hax) - view_ax*dot (camup (hax), view_ax);
  up /= norm (up);
  pitch_ax = cross (up, view_ax);

  if (strcmpi (coorsys, "camera"))
    yaw_ax = up;
  else
    yaw_ax = dir;
  end

  ## First pitch up then yaw right (order matters)
  pos = num2cell (campos (hax));
  [pos{:}] = __rotate_around_axis__ (pos{:}, phi, pitch_ax, camtarget (hax));
  [pos{:}] = __rotate_around_axis__ (pos{:}, theta, yaw_ax, camtarget (hax));
  pos = [pos{:}];

  up = num2cell (up);
  [up{:}] = __rotate_around_axis__ (up{:}, phi, pitch_ax, [0 0 0]);
  [up{:}] = __rotate_around_axis__ (up{:}, theta, yaw_ax, [0 0 0]);
  up = [up{:}];

  camup (hax, up)
  campos (hax, pos)

endfunction


%!demo
%! peaks ();
%! ## rotate the camera upwards
%! camorbit (0, 30);
%! ## rotate the camera right
%! camorbit (20, 0);


%!test
%! hf = figure ("visible", "off");
%! unwind_protect
%!   sphere ();
%!   campos ([20 0 0]);
%!   camorbit (0, 60);
%!   p = campos ();
%!   u = camup ();
%!   assert (p, [10 0 sqrt(3)*10], -eps);
%!   assert (u, [-sqrt(3)/2 0 0.5], -eps);
%! unwind_protect_cleanup
%!   close (hf);
%! end_unwind_protect

%!test
%! hf = figure ("visible", "off");
%! unwind_protect
%!   sphere ();
%!   camorbit(20, 30, "camera")
%!   p = campos ();
%!   u = camup ();
%!   ## Matlab 2008a
%!   pm = [-0.724972932190484  -9.37224596596009  14.5476946558943];
%!   um = [0.375634339316796  0.77045096344497  0.515076844803523];
%!   assert (p, pm, -5e-15);
%!   assert (u, um, -5e-15);
%! unwind_protect_cleanup
%!   close (hf);
%! end_unwind_protect

%!test
%! hf = figure ("visible", "off");
%! unwind_protect
%!   sphere ();
%!   camorbit(20, 30, "data", [1 2 3]);
%!   p = campos ();
%!   u = camup ();
%!   ## Matlab 2014a
%!   pm = [-0.215772672525099  -9.04926615428815  14.7669978066852];
%!   um = [0.413058199972826  0.773801198226611  0.48022351989284];
%!   assert (p, pm, -5e-15);
%!   assert (u, um, -5e-15);
%! unwind_protect_cleanup
%!   close (hf);
%! end_unwind_protect

%!test
%! hf = figure ("visible", "off");
%! unwind_protect
%!   sphere ();
%!   camorbit (54, 37);
%!   p = campos ();
%!   u = camup ();
%!   va = camva ();
%!   ## Matlab 2014a
%!   pm = [1.92211976102821  -6.48896756467585  15.9436117479337];
%!   um = [-0.261437503254928  0.882598219532154  0.390731128489274];
%!   vam = 10.1274850414735;
%!   assert (p, pm, -5e-15);
%!   assert (u, um, -5e-15);
%!   assert (va, vam, -5e-15);
%! unwind_protect_cleanup
%!   close (hf);
%! end_unwind_protect

## another figure, test hax
%!test
%! hf = figure ("visible", "off");
%! unwind_protect
%!   hax = subplot (1, 2, 1);
%!   sphere (hax);
%!   x = campos ();
%!   camorbit (20, 30)
%!   subplot (1, 2, 2);
%!   sphere ();
%!   camorbit (hax, -20, -30)
%!   y = campos (hax);
%!   assert (x, y, -eps);
%! unwind_protect_cleanup
%!   close (hf);
%! end_unwind_protect

## Test input validation
%!error <numeric scalars> camorbit ([1 2], [3 4])
%!error <Invalid call> camorbit (1, 2, "data", "z", 42)
%!error <DIR must be> camorbit (1, 2, "data", "meh")
%!error <DIR must be> camorbit (1, 2, "data", [1 2 3 4])
%!error <DIR must not be> camorbit (1, 2, "camera", "x")
%!error <COORSYS must be> camorbit (1, 2, "meh")
