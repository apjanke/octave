## Copyright (C) 2005, 2006, 2007 Michael Zeising
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
## @deftypefn {Function File} {@var{y} =} wavread (@var{filename})
## Load the RIFF/WAVE sound file @var{filename}, and return the samples
## in vector @var{y}.  If the file contains multichannel data, then
## @var{y} is a matrix with the channels represented as columns.
##
## @deftypefnx {Function File} {[@var{y}, @var{Fs}, @var{bits}] =} wavread (@var{filename})
## Additionally return the sample rate (@var{fs}) in Hz and the number of bits 
## per sample (@var{bits}).
##
## @deftypefnx {Function File} {[@dots{}] =} wavread (@var{filename}, @var{n})
## Read only the first @var{n} samples from each channel.
##
## @deftypefnx {Function File} {[@dots{}] =} wavread (@var{filename},[@var{n1} @var{n2}])
## Read only samples @var{n1} through @var{n2} from each channel.
##
## @deftypefnx {Function File} {[@var{samples}, @var{channels}] =} wavread (@var{filename}, "size")
## Return the number of samples (@var{n}) and channels (@var{ch})
## instead of the audio data.
## @seealso{wavwrite}
## @end deftypefn

## Author: Michael Zeising <michael@michaels-website.de>
## Created: 06 December 2005

function [y, samples_per_sec, bits_per_sample] = wavread (filename, param)

  FORMAT_PCM        = 0x0001;   # PCM (8/16/32 bit)
  FORMAT_IEEE_FLOAT = 0x0003;   # IEEE float (32/64 bit)
  BYTEORDER         = "ieee-le";

  if (nargin < 1 || nargin > 2)
    print_usage ();
  endif

  if (! ischar (filename))
    error ("wavwrite: expecting filename to be a character string");
  endif

  # open file for binary reading
  [fid, msg] = fopen (filename, "rb");
  if (fid < 0)
    error ("wavread: %s", msg);
  endif
  
  ## Check for RIFF/WAVE header.
  ck_id = char (fread (fid, 4))';
  fseek (fid, 4, SEEK_CUR);
  wave_id = char (fread (fid, 4))';
  if (ck_id != "RIFF" || wave_id != "WAVE")
    fclose (fid);
    error ("wavread: file contains no RIFF/WAVE signature");
  endif
  
  ## Find format chunk within the next 256 (4*64) bytes.
  i = 1;
  while (true)
    if (char (fread (fid, 4))' == "fmt ");
      break;
    endif
    if (i++ == 64)
      fclose (fid);
      error ("wavread: file contains no format chunk");
    endif
  endwhile

  ## Format chunk size.
  ck_size = fread (fid, 1, "uint32", 0, BYTEORDER);         
  
  ## Sample format code.
  format_tag = fread (fid, 1, "uint16", 0, BYTEORDER);
  if (format_tag != FORMAT_PCM && format_tag != FORMAT_IEEE_FLOAT)
    fclose (fid);
    error ("wavread: sample format %#x is not supported", format_tag);
  endif

  ## Number of interleaved channels.
  channels = fread (fid, 1, "uint16", 0, BYTEORDER);

  ## Sample rate.
  samples_per_sec = fread (fid, 1, "uint32", 0, BYTEORDER);

  ## Bits per sample.
  fseek (fid, 6, SEEK_CUR);
  bits_per_sample = fread (fid, 1, "uint16", 0, BYTEORDER);

  ## Ignore the rest of the chunk.
  fseek (fid, ck_size-16, SEEK_CUR);
  
  ## Find data chunk.
  i = 1;
  while (true)
    if (char (fread (fid, 4))' == "data")
      break;
    endif
    if (i++ == 64)
      fclose (fid);
      error ("wavread: file contains no data chunk");
    endif
  endwhile

  ## Data chunk size.
  ck_size = fread (fid, 1, "uint32", 0, BYTEORDER);
  
  ## Determine sample data type.
  if (format_tag == FORMAT_PCM)
    switch (bits_per_sample)
      case 8
        format = "uint8";
      case 16 
        format = "int16";
      case 24
	format = "uint8";
      case 32 
        format = "int32";
      otherwise
        fclose (fid);
        error ("wavread: %d bits sample resolution is not supported with PCM",
	       bits_per_sample);
    endswitch
  else
    switch (bits_per_sample)
      case 32 
        format = "float32";
      case 64 
        format = "float64";
      otherwise
        fclose (fid);
        error ("wavread: %d bits sample resolution is not supported with IEEE float",
	       bits_per_sample);
    endswitch
  endif
  
  ## Parse arguments.
  if (nargin == 1)
    length = 8 * ck_size / bits_per_sample;
  else
    if (size (param, 2) == 1)
      ## Number of samples is given.
      length = param * channels;
    elseif (size (param, 2) == 2)
      ## Sample range is given.
      if (fseek (fid, (param(1)-1) * channels * (bits_per_sample/8), SEEK_CUR) < 0)
        warning ("wavread: seeking failed");
      endif
      length = (param(2)-param(1)+1) * channels;
    elseif (size (param, 2) == 4 && char (param) == "size")
      ## Size of the file is requested.
      fclose (fid);
      y = [ck_size/channels/(bits_per_sample/8), channels];
      return
    else
      fclose (fid);
      error ("wavread: invalid argument 2");
    endif
  endif

  ## Read samples and close file.
  if (bits_per_sample == 24)
    length *= 3;
  endif
  [yi, n] = fread (fid, length, format, 0, BYTEORDER);
  fclose (fid);

  ## Check data.
  if (mod (numel (yi), channels) != 0)
    error ("wavread: data in %s doesn't match the number of channels",
	   filename);
  endif

  if (bits_per_sample == 24)
    yi = reshape (yi, 3, rows(yi)/3)';
    yi(yi(:,3) >= 128, 3) -= 256;
    yi = yi * [1; 256; 65536];
  endif

  if (format_tag == FORMAT_PCM)
    ## Normalize samples.
    switch (bits_per_sample)
      case 8
        yi = (yi - 128)/127;
      case 16
        yi /= 32767;
      case 24
		yi /= 8388607;
      case 32
        yi /= 2147483647;
    endswitch
  endif
  
  ## Deinterleave.
  nr = numel (yi) / channels;
  y = reshape (yi, channels, nr)';
  
endfunction
