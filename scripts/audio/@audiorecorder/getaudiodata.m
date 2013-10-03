## Copyright (C) 2013 Vytautas Jančauskas
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
## @deftypefn{Function File} getaudiodata (@var{recorderObj})
## Returns recorder audio data as a Matrix with values between -1.0 and 1.0 and with as many columns as there are channels in the recorder.
## @end deftypefn
## @deftypefn{Function File} getaudiodata (@var{recorderObj}, @var{dataType})
## Converts recorded data to specified @var{dataType}. It can be set to 'double',
## 'single', 'int16', 'int8' or 'uint8'.
## @end deftypefn

function data = getaudiodata (varargin)
  if (nargin < 1 || nargin > 2)
    print_usage ();
  endif
  recorder = varargin{1};
  if (nargin == 1)
    data = __recorder_getaudiodata__ (struct (recorder).recorder);
  else
    data = __recorder_getaudiodata__ (struct (recorder).recorder);
    type = varargin{2};
    switch (type)
      case "int16"
        data = int16 (data * (2.0 ^ 15));
      case "int8"
        data = int8 (data * (2.0 ^ 7));
      case "uint8"
        data = uint8 ((data + 1.0) * 0.5 * (2.0 ^ 8 - 1));
    endswitch
  endif
  if get (recorder, "NumberOfChannels") == 2
    data = data';
  else
    data = data(1,:)';
  endif
endfunction
