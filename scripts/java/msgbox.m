## Copyright (C) 2010 Martin Hepperle
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
## @deftypefn  {Function File} {@var{h} =} msgbox (@var{msg})
## @deftypefnx {Function File} {@var{h} =} msgbox (@var{msg}, @var{title})
## @deftypefnx {Function File} {@var{h} =} msgbox (@var{msg}, @var{title}, @var{icon})
## Display @var{msg} using a message dialog box. 
##
## The message may have multiple lines separated by newline characters
## (@code{"\n"}), or it may be a cellstr array with one element for each
## line.  The optional input @var{title} (character string) can be used to
## decorate the dialog caption.
##
## The optional argument @var{icon} selects a dialog icon. 
## It can be one of @code{"none"} (default), @code{"error"}, @code{"help"}, or
## @code{"warn"}.
##
## The return value is always 1.
## @seealso{errordlg, helpdlg, inputdlg, listdlg, questdlg, warndlg}
## @end deftypefn

function h = msgbox (msg, title = "", icon)

  if (nargin < 1 || nargin > 3)
    print_usage ();
  endif

  if (! ischar (msg))
    if (iscell (msg))
      msg = sprintf ("%s\n", msg{:});
      msg(end) = "";
    else
      error ("msgbox: MSG must be a character string or cellstr array");
    endif
  endif

  if (! ischar (title))
    error ("msgbox: TITLE must be a character string");
  endif
  
  dlg = "emptydlg";
  if (nargin == 3)
    switch (icon)
      case "error"
        dlg = "errordlg";
      case "help"
        dlg = "helpdlg";
      case "warn"
        dlg = "warndlg";
      case "none"
        dlg = "emptydlg";
      otherwise
        error ("msgbox: ICON is not a valid type");
    endswitch
  endif

  h = javaMethod (dlg, "org.octave.JDialogBox", msg, title);

endfunction

