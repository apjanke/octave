function x = umfpack_solve (arg1, op, arg2, Control)
% UMFPACK_SOLVE
%
% x = umfpack_solve (A, '\', b, Control)
% x = umfpack_solve (b, '/', A, Control)
%
% Computes x = A\b, or b/A, where A is square.  Uses UMFPACK if A is sparse.
% The Control argument is optional.
%
% See also umfpack, umfpack_make, umfpack_details, umfpack_report,
% and umfpack_simple.

% UMFPACK Version 4.4, Copyright (c) 2005 by Timothy A. Davis.
% All Rights Reserved.  Type umfpack_details for License.

%-------------------------------------------------------------------------------
% check inputs and get default control parameters
%-------------------------------------------------------------------------------

if (op == '\')
    A = arg1 ;
    b = arg2 ;
elseif (op == '/')
    A = arg2 ;
    b = arg1 ;
else
    help umfack_solve
    error ('umfpack_solve:  unrecognized operator') ;
end

[m n] = size (A) ;
if (m ~= n)
    help umfpack_solve
    error ('umfpack_solve:  A must be square') ;
end

[m1 n1] = size (b) ;
if ((op == '\' & n ~= m1) | (op == '/' & n1 ~= m))
    help umfpack_solve
    error ('umfpack_solve:  b has the wrong dimensions') ;
end

if (nargin < 4)
    Control = umfpack ;
end

%-------------------------------------------------------------------------------
% solve the system
%-------------------------------------------------------------------------------

if (op == '\')

    if (~issparse (A))

	% A is not sparse, so just use MATLAB
	x = A\b ;

    elseif (n1 == 1 & ~issparse (b))

	% the UMFPACK '\' requires b to be a dense column vector
	x = umfpack (A, '\', b, Control) ;

    else

	% factorize with UMFPACK and do the forward/back solves in MATLAB
	[L, U, P, Q, R] = umfpack (A, Control) ;
	x = Q * (U \ (L \ (P * (R \ b)))) ;

    end

else

    if (~issparse (A))

	% A is not sparse, so just use MATLAB
	x = b/A ;

    elseif (m1 == 1 & ~issparse (b))

	% the UMFPACK '\' requires b to be a dense column vector
	x = umfpack (b, '/', A, Control) ;

    else

	% factorize with UMFPACK and do the forward/back solves in MATLAB
	% this mimics the behavior of x = b/A, except for the row scaling
	[L, U, P, Q, R] = umfpack (A.', Control) ;
	x = (Q * (U \ (L \ (P * (R \ (b.')))))).' ;

	% an alternative method:
	% [L, U, P, Q, r] = umfpack (A, Control) ;
	% x = (R \ (P' * (L.' \ (U.' \ (Q' * b.'))))).' ;

    end

end
