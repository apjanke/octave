[l u p] = lu ([1, 2; 3, 4; 5, 6]);
(abs (l - [1, 0; 1/5, 1; 3/5, 1/2]) < sqrt (eps)
 && abs (u - [5, 6; 0, 4/5]) < sqrt (eps)
 && abs (p - [0, 0, 1; 1, 0, 0; 0 1 0]) < sqrt (eps))
