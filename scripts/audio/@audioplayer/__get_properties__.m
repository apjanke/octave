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
## @deftypefn {Function File} {@var{properties} =} __get_properties__ (@var{player})
## Return a struct containing all named properties of the audioplayer
## object @var{player}.
## @end deftypefn

function properties = __get_properties__ (player)
  properties.BitsPerSample = __player_get_nbits__ (struct (player).player);
  properties.CurrentSample = __player_get_sample_number__ (struct (player).player);
  properties.DeviceID = __player_get_id__ (struct (player).player);
  properties.NumberOfChannels = __player_get_channels__ (struct (player).player);
  if __player_isplaying__ (struct (player).player)
    running = "on";
  else
    running = "off";
  endif
  properties.Running = running;
  properties.SampleRate = __player_get_fs__ (struct (player).player);
  properties.TotalSamples = __player_get_total_samples__ (struct (player).player);
  properties.Tag = __player_get_tag__ (struct (player).player);
  properties.Type = "audioplayer";
  properties.UserData = __player_get_userdata__ (struct (player).player);
endfunction
