## Copyright (C) 2004, 2005, 2006, 2007, 2009 David Bateman
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
## @deftypefn {Function File} {@var{c} =} bitget (@var{A},@var{n})
## Return the status of bit(s) @var{n} of unsigned integers in @var{A}
## the lowest significant bit is @var{n} = 1.
##
## @example
## @group
## bitget (100, 8:-1:1)
## @result{} 0  1  1  0  0  1  0  0 
## @end group
## @end example
## @seealso{bitand, bitor, bitxor, bitset, bitcmp, bitshift, bitmax}
## @end deftypefn

## Liberally based of the version by Kai Habel from octave-forge

function C = bitget (A, n)

  if (nargin != 2)
    print_usage ();
  endif

  if (isa (A, "double"))
    Amax = log2 (bitmax) + 1;
    _conv = @double;
  else
    if (isa (A, "uint8"))
      Amax = 8;
      _conv = @uint8;
    elseif (isa (A, "uint16"))
      Amax = 16;
      _conv = @uint16;
    elseif (isa (A, "uint32"))
      Amax = 32;
      _conv = @uint32;
    elseif (isa (A, "uint64"))
      Amax = 64;
      _conv = @uint64;
    elseif (isa (A, "int8"))
      Amax = 8;
      _conv = @int8;
    elseif (isa (A, "int16"))
      Amax = 16;
      _conv = @int16;
    elseif (isa (A, "int32"))
      Amax = 32;
      _conv = @int32;
    elseif (isa (A, "int64"))
      Amax = 64;
      _conv = @int64;
    else
      error ("bitget: invalid class %s", class (A));
    endif
  endif

  m = double (n(:));
  if (any (m < 1) || any (m > Amax))
    error ("bitget: n must be in the range [1,%d]", Amax);
  endif

  C = bitand (A, bitshift (_conv (1), uint8 (n) - uint8 (1))) != _conv (0);

endfunction
