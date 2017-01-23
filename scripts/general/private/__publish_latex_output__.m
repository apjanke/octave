## Copyright (C) 2016 Kai T. Ohlhus
##
## This file is part of Octave.
##
## Octave is free software; you can redistribute it and/or modify it
## under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 3 of the License, or
## (at your option) any later version.
##
## Octave is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with Octave; see the file COPYING.  If not, see
## <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn {} {@var{outstr} =} __publish_latex_output__ (@var{type}, @var{varargin})
##
## Internal function.
##
## The first input argument @var{type} defines the required strings
## (@samp{str}) or cell-strings (@samp{cstr}) in @var{varargin} in order
## to produce LaTeX output.
##
## @var{type} is one of
##
## @itemize @bullet
## @item
## @samp{header} (title_str, intro_str, toc_cstr)
## @item
## @samp{footer} ()
## @item
## @samp{code} (str)
## @item
## @samp{code_output} (str)
## @item
## @samp{section} (str)
## @item
## @samp{preformatted_code} (str)
## @item
## @samp{preformatted_text} (str)
## @item
## @samp{bulleted_list} (cstr)
## @item
## @samp{numbered_list} (cstr)
## @item
## @samp{graphic} (str)
## @item
## @samp{html} (str)
## @item
## @samp{latex} (str)
## @item
## @samp{text} (str)
## @item
## @samp{bold} (str)
## @item
## @samp{italic} (str)
## @item
## @samp{monospaced} (str)
## @item
## @samp{link} (url_str, url_str, str)
## @item
## @samp{TM} ()
## @item
## @samp{R} ()
## @end itemize
## @end deftypefn

function outstr = __publish_latex_output__ (type, varargin)
  outstr = feval (["do_" type], varargin{:});
endfunction

function outstr = do_header (title_str, intro_str, toc_cstr)
  publish_comment = sprintf ("%s\n",
"",
"",
"% This document was generated by the publish-function",
["% from GNU Octave " version()],
"");

  latex_preamble = sprintf ("%s\n",
"",
"",
'\documentclass[10pt]{article}',
'\usepackage{listings}',
'\usepackage{mathtools}',
'\usepackage{amssymb}',
'\usepackage{graphicx}',
'\usepackage{hyperref}',
'\usepackage{xcolor}',
'\usepackage{titlesec}',
'\usepackage[utf8]{inputenc}',
'\usepackage[T1]{fontenc}',
'\usepackage{lmodern}');

  listings_option = sprintf ("%s\n",
"",
"",
'\lstset{',
'language=Octave,',
'numbers=none,',
'frame=single,',
'tabsize=2,',
'showstringspaces=false,',
'breaklines=true}');

  latex_head = sprintf ("%s\n",
"",
"",
'\titleformat*{\section}{\Huge\bfseries}',
'\titleformat*{\subsection}{\large\bfseries}',
'\renewcommand{\contentsname}{\Large\bfseries Contents}',
'\setlength{\parindent}{0pt}',
"",
'\begin{document}',
"",
['{\Huge\section*{' escape_latex(title_str) '}}'],
"",
'\tableofcontents',
'\vspace*{4em}',
"");

  outstr = [publish_comment, latex_preamble, listings_option, latex_head];

endfunction

function outstr = do_footer (m_source_str)
  outstr = ["\n\n" '\end{document}' "\n"];
endfunction

function outstr = do_code (str)
  outstr = ['\begin{lstlisting}' "\n", str, "\n" '\end{lstlisting}' "\n"];
endfunction

function outstr = do_code_output (str)
  outstr = sprintf ("%s\n",
'\begin{lstlisting}[language={},xleftmargin=5pt,frame=none]',
str,
'\end{lstlisting}');
endfunction

function outstr = do_section (str)
  outstr = sprintf ("%s\n",
"",
"",
'\phantomsection',
['\addcontentsline{toc}{section}{' escape_latex(str) '}'],
['\subsection*{' escape_latex(str) '}'],
"");
endfunction

function outstr = do_preformatted_code (str)
  outstr = sprintf ("%s\n",
'\begin{lstlisting}',
str,
'\end{lstlisting}');
endfunction

function outstr = do_preformatted_text (str)
  outstr = sprintf ("%s\n",
'\begin{lstlisting}[language={}]',
str,
'\end{lstlisting}');
endfunction

function outstr = do_bulleted_list (cstr)
  outstr = ["\n" '\begin{itemize}' "\n"];
  for i = 1:numel (cstr)
    outstr = [outstr, '\item ' escape_latex(cstr{i}) "\n"];
  endfor
  outstr = [outstr, '\end{itemize}' "\n"];
endfunction

function outstr = do_numbered_list (cstr)
  outstr = ["\n" '\begin{enumerate}' "\n"];
  for i = 1:numel (cstr)
    outstr = [outstr, '\item ' escape_latex(cstr{i}) "\n"];
  endfor
  outstr = [outstr, "\\end{enumerate}\n"];
endfunction

function outstr = do_graphic (str)
  outstr = sprintf ("%s\n",
'\begin{figure}[!ht]',
['\includegraphics[width=\textwidth]{' str '}'],
'\end{figure}');
endfunction

function outstr = do_html (str)
  outstr = "";
endfunction

function outstr = do_latex (str)
  outstr = str;
endfunction

function outstr = do_link (url_str, str)
  outstr = ['\href{' url_str '}{' str '}'];
endfunction

function outstr = do_text (str)
  outstr = ["\n\n" escape_latex(str) "\n\n"];
endfunction

function outstr = do_bold (str)
  outstr = ['\textbf{' str '}'];
endfunction

function outstr = do_italic (str)
  outstr = ['\textit{' str '}'];
endfunction

function outstr = do_monospaced (str)
  outstr = ['\texttt{' str '}'];
endfunction

function outstr = do_TM ()
  outstr = '\texttrademark ';
endfunction

function outstr = do_R ()
  outstr = '\textregistered ';
endfunction

function str = escape_latex (str)
  ## Escape "&", "%", "#", "_", "~", "^", "<", ">"
  ## FIXME: What about: "\", "{", "}"
  str = regexprep (str, '(?<!\\)(&|%|#|_)', '\\$1');
  str = regexprep (str, '(?<!\\)(~)', "\\ensuremath{\\tilde{\;}}");
  str = regexprep (str, '(?<!\\)(\^)', "\\^{}");
  str = regexprep (str, '(?<!\\)(<)', "\\ensuremath{<}");
  str = regexprep (str, '(?<!\\)(>)', "\\ensuremath{>}");
endfunction
