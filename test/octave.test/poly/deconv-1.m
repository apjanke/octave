[b, r] = deconv ([3, 6, 9, 9], [1, 2, 3]);
all (all (b == [3, 0])) && r == 9
