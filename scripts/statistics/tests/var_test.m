## Copyright (C) 1995, 1996, 1997  Kurt Hornik
## 
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
## 
## This program is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details. 
## 
## You should have received a copy of the GNU General Public License
## along with this file.  If not, write to the Free Software Foundation,
## 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

## usage:  [pval, f, df_num, df_den] = var_test (x, y [, alt])
##
## For two samples x and y from normal distributions with unknown 
## means and unknown variances, perform an F-test of the null
## hypothesis of equal variances.
## Under the null, the test statistic f follows an F-distribution
## with df_num and df_den degrees of freedom.
##
## With the optional argument string alt, the alternative of interest
## can be selected.  
## If alt is "!=" or "<>", the null is tested against the two-sided
## alternative var(x) != var(y).
## If alt is ">", the one-sided alternative var(x) > var(y) is used,
## similarly for "<".  
## The default is the two-sided case.
##
## pval is the p-value of the test.
##  
## If no output argument is given, the p-value of the test is displayed.

## Author:  KH <Kurt.Hornik@ci.tuwien.ac.at>
## Description:  F test to compare two variances

function [pval, f, df_num, df_den] = var_test (x, y, alt)
  
  if ((nargin < 2) || (nargin > 3))
    usage ("[pval, f, df_num, df_den] = var_test (x, y [, alt])");
  endif
    
  if (! (is_vector (x) && is_vector (y)))
    error ("var_test:  both x and y must be vectors");
  endif

  df_num = length (x) - 1;
  df_den = length (y) - 1;
  f      = var (x) / var (y);
  cdf    = f_cdf (f, df_num, df_den);
  
  if (nargin == 2)
    alt  = "!=";
  endif
    
  if (! isstr (alt))
    error ("var_test:  alt must be a string");
  endif
  if (strcmp (alt, "!=") || strcmp (alt, "<>"))
    pval = 2 * min (cdf, 1 - cdf);
  elseif (strcmp (alt, ">"))
    pval = 1 - cdf;
  elseif (strcmp (alt, "<"))
    pval = cdf;
  else
    error (sprintf ("var_test:  option %s not recognized", alt));
  endif
  
  if (nargout == 0)
    printf ("pval:  %g\n", pval);
  endif

endfunction
