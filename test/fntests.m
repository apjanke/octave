## Copyright (C) 2005, 2006, 2007, 2008, 2009 David Bateman
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

clear all;

global files_with_no_tests = {};
global files_with_tests = {};
global topsrcdir;
global topbuilddir;

currdir = canonicalize_file_name (".");

if (nargin == 1)
  xdir = argv(){1};
else
  xdir = ".";
endif

srcdir = canonicalize_file_name (xdir);
topsrcdir = canonicalize_file_name (fullfile (xdir, ".."));
topbuilddir = canonicalize_file_name (fullfile (currdir, ".."));

if (strcmp (currdir, srcdir))
  testdirs = {srcdir};
else
  testdirs = {currdir, srcdir};
endif

src_tree = canonicalize_file_name (fullfile (topsrcdir, "src"));
liboctave_tree = canonicalize_file_name (fullfile (topsrcdir, "liboctave"));
script_tree = canonicalize_file_name (fullfile (topsrcdir, "scripts"));
local_script_tree = canonicalize_file_name (fullfile (currdir, "../scripts"));

fundirs = {src_tree, liboctave_tree, script_tree};

if (! strcmp (currdir, srcdir))
  fundirs{end+1} = local_script_tree;
endif

function print_test_file_name (nm)
  filler = repmat (".", 1, 55-length (nm));
  printf ("  %s %s", nm, filler);
endfunction

function print_pass_fail (n, p)
  if (n > 0)
    printf (" PASS %4d/%-4d", p, n);
    nfail = n - p;
    if (nfail > 0)
      printf (" FAIL %d", nfail);
    endif
  endif
  puts ("\n");
endfunction

function y = hasfunctions (f)
  fid = fopen (f);
  if (fid < 0)
    error ("fopen failed: %s", f);
  else
    str = fread (fid, "*char")';
    fclose (fid);
    y = regexp (str,'^(DEFUN|DEFUN_DLD)\b', "lineanchors");
  endif
endfunction

## FIXME -- should we only try match the keyword at the start of a line?
function y = hastests (f)
  fid = fopen (f);
  if (fid < 0)
    error ("fopen failed: %s", f);
  else
    str = fread (fid, "*char")';
    fclose (fid);
    y = (findstr (str, "%!test") || findstr (str, "%!assert")
	 || findstr (str, "%!error") || findstr (str, "%!warning"));
  endif
endfunction

function [dp, dn, dxf, dsk] = run_test_dir (fid, d);
  global files_with_tests;
  global files_with_no_tests;
  lst = dir (d);
  dp = dn = dxf = dsk = 0;
  for i = 1:length (lst)
    nm = lst(i).name;
    if (length (nm) > 5 && strcmp (nm(1:5), "test_")
	&& strcmp (nm((end-1):end), ".m"))
      p = n = 0;
      ffnm = fullfile (d, nm);
      if (hastests (ffnm))
	print_test_file_name (nm);
	[p, n, xf, sk] = test (nm(1:(end-2)), "quiet", fid);
	print_pass_fail (n, p);
	files_with_tests(end+1) = ffnm;
      else
	files_with_no_tests(end+1) = ffnm;
      endif
      dp += p;
      dn += n;
      dxf += xf;
      dsk += sk;
    endif
  endfor
endfunction

function [dp, dn, dxf, dsk] = run_test_script (fid, d);
  global files_with_tests;
  global files_with_no_tests;
  global topsrcdir;
  global topbuilddir;
  lst = dir (d);
  dp = dn = dxf = dsk = 0;
  for i = 1:length (lst)
    nm = lst(i).name;
    if (lst(i).isdir && ! strcmp (nm, ".") && ! strcmp (nm, "..")
	&& ! strcmp (nm, "CVS") && ! strcmp (nm, "deprecated") )
      [p, n, xf, sk] = run_test_script (fid, [d, "/", nm]);
      dp += p;
      dn += n;
      dxf += xf;
      dsk += sk;
    endif
  endfor
  for i = 1:length (lst)
    nm = lst(i).name;
    f = fullfile (d, nm);
    if ((length (nm) > 2 && strcmp (nm((end-1):end), ".m")) || 
        (length (nm) > 3 && strcmp (nm((end-2):end), ".cc") && hasfunctions(f)))
      p = n = xf = 0;
      ## Only run if it contains %!test, %!assert %!error or %!warning
      if (hastests (f))
	tmp = strrep (f, [topsrcdir, "/"], "");
	tmp = strrep (tmp, [topbuilddir, "/"], "../");
	print_test_file_name (tmp);
	[p, n, xf, sk] = test (f, "quiet", fid);
	print_pass_fail (n, p);
	dp += p;
	dn += n;
	dxf += xf;
	dsk += sk;
	files_with_tests(end+1) = f;
      else
	files_with_no_tests(end+1) = f;
      endif
    endif
  endfor 
  ##  printf("%s%s -> passes %d of %d tests\n", ident, d, dp, dn);
