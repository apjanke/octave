%% Automatically generated from DejaGNU files

%% test/octave.test/try/try-1.m
%!test
%! try
%! catch
%!   error("Shoudn't get here");
%! end_try_catch

%% test/octave.test/try/try-2.m
%!test
%! try
%!   clear a
%!   a;
%! catch
%! end_try_catch
%! a = 1;
%! assert(a,1);

%% test/octave.test/try/try-3.m
%!test
%! clear x;
%! try
%!   clear a
%!   a;
%!   x = 1;
%! catch
%! end_try_catch
%! a = 2;
%! assert(!exist('x'))
%! assert(a,2)

%% test/octave.test/try/try-4.m
%!test
%! try
%!   clear a
%!   a;
%! catch
%!   x = 1;
%! end_try_catch
%! assert(exist('x'))

%% test/octave.test/try/try-5.m
%!test
%! try
%!   clear a;
%!   a;
%!   error("Shoudn't get here");
%! catch
%!   assert (strcmp(lasterr()(1:20), "error: `a' undefined"))
%! end_try_catch
%! assert (strcmp(lasterr()(1:20), "error: `a' undefined"))

%% test/octave.test/try/try-6.m
%!test 
%! try
%!   error ("user-defined error")
%! catch
%!   assert(lasterr,"error: user-defined error\n");
%! end_try_catch

%% test/octave.test/try/try-7.m
%!function ms = mangle (s)
%!  ## Wrap angle brackets around S.
%!  ms = strcat ("<", s, ">");
%!test
%! try
%!   clear a
%!   a;
%!   error("Shoudn't get here");
%! catch
%!   assert(strcmp(mangle (lasterr)(1:21),"<error: `a' undefined"))
%! end_try_catch


%% test/octave.test/try/try-8.m
%!test
%! try
%!   try
%!     clear a
%!     a;
%!     error("Shoudn't get here");
%!   catch
%!     assert(strcmp(lasterr()(1:20), "error: `a' undefined"))
%!   end_try_catch
%!   clear b
%!   b;
%!   error("Shoudn't get here");
%! catch
%!   assert(strcmp(lasterr()(1:20), "error: `b' undefined"))
%! end_try_catch

%% test/octave.test/try/try-9.m
%!test
%! try
%!   clear a
%!   a;
%!   error("Shoudn't get here");
%! catch
%!   try
%!     assert(strcmp(lasterr()(1:20), "error: `a' undefined"))
%!     clear b
%!     b;
%!     error("Shoudn't get here");
%!   catch
%!     assert(strcmp(lasterr()(1:20), "error: `b' undefined"))
%!   end_try_catch
%! end_try_catch

%% test/octave.test/try/try-10.m
%!test
%! try
%!   try
%!     clear a
%!     a;
%!     error("Shoudn't get here");
%!   catch
%!     error(strcat("rethrow: ",lasterr));
%!   end_try_catch
%! catch
%!   assert(strcmp(lasterr()(1:36), "error: rethrow: error: `a' undefined"))
%! end_try_catch

