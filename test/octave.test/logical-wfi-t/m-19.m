warn_fortran_indexing = 1;
a = [9,8;7,6];
a(logical ([0,1]),1) == 7
