function outstr = __publish_html_output__ (type, varargin)
  ## Recognized types are:
  ##
  ## "header" (title_str, intro_str, toc_cstr)
  ## "footer" ()
  ## "code" (str)
  ## "code_output" (str)
  ## "section" (str)
  ## "preformatted_code" (str)
  ## "preformatted_text" (str)
  ## "bulleted_list" (cstr)
  ## "numbered_list" (cstr)
  ## "graphic" (str)
  ## "html" (str)
  ## "latex" (str)
  ## "text" (str)
  ## "bold" (str)
  ## "italic" (str)
  ## "monospaced" (str)
  ## "link" (url_str, url_str, str)
  ## "TM" ()
  ## "R" ()

  outstr = feval (["do_" type], varargin{:});
endfunction


function outstr = do_header (title_str, intro_str, toc_cstr)

  mathjax_str = sprintf ("%s\n",
"<script type=\"text/x-mathjax-config\">",
"MathJax.Hub.Config({",
"  tex2jax: { inlineMath: [['$','$'], ['\\\\(','\\\\)']] },",
"  TeX: { equationNumbers: { autoNumber: 'all' } }",
"});",
"</script>",
["<script type=\"text/javascript\" async ", ...
 "src=\"https://cdn.mathjax.org/mathjax/latest/MathJax.js?", ...
 "config=TeX-MML-AM_CHTML\"></script>"]);

  stylesheet_str = sprintf ("%s\n",
"<style>",
"body > * {",
"  max-width: 42em;",
"}",
"body {",
"  font-family: \"Roboto Condensed\", sans-serif;",
"  padding-left: 7.5em;",
"  padding-right: 7.5em;",
"}",
"pre, code {",
"  max-width: 50em;",
"  font-family: monospace;",
"}",
"pre.oct-code {",
"  border: 1px solid Grey;",
"  padding: 5px;",
"}",
"pre.oct-code-output {",
"  margin-left: 2em;",
"}",
"span.comment {",
"  color: ForestGreen;",
"}",...
"span.keyword {",
"  color: Blue;",
"}",...
"span.string {",
"  color: DarkOrchid;",
"}",...
"footer {",
"  margin-top: 2em;",
"  font-size: 80%;",
"}",
"a, a:visited {",
"  color: Blue;",
"}",
"h2 {",
"  font-family: \"Roboto Condensed\", serif;",
"  margin-top: 1.5em;",
"}",
"h2 a, h2 a:visited {",
"  color: Black;",
"}",
"</style>");

  outstr = sprintf ("%s\n",
"<!DOCTYPE html>",
"<html>",
"<head>",
"<meta charset=\"UTF-8\">",
["<title>" title_str "</title>"],
mathjax_str,
stylesheet_str,
"</head>",
"<body>",
["<h1>" title_str "</h1>"],
intro_str);

  if (! isempty (toc_cstr))
    for i = 1:numel (toc_cstr)
      toc_cstr{i} = do_link (["#node" sprintf("%d", i)], toc_cstr{i});
    endfor
    outstr = [outstr, "<h2>Contents</h2>", do_bulleted_list(toc_cstr)];
  endif

  ## Reset section counter
  do_section ();

endfunction

function outstr = do_footer (m_source_str)
  outstr = sprintf ("%s\n",
"",
"<footer>",
"<hr>",
["<a href=\"http://www.octave.org\">Published with GNU Octave " version() "</a>"],
"</footer>",
"<!--",
"##### SOURCE BEGIN #####",
m_source_str,
"##### SOURCE END #####",
"-->",
"</body>",
"</html>");
endfunction

function outstr = do_code (str)
  outstr = ["\n", '<pre class="oct-code">' syntax_highlight(str) "</pre>\n"];
endfunction

function outstr = do_code_output (str)
  outstr = ["\n", '<pre class="oct-code-output">' str "</pre>\n"];
endfunction

function outstr = do_section (varargin)
  persistent counter = 1;

  if (nargin == 0)
    outstr = "";
    counter = 1;
    return;
  endif

  outstr = ['<h2><a id="node' sprintf("%d", counter) '">', ...
            varargin{1}, ...
            "</a></h2>"];

  counter++;

endfunction

function outstr = do_preformatted_code (str)
  outstr = ["\n", '<pre class="pre-code">' syntax_highlight(str) "</pre>\n"];
endfunction

function outstr = do_preformatted_text (str)
  outstr = ["\n", '<pre class="pre-text">' str "</pre>\n"];
endfunction

function outstr = do_bulleted_list (cstr)
  outstr = "\n<ul>\n";
  for i = 1:numel (cstr)
    outstr = [outstr, "<li>" cstr{i} "</li>\n"];
  endfor
  outstr = [outstr, "</ul>\n"];
endfunction

function outstr = do_numbered_list (cstr)
  outstr = "\n<ol>\n";
  for i = 1:numel (cstr)
    outstr = [outstr, "<li>" cstr{i} "</li>\n"];
  endfor
  outstr = [outstr, "</ol>\n"];
