## Copyright (C) 2007 John W. Eaton
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

%% Automatically generated from DejaGNU files

%% test/octave.test/set/create_set-1.m
%!assert(all (all (create_set ([1, 2; 3, 4; 2, 4]) == [1, 2, 3, 4])));

%% test/octave.test/set/create_set-2.m
%!assert(all (all (create_set ([1; 2; 3; 4; 2; 4]) == [1, 2, 3, 4])));

%% test/octave.test/set/create_set-3.m
%!assert(isempty (create_set ([])));

%% test/octave.test/set/create_set-4.m
%!error create_set (1, 2);

%% test/octave.test/set/union-1.m
%!assert(all (all (union ([1, 2, 4], [2, 3, 5]) == [1, 2, 3, 4, 5])));

%% test/octave.test/set/union-2.m
%!assert(all (all (union ([1; 2; 4], [2, 3, 5]) == [1, 2, 3, 4, 5])));

%% test/octave.test/set/union-3.m
%!assert(all (all (union ([1, 2, 3], [5; 7; 9]) == [1, 2, 3, 5, 7, 9])));

%% test/octave.test/set/union-4.m
%!error union (1);

%% test/octave.test/set/union-5.m
%!error union (1, 2, 3);

%% test/octave.test/set/intersection-1.m
%!assert(all (all (intersection ([1, 2, 3], [2, 3, 5]) == [2, 3])));

%% test/octave.test/set/intersection-2.m
%!assert(all (all (intersection ([1; 2; 3], [2, 3, 5]) == [2, 3])));

%% test/octave.test/set/intersection-3.m
%!assert(isempty (intersection ([1, 2, 3], [4; 5; 6])));

%% test/octave.test/set/intersection-4.m
%!error intersection (1);

%% test/octave.test/set/intersection-5.m
%!error intersection (1, 2, 5);

%% test/octave.test/set/complement-1.m
%!assert(all (all (complement ([1, 2, 3], [3; 4; 5; 6]) == [4, 5, 6])));

%% test/octave.test/set/complement-2.m
%!assert(all (all (complement ([1, 2, 3], [3, 4, 5, 6]) == [4, 5, 6])));

%% test/octave.test/set/complement-3.m
%!assert(isempty (complement ([1, 2, 3], [3, 2, 1])));

%% test/octave.test/set/complement-4.m
%!error complement (1);

%% test/octave.test/set/complement-5.m
%!error complement (1, 2, 3);

