## Copyright (C) 1996, 1997 John W. Eaton
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, write to the Free
## Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
## 02110-1301, USA.

## -*- texinfo -*-
## @deftypefn {Function File} {} __plt__ (@code{caller}, @dots{})
## @end deftypefn

## Author: jwe

function __plt__ (caller, varargin)

  __plot_globals__;

  cf = __current_figure__;
  mxi = __multiplot_xi__(cf);
  myi = __multiplot_yi__(cf);

  __setup_plot__ ("plot");

  nargs = nargin ();

  if (nargs > 1)

    k = 1;
    j = __plot_data_offset__{cf}(mxi,myi);

    x_set = false;
    y_set = false;

    ## Gather arguments, decode format, gather plot strings, and plot lines.

    while (--nargs > 0 || x_set)

      if (nargs == 0)
	## Force the last plot when input variables run out.
	next_arg = {""};
      else
	next_arg = varargin{k++};
      endif

      if (ischar (next_arg) || iscellstr (next_arg))
	if (x_set)
	  [fmt, keystr] = __pltopt__ (caller, next_arg);
	  if (y_set)
	    [tdata, tfmtstr, key] = __plt2__ (x, y, fmt, keystr);
	  else
	    [tdata, tfmtstr, key] = __plt1__ (x, fmt, keystr);
	  endif
	  if (! isempty (tdata))
	    for i = 1:numel (tdata)
	      __plot_usingstr__{cf}{mxi,myi}{j}{i} ...
		  = __make_using_clause__ (tdata{i});
	      __plot_withstr__{cf}{mxi,myi}{j}{i} = "";
	    endfor
	    __plot_data__{cf}{mxi,myi}{j} = tdata;
	    __plot_data_type__{cf}{mxi,myi}(j) = 2;
	    __plot_fmtstr__{cf}{mxi,myi}{j} = tfmtstr;
	    __plot_key_labels__{cf}{mxi,myi}{j} = key;
	    j++;
	  endif
	  x_set = false;
	  y_set = false;
	else
	  error ("plot: no data to plot");
	endif
      elseif (x_set)
	if (y_set)
	  [fmt, keystr] = __pltopt__ (caller, {""});
	  [tdata, tfmtstr, key] = __plt2__ (x, y, fmt, keystr);
	  if (! isempty (tdata))
	    for i = 1:numel (tdata)
	      __plot_usingstr__{cf}{mxi,myi}{j}{i} ...
		  = __make_using_clause__ (tdata{i});
	      __plot_withstr__{cf}{mxi,myi}{j}{i} = "";
	    endfor
	    __plot_data__{cf}{mxi,myi}{j} = tdata;
	    __plot_data_type__{cf}{mxi,myi}(j) = 2;
	    __plot_fmtstr__{cf}{mxi,myi}{j} = tfmtstr;
	    __plot_key_labels__{cf}{mxi,myi}{j} = key;
	    j++;
	  endif
	  x = next_arg;
	  y_set = false;
	else
	  y = next_arg;
	  y_set = true;
	endif
      else
	x = next_arg;
	x_set = true;
      endif

    endwhile

    __plot_data_offset__{cf}(mxi,myi) = j;

    __render_plot__ ();

  else
    msg = sprintf ("%s (y)\n", caller);
    msg = sprintf ("%s       %s (x, y, ...)\n", msg, caller);
    msg = sprintf ("%s       %s (x, y, fmt, ...)", msg, caller);
    usage (msg);
  endif

endfunction
