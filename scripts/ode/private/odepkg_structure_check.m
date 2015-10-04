%# Copyright (C) 2006-2012, Thomas Treichl <treichl@users.sourceforge.net>
%# Copyright (C) 2013, Roberto Porcu' <roberto.porcu@polimi.it>
%# OdePkg - A package for solving ordinary differential equations and more
%#
%# This program is free software; you can redistribute it and/or modify
%# it under the terms of the GNU General Public License as published by
%# the Free Software Foundation; either version 2 of the License, or
%# (at your option) any later version.
%#
%# This program is distributed in the hope that it will be useful,
%# but WITHOUT ANY WARRANTY; without even the implied warranty of
%# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%# GNU General Public License for more details.
%#
%# You should have received a copy of the GNU General Public License
%# along with this program; If not, see <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {Function File} {[@var{newstruct}] =} odepkg_structure_check (@var{oldstruct}, [@var{"solver"}])
##
## If this function is called with one input argument of type structure array
## then check the field names and the field values of the OdePkg structure
## @var{oldstruct} and return the structure as @var{newstruct} if no error is
## found.
##
## Optionally if this function is called with a second input argument
## @var{"solver"} of type string taht specifies the name of a valid OdePkg
## solver then a higher level error detection is performed.  The function
## does not modify any of the field names or field values but terminates with
## an error if an invalid option or value is found.
##
## This function is an OdePkg internal helper function therefore it should
## never be necessary that this function is called directly by a user.  There
## is only little error detection implemented in this function file to
## achieve the highest performance.
##
## Run examples with the command
##
## @example
## demo odepkg_structure_check
## @end example
## @end deftypefn
##
## @seealso{odepkg}