endfunction

function printf_assert (varargin)
  global _assert_printf;
  _assert_printf = cat (2, _assert_printf, sprintf (varargin{:}));
endfunction

function ret = prog_output_assert (str)
  global _assert_printf;
  if (isempty (_assert_printf))
    ret = isempty (str);
  elseif (_assert_printf(end) == "\n")
    ret = strcmp (_assert_printf(1:(end-1)), str);
  else
    ret = strcmp (_assert_printf, str);
  endif
  _assert_printf = "";
endfunction

function n = num_elts_matching_pattern (lst, pat)
  n = 0;
  for i = 1:length (lst)
    if (! isempty (regexp (lst{i}, pat)))
      n++;
    endif
  endfor
endfunction

function report_files_with_no_tests (with, without, typ)
  pat = cstrcat ("\\", typ, "$");
  n_with = num_elts_matching_pattern (with, pat);
  n_without = num_elts_matching_pattern (without, pat);
  n_tot = n_with + n_without;
  printf ("\n%d (of %d) %s files have no tests.\n", n_without, n_tot, typ);
endfunction

pso = page_screen_output ();
warn_state = warning ("query", "quiet");
warning ("on", "quiet");
try
  page_screen_output (0);
  fid = fopen ("fntests.log", "wt");
  if (fid < 0)
    error ("could not open fntests.log for writing");
  endif
  test ("", "explain", fid);
  dp = dn = dxf = dsk = 0;
  puts ("\nIntegrated test scripts:\n\n");
  for i = 1:length (fundirs)
    [p, n, xf, sk] = run_test_script (fid, fundirs{i});
    dp += p;
    dn += n;
    dxf += xf;
    dsk += sk;
  endfor
  puts ("\nFixed test scripts:\n\n");
  for i = 1:length (testdirs)
    [p, n, xf, sk] = run_test_dir (fid, testdirs{i});
    dp += p;
    dn += n;
    dxf += xf;
    dsk += sk;
  endfor
  printf ("\nSummary:\n\n  PASS %6d\n", dp);
  nfail = dn - dp;
  printf ("  FAIL %6d\n", nfail);
  if (dxf > 0)
    if (dxf > 1)
      t1 = "were";
      t2 = "failures";
    else
      t1 = "was";
      t2 = "failure";
    endif
    printf ("\nThere %s %d expected %s (see fntests.log for details).\n",
	    t1, dxf, t2);
    puts ("\nExpected failures are known bugs.  Please help improve\n");
    puts ("Octave by contributing fixes for them.\n");
  endif
  if (dsk > 0)
    printf ("\nThere were %d skipped tests (see fntests.log for details).\n", dsk);
    puts ("Skipped tests are features that are disabled in this version\n");
    puts ("of Octave as the needed libraries were not present when Octave\n");
    puts ("was built\n");
  endif

  report_files_with_no_tests (files_with_tests, files_with_no_tests, ".m");
  report_files_with_no_tests (files_with_tests, files_with_no_tests, ".cc");

  puts ("\nPlease help improve Octave by contributing tests for\n");
  puts ("these files (see the list in the file fntests.log).\n");

  fprintf (fid, "\nFiles with no tests:\n\n%s",
	  list_in_columns (files_with_no_tests, 80));
  fclose (fid);

  page_screen_output (pso);
  warning (warn_state.state, "quiet");
catch
  page_screen_output (pso);
  warning (warn_state.state, "quiet");
  disp (lasterr ());
end_try_catch
