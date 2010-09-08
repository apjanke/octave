## Copyright (C) 2010 Shai Ayal
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
## @deftypefn {Function File} {} __fltk_print__ (@var{@dots{}})
## Undocumented internal function.
## @end deftypefn

function opts = __fltk_print__ (opts)

  figure (opts.figure)
  drawnow ("expose")

  if (! isempty (opts.fig2dev_binary))
    fig2dev_devices = {"pstex", "mf", "emf"};
  else
    ## If no fig2dev is present, support emf using pstoedit.
    fig2dev_devices = {"pstex", "mf"};
  endif

  switch lower (opts.devopt)
  case {"eps", "eps2", "epsc", "epsc2"}
    drawnow ("eps", opts.name);
    if (opts.tight_flag)
      __tight_eps_bbox__ (opts, opts.name);
    endif
  case {"epslatex", "pslatex", "pdflatex", "epslatexstandalone", ...
        "pslatexstandalone", "pdflatexstandalone"}
    ## format GL2PS_TEX
    n = find (opts.devopt == "l", 1);
    suffix = opts.devopt(1:n-1);
    dot = find (opts.name == ".", 1, "last");
    if ((! isempty (dot))
        && any (strcmpi (opts.name(dot:end), ...
                {".eps", ".ps", ".pdf", ".tex", "."})))
      name = opts.name(1:dot-1);
      if (dot < numel (opts.name)
          && any (strcmpi (opts.name(dot+1:end), {"eps", "ps", "pdf"})))
        ## If user provides eps/ps/pdf suffix, use it.
        suffix = opts.name(dot+1:end);
      endif
    elseif (dot == numel (opts.name))
      name = opts.name;
    endif
    drawnow (strcat (lower (suffix), "notxt"), strcat (name, ".", suffix));
    drawnow ("tex", strcat (name, ".tex"));
    if (opts.tight_flag && strcmp (suffix, "eps"))
      __tight_eps_bbox__ (opts, strcat (name, ".", suffix));
    endif
    if (! isempty (strfind (opts.devopt, "standalone")))
      __latex_standalone__ (strcat (name, ".tex"));
    endif
  case {"tikz"}
    ## format GL2PS_PGF
    drawnow ("pgf", opts.name);
  case {"svg"}
    ## format GL2PS_SVG
    drawnow ("svg", opts.name);
  case fig2dev_devices
    tmp_figfile = strcat (tmpnam (), ".fig");
    opts.unlink{end+1} = tmp_figfile;
    status = __pstoedit__ (opts, "fig", tmp_figfile);
    if (status == 0)
      status = __fig2dev__ (opts, tmp_figfile);
      if (strcmp (opts.devopt, "pstex") && status == 0)
        if (strfind (opts.name, ".ps") == numel(opts.name) - 2)
          devfile = strcat (opts.name(1:end-2), "tex");
        else
          devfile = strcat (opts.name, ".tex");
        endif
        status = __fig2dev__ (opts, tmp_figfile, "pstex_t", devfile);
      endif
    endif
  case {"aifm"}
    status = __pstoedit__ (opts, "ps2ai");
  case {"dxf", "emf", "fig", "hpgl"}
    status = __pstoedit__ (opts);
  case {"corel", "gif"}
    error ("print:unsupporteddevice",
           "print.m: %s output is not available for the FLTK backend.",
           upper (opts.devopt))
  case opts.ghostscript.device
    drawnow ("eps", opts.ghostscript.source);
  endswitch

endfunction

function status = __fig2dev__ (opts, figfile, devopt, devfile)
  persistent warn_on_absence = true
  if (nargin < 4)
    devfile = opts.name;
  endif
  if (nargin < 3)
    devopt =  opts.devopt;
  endif
  if (! isempty (opts.fig2dev_binary))
    cmd = sprintf ("%s -L %s %s %s 2>&1", opts.fig2dev_binary, devopt,
                   figfile, devfile);
    [status, output] = system (cmd);
    if (opts.debug || status != 0)
      fprintf ("fig2dev command: %s", cmd)
    endif
    if (status != 0)
      disp (output)
      warning ("print:fig2devfailed", "print.m: error running fig2dev.")
    endif
  elseif (isempty (opts.fig2dev_binary) && warn_on_absence)
    warning ("print:nofig2dev", "print.m: 'fig2dev' not found in EXEC_PATH.")
    warn_on_absence = false;
  endif
endfunction

function status = __pstoedit__ (opts, devopt, name)
  persistent warn_on_absence = true
  if (nargin < 3)
    name = opts.name;
  endif
  if (nargin < 2)
    devopt =  opts.devopt;
  endif
  if (! isempty (opts.pstoedit_binary))
    tmp_epsfile = strcat (tmpnam (), ".eps");
    unwind_protect
      drawnow ("eps", tmp_epsfile)
      if (opts.tight_flag)
        __tight_eps_bbox__ (opts, tmp_epsfile);
      endif
      cmd = sprintf ("%s -f %s %s %s 2>&1", opts.pstoedit_binary, devopt,
                     tmp_epsfile, name);
      [status, output] = system (cmd);
      if (opts.debug || status != 0)
        fprintf ("pstoedit command: %s", cmd)
      endif
      if (status != 0)
        disp (output)
        warning ("print:pstoeditfailed", "print.m: error running pstoedit.")
      endif
    unwind_protect_cleanup
      [status, output] = unlink (tmp_epsfile);
      if (status != 0)
        warning ("print.m: %s, '%s'.", output, tmp_epsfile)
      endif
    end_unwind_protect
  elseif (isempty (opts.pstoedit_binary) && warn_on_absence)
    warning ("print:nopstoedit", "print.m: 'pstoedit' not found in EXEC_PATH.")
    warn_on_absence = false;
  endif
endfunction

function __latex_standalone__ (latexfile)
  prepend = {"\\documentclass{minimal}";
             "\\usepackage{epsfig,color}";
             "\\begin{document}";
             "\\centering"};
  postpend = {"\\end{document}"};
  fid = fopen (latexfile, "r");
  if (fid >= 0)
    latex = fscanf (fid, "%c", Inf);
    status = fclose (fid);
    if (status != 0)
      error ("print:errorclosingfile",
             "print.m: error closing file '%s'", latexfile)
    endif
  else
    error ("print:erroropeningfile",
           "print.m: error opening file '%s'", latexfile)
  endif
  fid = fopen (latexfile, "w");
  if (fid >= 0)
    fprintf (fid, "%s\n", prepend{:});
    fprintf (fid, "%s", latex);
    fprintf (fid, "%s\n", postpend{:});
    status = fclose (fid);
    if (status != 0)
      error ("print:errorclosingfile",
             "print.m: error closing file '%s'", latexfile)
    endif
  else
    error ("print:erroropeningfile",
           "print.m: error opening file '%s'", latexfile)
  endif
endfunction


