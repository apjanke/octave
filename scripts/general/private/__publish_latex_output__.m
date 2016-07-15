function outstr = __publish_latex_output__ (varargin)
  ##
  ## Types to handle are:
  ##
  ## * "header" (title_str, intro_str, toc_cstr)
  ## * "footer" ()
  ## * "code" (str)
  ## * "code_output" (str)
  ## * "section" (str)
  ## * "preformatted_code" (str)
  ## * "preformatted_text" (str)
  ## * "bulleted_list" (cstr)
  ## * "numbered_list" (cstr)
  ## * "graphic" (str)
  ## * "html" (str)
  ## * "latex" (str)
  ## * "text" (str)
  ## * "bold" (str)
  ## * "italic" (str)
  ## * "monospaced" (str)
  ## * "link" (url_str, url_str, str)
  ## * "TM" ()
  ## * "R" ()
  ##
  eval (["outstr = handle_", varargin{1}, " (varargin{2:end});"]);
endfunction

function outstr = handle_header (title_str, intro_str, toc_cstr)
  publish_comment = ["\n\n", ...
    "% This document was generated by the publish-function\n", ...
    "% from GNU Octave ", version(), "\n\n"];

  latex_preamble = ["\n\n", ...
    "\\documentclass[10pt]{article}\n", ...
    "\\usepackage{listings}\n", ...
    "\\usepackage{mathtools}\n", ...
    "\\usepackage{graphicx}\n", ...
    "\\usepackage{hyperref}\n", ...
    "\\usepackage{xcolor}\n", ...
    "\\usepackage{titlesec}\n", ...
    "\\usepackage[utf8]{inputenc}\n", ...
    "\\usepackage[T1]{fontenc}\n", ...
    "\\usepackage{lmodern}\n"];

  listings_option = ["\n\n", ...
    "\\lstset{\n", ...
    "language=Octave,\n", ...
    "numbers=none,\n", ...
    "frame=single,\n", ...
    "tabsize=2,\n", ...
    "showstringspaces=false,\n", ...
    "breaklines=true}\n"];

  ## Escape "_" in title_str, as many file names contain it
  latex_head = ["\n\n", ...
    "\\titleformat*{\\section}{\\Huge\\bfseries}\n", ...
    "\\titleformat*{\\subsection}{\\large\\bfseries}\n", ...
    "\\renewcommand{\\contentsname}{\\Large\\bfseries Contents}\n", ...
    "\\setlength{\\parindent}{0pt}\n\n",...
    "\\begin{document}\n\n", ...
    "{\\Huge\\section*{", escape_chars(title_str),"}}\n\n", ...
    "\\tableofcontents\n", ...
    "\\vspace*{4em}\n\n"];
    
  outstr = [publish_comment, latex_preamble, listings_option, latex_head];
endfunction

function outstr = handle_footer (m_source_str)
  outstr = ["\n\n\\end{document}\n"];
endfunction

function outstr = handle_code (str)
  outstr = ["\\begin{lstlisting}\n", str, "\n\\end{lstlisting}\n"];
endfunction

function outstr = handle_code_output (str)
  outstr = ["\\begin{lstlisting}", ...
    "[language={},xleftmargin=5pt,frame=none]\n", ...
    str, "\n\\end{lstlisting}\n"];
endfunction

function outstr = handle_section (str)
  outstr = ["\n\n\\phantomsection\n", ...
    "\\addcontentsline{toc}{section}{", str, "}\n", ...
    "\\subsection*{", str, "}\n\n"];
endfunction

function outstr = handle_preformatted_code (str)
  outstr = ["\\begin{lstlisting}\n", str, "\n\\end{lstlisting}\n"];
endfunction

function outstr = handle_preformatted_text (str)
  outstr = ["\\begin{lstlisting}[language={}]\n", ...
    str, "\n\\end{lstlisting}\n"];
endfunction

function outstr = handle_bulleted_list (cstr)
  outstr = "\n\\begin{itemize}\n";
  for i = 1:length(cstr)
    outstr = [outstr, "\\item ", cstr{i}, "\n"];
  endfor
  outstr = [outstr, "\\end{itemize}\n"];
endfunction

function outstr = handle_numbered_list (cstr)
  outstr = "\n\\begin{enumerate}\n";
  for i = 1:length(cstr)
    outstr = [outstr, "\\item ", cstr{i}, "\n"];
  endfor
  outstr = [outstr, "\\end{enumerate}\n"];
endfunction

function outstr = handle_graphic (str)
  outstr = ["\\begin{figure}[!ht]\n", ...
    "\\includegraphics[width=\\textwidth]{", str, "}\n", ...
    "\\end{figure}\n"];
endfunction

function outstr = handle_html (str)
  outstr = "";
endfunction

function outstr = handle_latex (str)
  outstr = str;
endfunction

function outstr = handle_link (url_str, str)
  outstr = ["\\href{", url_str,"}{", str, "}"];
endfunction

function outstr = handle_text (str)
  outstr = ["\n\n", str, "\n\n"];
endfunction

function outstr = handle_bold (str)
  outstr = ["\\textbf{", str, "}"];
endfunction

function outstr = handle_italic (str)
  outstr = ["\\textit{", str, "}"];
endfunction

function outstr = handle_monospaced (str)
  outstr = ["\\texttt{", str, "}"];
endfunction

function outstr = handle_TM ()
  outstr = "\\texttrademark";
endfunction

function outstr = handle_R ()
  outstr = "\\textregistered";
endfunction

function str = escape_chars (str)
  ## Escape "_"
  str = regexprep (str, '([^\\]|^)(_)', "$1\\_");
endfunction
