function Aib = qtransvmat(qib)
# function Aib = qtransvmat(qib)
# construct 3x3  transformation matrix from quaternion qib
# Aib is equivalent to rotation of th radians about axis vv, where
#    [vv,th] = quaternion(qib)

if(!is_vector(qib) | length(qib) != 4)
  error(sprintf("qtransvmat: q(%d,%d) must be a quaternion",rows(qib),columns(qib)))
elseif(max(abs(imag(qib))) != 0)
  qib
  error("qtransvmat: input values must be real.");
endif

Aib = [ (2.*(qib(1)^2 + qib(4)^2) -1.), ...
	  (2.*(qib(1)*qib(2)-qib(3)*qib(4))), ...
	  (2.*(qib(1)*qib(3)+qib(2)*qib(4))); ...
	(2.*(qib(1)*qib(2)+qib(3)*qib(4))), ...
	  (2.*(qib(2)*qib(2)+qib(4)*qib(4))-1.), ...
	  (2.*(qib(2)*qib(3)-qib(1)*qib(4))); ...
	(2.*(qib(1)*qib(3)-qib(2)*qib(4))), ...
	  (2.*(qib(2)*qib(3)+qib(1)*qib(4))), ...
	  (2.*(qib(3)*qib(3)+qib(4)*qib(4))-1.)];
endfunction

