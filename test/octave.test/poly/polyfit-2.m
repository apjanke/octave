x = [-2, -1, 0, 1, 2];
all (all (polyfit (x, x.^2+x+1, 3) - [0; 1; 1; 1] < 8*eps))