endfunction

function outstr = do_graphic (str)
  outstr = ['<img src="' str '" alt="' str '">'];
endfunction

function outstr = do_html (str)
  outstr = str;
endfunction

function outstr = do_latex (str)
  outstr = "";
endfunction

function outstr = do_link (url_str, str)
  outstr = ['<a href="' url_str '">' str "</a>"];
endfunction

function outstr = do_text (str)
  outstr = ["\n<p>" str "</p>\n"];
endfunction

function outstr = do_bold (str)
  outstr = ["<b>" str "</b>"];
endfunction

function outstr = do_italic (str)
  outstr = ["<i>" str "</i>"];
endfunction

function outstr = do_monospaced (str)
  outstr = ["<code>" str "</code>"];
endfunction

function outstr = do_TM ()
  outstr = "&trade;";
endfunction

function outstr = do_R ()
  outstr = "&reg;";
endfunction

## SYNTAX_HIGHLIGHT: A primitive parser to highlight syntax via <span> tags.
## FIXME: Needs to be replaced by a better solution.
function outstr = syntax_highlight (str)
  outstr = "";
  placeholder_cstr = {};
  i = 1;
  plh = 0;

  while (i <= numel (str))
    ## Block comment
    if (any (strncmp (str(i:end), {"%{", "#{"}, 2)))
      plh_str = ['<span class="comment">', str(i:i+1)];
      i += 2;
      while (i <= numel (str)
             && ! (any (strncmp (str(i:end), {"%}", "#}"}, 2))))
        plh_str = [plh_str, str(i)];
        i += 1;
      endwhile
      if (i < numel (str))
        plh_str = [plh_str, str(i:i+1), "</span>"];
        i += 2;
      else
        plh_str = [plh_str, "</span>"];
      endif
      plh += 1;
      placeholder_cstr{plh} = plh_str;
      outstr = [outstr, " PUBLISHPLACEHOLDER", sprintf("%d", plh), " "];
    ## Line comment
    elseif (str(i) == "#" || str(i) == "%")
      plh_str = '<span class="comment">';
      idx = find (str(i:end) == "\n", 1);
      if (isempty (idx))
        plh_str = [plh_str, str(i:end)];
        i = numel (str) + 1;
      else
        plh_str = [plh_str, str(i:i+idx-2)];
        i += idx;
      endif
      plh_str = [plh_str, "</span>\n"];
      plh += 1;
      placeholder_cstr{plh} = plh_str;
      outstr = [outstr, " PUBLISHPLACEHOLDER", sprintf("%d", plh), " "];
    ## Single quoted string
    elseif (str(i) == "'")
      plh_str = "<span class=\"string\">'";
      i += 1;
      while (i <= numel (str))
        ## Ignore escaped string terminations
        if (strncmp (str(i:end), "''", 2))
          plh_str = [plh_str, "''"];
          i += 2;
        ## Is char a string termination?
        elseif (str(i) == "'")
          plh_str = [plh_str, "'"];
          i += 1;
          break;
        ## Is string terminated by line break?
        elseif (str(i) == "\n")
          break;
        ## String content
        else
          plh_str = [plh_str, str(i)];
          i += 1;
        endif
      endwhile
      plh_str = [plh_str, "</span>"];
      plh += 1;
      placeholder_cstr{plh} = plh_str;
      outstr = [outstr, " PUBLISHPLACEHOLDER", sprintf("%d", plh), " "];
    ## Double quoted string
    elseif (str(i) == '"')
      plh_str = '<span class="string">"';
      i += 1;
      while (i <= numel (str))
        ## Is char a string termination?
        if (str(i) == '"' && str(i-1) != '\')
          plh_str = [plh_str, '"'];
          i += 1;
          break;
        ## Is string terminated by line break?
        elseif (str(i) == "\n")
          break;
        ## String content
        else
          plh_str = [plh_str, str(i)];
          i += 1;
        endif
      endwhile
      plh_str = [plh_str, "</span>"];
      plh += 1;
      placeholder_cstr{plh} = plh_str;
      outstr = [outstr, " PUBLISHPLACEHOLDER", sprintf("%d", plh), " "];
    else
      outstr = [outstr, str(i)];
      i += 1;
    endif
  endwhile

  persistent kword_ptn = strjoin (iskeyword (), '|');

  ## FIXME: remove hack for regexprep once bug #38149 is solved
  outstr = [" ", strrep(outstr, "\n", " \n "), " "];
  outstr = regexprep (outstr,
                      ['(\s)(' kword_ptn ')(\s|\()'],
                      ['$1<span class="keyword">$2</span>$3']);
  ## FIXME: remove hack for regexprep once bug #38149 is solved
  outstr = strrep (outstr(2:end-1), " \n ", "\n");

  ## Restore placeholders
  for i = plh:-1:1
    outstr = strrep (outstr, [" PUBLISHPLACEHOLDER", sprintf("%d", i), " "],
                             placeholder_cstr{i});
  endfor

endfunction

