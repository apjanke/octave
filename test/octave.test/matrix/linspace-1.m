x1 = linspace (1, 2);
x2 = linspace (1, 2, 10);
x3 = linspace (1, -2, 10);
(size (x1) == [1, 100] && x1(1) == 1 && x1(100) == 2
 && size (x2) == [1, 10] && x2(1) == 1 && x2(10) == 2
 && size (x3) == [1, 10] && x3(1) == 1 && x3(10) == -2)
