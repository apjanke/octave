function display (p)

  a = p.poly;
  first = true;
  printf ("%s =", inputname (1));
  for i = 1 : length (a);
    if (a(i) != 0)
      if (first)
        first = false;
      elseif (a(i) > 0)
        printf (" +");
      endif
      if (a(i) < 0)
        printf (" -");
      endif
      if (i == 1)
        printf (" %g", abs (a(i)));
      elseif (abs(a(i)) != 1)
        printf (" %g *", abs (a(i)));
      endif
      if (i > 1)
        printf (" X");
      endif
      if (i > 2)
        printf (" ^ %d", i - 1);
      endif
    endif
  endfor

  if (first)
    printf (" 0");
  endif
  printf ("\n");

endfunction