function [vret] = odepkg_structure_check (varargin)

  %# Check the number of input arguments
  if (nargin == 0)
    help ('odepkg_structure_check');
    error ('OdePkg:InvalidArgument', ...
      'Number of input arguments must be greater than zero');
  elseif (nargin > 2)
    print_usage;
  elseif (nargin == 1 && isstruct (varargin{1}))
    vret = varargin{1};
    vsol = '';
    vfld = fieldnames (vret);
    vlen = length (vfld);
  elseif (nargin == 2 && isstruct (varargin{1}) && ischar (varargin{2}))
    vret = varargin{1};
    vsol = varargin{2};
    vfld = fieldnames (vret);
    vlen = length (vfld);
  end

  for vcntarg = 1:vlen %# Run through the number of given structure field names

    switch (vfld{vcntarg})

      case 'RelTol'
        if (isempty   (vret.(vfld{vcntarg})) || ...
           (isnumeric (vret.(vfld{vcntarg})) && ...
           isreal    (vret.(vfld{vcntarg})) && ...
           all       (vret.(vfld{vcntarg}) > 0))) %# 'all' is a MatLab need
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

        switch (vsol)
          case {'ode23', 'ode45', 'ode54', 'ode78', ...
                'ode23d', 'ode45d', 'ode54d', 'ode78d',}
            if (~isscalar (vret.(vfld{vcntarg})) && ...
                ~isempty (vret.(vfld{vcntarg})))
              error ('OdePkg:InvalidParameter', ...
                'Value of option "RelTol" must be a scalar');
            end
          otherwise

          end

      case 'AbsTol'
        if (isempty   (vret.(vfld{vcntarg})) || ...
           (isnumeric (vret.(vfld{vcntarg})) && ...
            isreal    (vret.(vfld{vcntarg})) && ...
            all       (vret.(vfld{vcntarg}) > 0)))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'NormControl'
        if (isempty   (vret.(vfld{vcntarg})) || ...
           (strcmp (vret.(vfld{vcntarg}), 'on') || ...
            strcmp (vret.(vfld{vcntarg}), 'off')))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'NonNegative'
        if (isempty  (vret.(vfld{vcntarg})) || ...
            (isnumeric (vret.(vfld{vcntarg})) && isvector (vret.(vfld{vcntarg}))))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'OutputFcn'
        if (isempty (vret.(vfld{vcntarg})) || ...
            isa     (vret.(vfld{vcntarg}), 'function_handle'))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'OutputSel'
        if (isempty  (vret.(vfld{vcntarg})) || ...
            (isnumeric (vret.(vfld{vcntarg})) && isvector (vret.(vfld{vcntarg}))) || ...
            isscalar (vret.(vfld{vcntarg})))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'OutputSave'
        if (isempty (vret.(vfld{vcntarg})) || ...
            (isscalar (vret.(vfld{vcntarg})) && ...
             mod (vret.(vfld{vcntarg}), 1) == 0 && ...
             vret.(vfld{vcntarg}) > 0) || ...
            vret.(vfld{vcntarg}) == Inf)
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end
        
      case 'Refine'
        if (isempty   (vret.(vfld{vcntarg})) || ...
           (isscalar (vret.(vfld{vcntarg})) && ...
            mod (vret.(vfld{vcntarg}), 1) == 0 && ...
            vret.(vfld{vcntarg}) >= 0 && ...
            vret.(vfld{vcntarg}) <= 5))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'Stats'
        if (isempty   (vret.(vfld{vcntarg})) || ...
           (strcmp (vret.(vfld{vcntarg}), 'on') || ...
            strcmp (vret.(vfld{vcntarg}), 'off')))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'InitialStep'
        if (isempty (vret.(vfld{vcntarg})) || ...
            (isscalar (vret.(vfld{vcntarg})) && ...
             isreal (vret.(vfld{vcntarg}))))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'MaxStep'
        if (isempty (vret.(vfld{vcntarg})) || ...
            (isscalar (vret.(vfld{vcntarg})) && ...
             vret.(vfld{vcntarg}) > 0) )
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'Events'
        if (isempty (vret.(vfld{vcntarg})) || ...
            isa     (vret.(vfld{vcntarg}), 'function_handle'))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'Jacobian'
        if (isempty (vret.(vfld{vcntarg})) || ...
            isnumeric (vret.(vfld{vcntarg})) || ...
            isa (vret.(vfld{vcntarg}), 'function_handle') || ...
            iscell (vret.(vfld{vcntarg})))
        else
          error ('OdePkg:InvalidParameter', ...
                 'Unknown parameter name "%s" or no valid parameter value', ...
                 vfld{vcntarg});
        end

      case 'JPattern'
        if (isempty (vret.(vfld{vcntarg})) || ...
            isvector (vret.(vfld{vcntarg})) || ...
            isnumeric (vret.(vfld{vcntarg})))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'Vectorized'
        if (isempty   (vret.(vfld{vcntarg})) || ...
           (strcmp (vret.(vfld{vcntarg}), 'on') || ...
            strcmp (vret.(vfld{vcntarg}), 'off')))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'Mass'
        if (isempty (vret.(vfld{vcntarg})) || ...
            (isnumeric (vret.(vfld{vcntarg})) || ...
            isa (vret.(vfld{vcntarg}), 'function_handle')))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'MStateDependence'
        if (isempty   (vret.(vfld{vcntarg})) || ...
           (strcmp (vret.(vfld{vcntarg}), 'none') || ...
            strcmp (vret.(vfld{vcntarg}), 'weak') || ...
            strcmp (vret.(vfld{vcntarg}), 'strong')))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'MvPattern'
        if (isempty (vret.(vfld{vcntarg})) || ...
            (isvector (vret.(vfld{vcntarg})) || ...
            isnumeric (vret.(vfld{vcntarg}))))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'MassSingular'
        if (isempty (vret.(vfld{vcntarg})) || ...
            (strcmp (vret.(vfld{vcntarg}), 'yes') || ...
            strcmp (vret.(vfld{vcntarg}), 'no') || ...
            strcmp (vret.(vfld{vcntarg}), 'maybe')))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'InitialSlope'
        if (isempty (vret.(vfld{vcntarg})) || ...
            isvector (vret.(vfld{vcntarg})))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'MaxOrder'
        if (isempty (vret.(vfld{vcntarg})) || ...
            (mod (vret.(vfld{vcntarg}), 1) == 0 && ...
             vret.(vfld{vcntarg}) > 0 && ...
             vret.(vfld{vcntarg}) < 8))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'BDF'
        if (isempty (vret.(vfld{vcntarg})) || ...
            (strcmp (vret.(vfld{vcntarg}), 'on') || ...
             strcmp (vret.(vfld{vcntarg}), 'off')))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'NewtonTol'
        if (isempty   (vret.(vfld{vcntarg})) || ...
           (isnumeric (vret.(vfld{vcntarg})) && ...
            isreal    (vret.(vfld{vcntarg})) && ...
            all       (vret.(vfld{vcntarg}) > 0))) %# 'all' is a MatLab need
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'MaxNewtonIterations'
        if (isempty (vret.(vfld{vcntarg})) || ...
            (mod (vret.(vfld{vcntarg}), 1) == 0 && ...
             vret.(vfld{vcntarg}) > 0))
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      %# new fields added
      case 'Algorithm'
        if( isempty(vret.(vfld{vcntarg})) || ischar(vret.(vfld{vcntarg})) )
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'Choice'
        if( isempty(vret.(vfld{vcntarg})) || (isnumeric(vret.(vfld{vcntarg})) && (vret.(vfld{vcntarg})==1) || vret.(vfld{vcntarg})==2 ) )
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'Eta'
        if( isempty(vret.(vfld{vcntarg})) || ( isreal(vret.(vfld{vcntarg})) && vret.(vfld{vcntarg})>=0 && vret.(vfld{vcntarg})<1) )
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'Explicit'
        if( isempty(vret.(vfld{vcntarg})) || (ischar(vret.(vfld{vcntarg})) && (strcmp(vret.(vfld{vcntarg}),'yes') || strcmp(vret.(vfld{vcntarg}),'no'))) )
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'InexactSolver'
        if( isempty(vret.(vfld{vcntarg})) || ischar(vret.(vfld{vcntarg})) )
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'InitialSlope'
        if( isempty(vret.(vfld{vcntarg})) || ( ischar(vret.(vfld{vcntarg})) || (isnumeric(vret.(vfld{vcntarg})) && (isvector(vret.(vfld{vcntarg})) || isreal(vret.(vfld{vcntarg}))))) )
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'JConstant'
        if( isempty(vret.(vfld{vcntarg})) || (ischar(vret.(vfld{vcntarg})) && (strcmp(vret.(vfld{vcntarg}),'yes') || strcmp(vret.(vfld{vcntarg}),'no'))) )
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'MassConstant'
        if( isempty(vret.(vfld{vcntarg})) || (strcmp(vret.(vfld{vcntarg}),'on') || strcmp(vret.(vfld{vcntarg}),'off')) )
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'PolynomialDegree'
        if( isempty(vret.(vfld{vcntarg})) || (isnumeric(vret.(vfld{vcntarg})) && mod(vret.(vfld{vcntarg}),1)==0 && vret.(vfld{vcntarg})>0) )
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'QuadratureOrder'
        if( isempty(vret.(vfld{vcntarg})) || (isnumeric(vret.(vfld{vcntarg})) && mod(vret.(vfld{vcntarg}),1)==0 && vret.(vfld{vcntarg})>0) )
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'Restart'
        if( isempty(vret.(vfld{vcntarg})) || (isnumeric(vret.(vfld{vcntarg})) && mod(vret.(vfld{vcntarg}),1)==0 && vret.(vfld{vcntarg})>0) )
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'TimeStepNumber'
        if( isempty(vret.(vfld{vcntarg})) || (isnumeric(vret.(vfld{vcntarg})) && mod(vret.(vfld{vcntarg}),1)==0 && vret.(vfld{vcntarg})>0) )
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'TimeStepSize'
        if( isempty(vret.(vfld{vcntarg})) || ( isreal(vret.(vfld{vcntarg})) && vret.(vfld{vcntarg})!=0) )
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      case 'UseJacobian'
        if( isempty(vret.(vfld{vcntarg})) || (ischar(vret.(vfld{vcntarg})) && (strcmp(vret.(vfld{vcntarg}),'yes') || strcmp(vret.(vfld{vcntarg}),'no'))) )
        else
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s" or no valid parameter value', ...
            vfld{vcntarg});
        end

      otherwise
          error ('OdePkg:InvalidParameter', ...
            'Unknown parameter name "%s"', ...
            vfld{vcntarg});

    end %# switch

  end %# for

%!demo
%! # Return the checked OdePkg options structure that is created by
%! # the command odeset.
%!
%! odepkg_structure_check (odeset);
%!
%!demo
%! # Create the OdePkg options structure A with odeset and check it 
%! # with odepkg_structure_check. This actually is unnecessary
%! # because odeset automtically calls odepkg_structure_check before
%! # returning.
%!
%! A = odeset (); odepkg_structure_check (A);

%# Local Variables: ***
%# mode: octave ***
%# End: ***
