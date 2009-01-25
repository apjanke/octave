## Copyright (C) 2009 S�ren Hauberg
##
## This program is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or (at
## your option) any later version.
##
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {Function File} {} print_usage ()
## @deftypefnx{Function File} {} print_usage (@var{name})
## Print the usage message for a function.  When called with no input arguments
## the @code{print_usage} function displays the usage message of the currently
## executing function.
## @seealso{help}
## @end deftypefn

function print_usage (name)
  ## Handle input
  if (nargin == 0)
    ## Determine the name of the calling function
    x = dbstack ();
    if (numel (x) > 1)
      name = x (2).name;
    else
      error ("print_usage: invalid function\n");
    endif
  elseif (!ischar (name))
    error ("print_usage: input argument must be a string");
  endif
  
  ## Do the actual work
  [text, format] = get_help_text (name);
  max_len = 80;
  switch (lower (format))
    case "plain text"
      [usage_string, status] = get_usage_plain_text (text, max_len);
    case "texinfo"
      [usage_string, status] = get_usage_texinfo (text, max_len);
    case "html"
      [usage_string, status] = get_usage_html (text, max_len);
    case "not documented"
      error ("print_usage: `%s' is not documented\n", name);
    case "not found"
      error ("print_usage: `%s' not found\n", name);
    otherwise
      error ("print_usage: internal error: unsupported help text format: '%s'\n", format);
  endswitch
  
  ## Raise the final error
  if (status != 0)
    warning ("makeinfo: Texinfo formatting filter exited abnormally");
    warning ("makeinfo: raw Texinfo source of help text follows...\n");
  endif

  error ("Invalid call to %s.  Correct usage is:\n\n%s\n%s",
	 name, usage_string, __additional_help_message__ ());
endfunction

function [retval, status] = get_usage_plain_text (help_text, max_len)
  ## Extract first line by searching for a double line-end.
  line_end_idx = strfind (help_text, "\n\n");
  retval = help_text (1:min ([line_end_idx , max_len, length(help_text)]));
  status = 0;
endfunction

function [retval, status] = get_usage_texinfo (help_text, max_len)
  ## Lines ending with "@\n" are continuation lines, so they should be
  ## concatenated with the following line.
  help_text = strrep (help_text, "@\n", " ");
  
  ## Find, and keep, lines that start with @def or @end def. This should include things
  ## such as @deftypefn, @deftypefnx, @defvar, etc. and their corresponding @end's
  def_idx = strfind (help_text, "@def");
  if (!isempty (def_idx))
    buffer = "";
    endl_idx = find (help_text == "\n");
    for k = 1:length (def_idx)
      endl = endl_idx (find (endl_idx > def_idx (k), 1));
      if (isempty (endl))
        buffer = strcat (buffer, help_text (def_idx (k):end), "\n");
      else
        buffer = strcat (buffer, help_text (def_idx (k):endl));
      endif
    endfor
    
    end_def_idx = strfind (help_text, "@end def");
    if (!isempty (end_def_idx))
      buffer = strcat (buffer, help_text (end_def_idx:end));
    endif
  else
    [retval, status] = get_usage_plain_text (help_text, max_len);
  endif

  ## Run makeinfo to generate plain text
  [retval, status] = makeinfo (buffer, "plain text");
endfunction

function [retval, status] = get_usage_html (help_text, max_len)
  ## Strip tags
  [help_text, status] = strip_html_tags (help_text);
  
  ## Extract first line with plain text method.
  retval = get_usage_plain_text (help_text, max_len);
endfunction

