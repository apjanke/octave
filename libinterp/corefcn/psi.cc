/*

Copyright (C) 2015 Carnë Draug

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ov.h"
#include "defun.h"
#include "error.h"
#include "dNDArray.h"
#include "fNDArray.h"

#include "lo-specfun.h"

DEFUN (psi, args, ,
"-*- texinfo -*-\n\
@deftypefn  {Function File} {} psi (@var{z})\n\
@deftypefnx {Function File} {} psi (@var{k}, @var{z})\n\
Compute the psi (polygamma) function.\n\
\n\
The polygamma functions are the @var{k}th derivative of the logarithm\n\
of the gamma function.  If unspecified, @var{k} defaults to zero.  A value\n\
of zero computes the digamma function, a value of 1, the trigamma function,\n\
and so on.\n\
\n\
The digamma function is defined:\n\
\n\
@tex\n\
$$\n\
\\Psi (z) = {d (log (\\Gamma (z))) \\over dx}\n\
$$\n\
@end tex\n\
@ifnottex\n\
@example\n\
@group\n\
psi (z) = d (log (gamma (z))) / dx\n\
@end group\n\
@end example\n\
@end ifnottex\n\
\n\
When computing the digamma function (when @var{k} equals zero), @var{z}\n\
can have any value real or complex value.  However, for polygamma functions\n\
(@var{k} higher than 0), @var{z} must be real and non-negative.\n\
\n\
@seealso{gamma, gammainc, gammaln}\n\
@end deftypefn")
{
  octave_value retval;

  const octave_idx_type nargin = args.length ();
  if (nargin < 1 || nargin > 2)
    {
      print_usage ();
      return retval;
    }

  const octave_value oct_z = (nargin == 1) ? args(0) : args(1);
  const octave_idx_type k = (nargin == 1) ? 0 : args(0).idx_type_value ();
  if (error_state || k < 0)
    {
      error ("psi: K must be a non-negative integer");
      return retval;
    }
  else if (k == 0)
    {
#define FLOAT_BRANCH(T, A, M, E) \
      if (oct_z.is_ ## T ##_type ()) \
        { \
          const A ## NDArray z = oct_z.M ## array_value (); \
          A ## NDArray psi_z (z.dims ()); \
\
          const E* zv = z.data (); \
          E* psi_zv = psi_z.fortran_vec (); \
          const octave_idx_type n = z.numel (); \
          for (octave_idx_type i = 0; i < n; i++) \
            *psi_zv++ = psi (*zv++); \
\
          retval = psi_z; \
        }

      if (oct_z.is_complex_type ())
        {
          FLOAT_BRANCH(double, Complex, complex_, Complex)
          else FLOAT_BRANCH(single, FloatComplex, float_complex_, FloatComplex)
          else
            {
              error ("psi: Z must be a floating point");
            }
        }
      else
        {
          FLOAT_BRANCH(double, , , double)
          else FLOAT_BRANCH(single, Float, float_, float)
          else
            {
              error ("psi: Z must be a floating point");
            }
        }

#undef FLOAT_BRANCH
    }
  else
    {
      if (! oct_z.is_real_type ())
        {
          error ("psi: Z must be real value for polygamma (K > 0)");
          return retval;
        }

#define FLOAT_BRANCH(T, A, M, E) \
      if (oct_z.is_ ## T ##_type ()) \
        { \
          const A ## NDArray z = oct_z.M ## array_value (); \
          A ## NDArray psi_z (z.dims ()); \
\
          const E* zv = z.data (); \
          E* psi_zv = psi_z.fortran_vec (); \
          const octave_idx_type n = z.numel (); \
          for (octave_idx_type i = 0; i < n; i++) \
            { \
              if (*zv < 0) \
                { \
                  error ("psi: Z must be non-negative for polygamma (K > 0)"); \
                  return retval; \
                } \
              *psi_zv++ = psi (k, *zv++); \
            } \
          retval = psi_z; \
        }

      FLOAT_BRANCH(double, , , double)
      else FLOAT_BRANCH(single, Float, float_, float)
      else
        {
          error ("psi: Z must be a floating point for polygamma (K > 0)");
        }

#undef FLOAT_BRANCH
    }

  return retval;
}

/*
%!shared em
%! em = 0.577215664901532860606512090082402431042; # Euler-Mascheroni Constant

%!assert (psi (ones (7, 3, 5)), repmat (-em, [7 3 5]))
%!assert (psi ([0 1]), [-Inf -em])
%!assert (psi ([-20:1]), [repmat(-Inf, [1 21]) -em])
%!assert (psi (single ([0 1])), single ([-Inf -em]))

## Abramowitz and Stegun, page 258, eq 6.3.5
%!test
%! z = [-10:.1:-.1 .1:.1:20]; # drop the 0
%! assert (psi (z + 1), psi (z) + 1 ./ z, eps*1000)

## Abramowitz and Stegun, page 258, eq 6.3.2
%!assert (psi (1), -em)

## Abramowitz and Stegun, page 258, eq 6.3.3
%!assert (psi (1/2), -em - 2 * log (2))

## The following tests are from Pascal Sebah and Xavier Gourdon (2002)
## "Introduction to the Gamma Function"

## Interesting identities of the digamma function, in section of 5.1.3
%!assert (psi (1/3), - em - (3/2) * log(3) - ((sqrt (3) / 6) * pi), eps*10)
%!assert (psi (1/4), - em -3 * log (2) - pi /2)
%!assert (psi (1/6), - em -2 * log (2) - (3/2) * log (3) - ((sqrt (3) / 2) * pi), eps*10)

## First 6 zeros of the digamma function, in section of 5.1.5 (and also on
## Abramowitz and Stegun, page 258, eq 6.3.19)
%!assert (psi ( 1.46163214496836234126265954232572132846819620400644), 0, eps)
%!assert (psi (-0.504083008264455409258269304533302498955385182368579), 0, eps)
%!assert (psi (-1.573498473162390458778286043690434612655040859116846), 0, eps)
%!assert (psi (-2.610720868444144650001537715718724207951074010873480), 0, eps*10)
%!assert (psi (-3.635293366436901097839181566946017713948423861193530), 0, eps*10)
%!assert (psi (-4.653237761743142441714598151148207363719069416133868), 0, eps*100)

## Tests for complex values
%!shared z
%! z = [-10:.1:-.1 .1:.1:20]; # drop the 0

## Abramowitz and Stegun, page 259 eq 6.3.10
%!assert (real (psi (i*z)), real (psi (1 - i*z)))

## Abramowitz and Stegun, page 259 eq 6.3.11
%!assert (imag (psi (i*z)), 1/2 .* 1./z + 1/2 * pi * coth (pi * z), eps *10)

## Abramowitz and Stegun, page 259 eq 6.3.12
%!assert (imag (psi (1/2 + i*z)), 1/2 * pi * tanh (pi * z), eps)

## Abramowitz and Stegun, page 259 eq 6.3.13
%!assert (imag (psi (1 + i*z)), - 1./(2*z) + 1/2 * pi * coth (pi * z), eps*10)

## Abramowitz and Stegun, page 260 eq 6.4.5
%!test
%! for z = 0:20
%!   assert (psi (1, z + 0.5), 0.5 * (pi^2) - 4 * sum ((2*(1:z) -1) .^(-2)), eps*10)
%! endfor

## Abramowitz and Stegun, page 260 eq 6.4.6
%!test
%! z = 0.1:0.1:20;
%! for n = 0:8
%!   ## our precision goes down really quick when computing n is too high,
%!   assert (psi (n, z+1), psi (n, z) + ((-1)^n) * factorial (n) * (z.^(-n-1)), 0.1)
%! endfor

## Test input validation
%!error psi ()
%!error psi (1, 2, 3)
%!error <Z must be> psi ("non numeric")
%!error <K must be a non-negative integer> psi (-5, 1)
%!error <Z must be non-negative for polygamma> psi (5, -1)
%!error <Z must be a floating point> psi (5, uint8 (-1))
%!error <Z must be real value for polygamma> psi (5, 5i)

*/
