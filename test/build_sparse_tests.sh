#!/bin/sh

# Copyright (C) 2006, 2007 David Bateman
#
# This file is part of Octave.
# 
# Octave is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 3 of the License, or (at
# your option) any later version.
# 
# Octave is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
# 
# You should have received a copy of the GNU General Public License
# along with Octave; see the file COPYING.  If not, see
# <http://www.gnu.org/licenses/>.

# Some tests are commented out because they are known to be broken!
# Search for "# fails"   

# ./buildtest.sh preset
#    creates test_sparse.m with preset tests.
#    Use "test test_sparse" from octave to run the tests.
#
# ./buildtest.sh random
#    Creates test_sprandom.m with randomly generated matrices.

# buildtest.sh generates tests for real and complex sparse matrices.
# Also, we want to run both fixed tests with known outputs (quick tests)
# and longer tests with unknown outputs (thorough tests).  This requires
# two sets of tests --- one which uses preset matrices and another which
# uses randomly generated matrices.
#
# The tests are mostly identical for each case but the code is different,
# so it is important that the tests be run on all cases.  Because our test 
# harness doesn't have support for looping or macros (it is only needed
# for new data types), but sh does, we use sh to generate inline versions of
# the tests for each case.
#
# Our 'macros' use shared variables as parameters.  This allows us to
# for example define A complex and include all the unary ops tests, 
# then set A=real(A) and include all the unary ops tests.  Thus the
# same tests work for real and complex.  For binary tests it is even
# more complicated because we want full X sparse, sparse X full and
# sparse X sparse tested.
#
# We use the following macros:
#
#    gen_section
#        place a separator in the test file
#    gen_function
#        define the function definion
#    helper gen_specific
#        specific tests such as error handling and null input
#    helper gen_eat_zeros
#        make sure sparse-scalar ops which generate 0 work
#    gen_specific_tests
#        specific and eat zeros tests 
#    helper gen_ordering_tests
#        ordered comparison operators for real valued tests
#    helper gen_sparsesparse_ordering_tests
#        ordered comparison operators for real valued sparse-sparse tests
#    helper gen_elementop_tests
#        element-wise matrix binary operators, including scalar-matrix ops.
#        horizontal/vertical concatenation are here as well.
#    helper gen_sparsesparse_elementop_tests
#        element-wise matrix binary operators, for sparse-sparse ops.
#        horizontal/vertical concatenation are here as well.
#    helper gen_divop_tests
#        left and right matrix division operators of rectangular matrices. 
#        Needs QR solvers
#    helper gen_square_divop_tests
#        left and right matrix division operators of square matrices. 
#    helper gen_matrixop_tests
#        rectangular matrix binary operators: * 
#    helper gen_matrixdiag_tests
#        Tests extract of diag and creation of diagonal matrices using
#        diag and spdiags functions
#    helper gen_matrixreshape_tests
#        Test the reshape function on sparse matrices
#    helper print_mapper_test
#        sub-helper function of gen_mapper_tests to print individual tests
#    helper gen_mapper_tests
#        Tests all of the one argument mapper functions. There are a few
#        specific tests that abs, real and imag return real values.
#    helper gen_unaryop_tests
#        functions and operators which transform a single matrix
#    helper gen_save_tests
#        Tests the load/save functionality for ascii/binary and hdf5 formats
#    gen_scalar_tests
#        element ops for real and complex scalar and sparse
#    gen_rectangular_tests
#        unary, element, and matrix tests for a and full/sparse b
#    gen_square_tests
#        operations which require square matrices: lu, inv, \
#        A square non-singular matrix is defined from the rectangular
#        inputs A and B.
#    gen_assembly_tests
#        test for sparse constructors with 'sum' vs. 'unique'
#    gen_select_tests
#        indexing tests
#    gen_solver_tests
#        Tests the solve function with triangular/banded, etc matrices

case $1 in
    random) preset=false ;;
    preset) preset=true ;;
    '') preset=true ;;
    *) echo "buildtest.sh random|preset" && exit 1 ;;
esac

if $preset; then
    TESTS=test_sparse.m
else
    TESTS=test_sprandom.m
fi

# create initial file
cat >$TESTS <<EOF
## THIS IS AN AUTOMATICALLY GENERATED FILE --- DO NOT EDIT ---
## instead modify build_sparse_tests.sh to generate the tests that you want.
EOF


# define all functions


# =======================================================
# Section separator

gen_section() {
cat >>$TESTS <<EOF

# ==============================================================

EOF
}


# =======================================================
# Specific preset tests

# =======================================================
# If a sparse operation yields zeros, then those elements 
# of the returned sparse matrix should be eaten.
gen_eat_zeros() {
cat >>$TESTS <<EOF
%% Make sure newly introduced zeros get eaten
%!assert(nnz(sparse([bf,bf,1]).^realmax),1);
%!assert(nnz(sparse([1,bf,bf]).^realmax),1);
%!assert(nnz(sparse([bf,bf,bf]).^realmax),0); 

%!assert(nnz(sparse([bf;bf;1]).^realmax),1);
%!assert(nnz(sparse([1;bf;bf]).^realmax),1);
%!assert(nnz(sparse([0.5;bf;bf]).^realmax),0);

%!assert(nnz(sparse([bf,bf,1])*realmin),1);
%!assert(nnz(sparse([1,bf,bf])*realmin),1);
%!assert(nnz(sparse([bf,bf,bf])*realmin),0);

%!assert(nnz(sparse([bf;bf;1])*realmin),1);
%!assert(nnz(sparse([1;bf;bf])*realmin),1);
%!assert(nnz(sparse([bf;bf;bf])*realmin),0);

EOF
}

gen_specific() {
cat >>$TESTS <<EOF

%!test # segfault test from edd@debian.org
%! n = 510;
%! sparse(kron((1:n)', ones(n,1)), kron(ones(n,1), (1:n)'), ones(n)); 

%% segfault tests from Fabian@isas-berlin.de
%% Note that the last four do not fail, but rather give a warning
%% of a singular matrix, which is consistent with the full matrix
%% behaviour. They are therefore disabled.. 
%!assert(spinv(sparse([1,1;1,1+i])),sparse([1-1i,1i;1i,-1i]),10*eps);
% !error spinv( sparse( [1,1;1,1]   ) );
% !error spinv( sparse( [0,0;0,1]   ) );
% !error spinv( sparse( [0,0;0,1+i] ) );
% !error spinv( sparse( [0,0;0,0]   ) );

%% error handling in constructor
%!error sparse(1,[2,3],[1,2,3]);
%!error sparse([1,1],[1,1],[1,2],3,3,"bogus");
%!error sparse([1,3],[1,-4],[3,5],2,2);
%!error sparse([1,3],[1,-4],[3,5i],2,2);
%!error sparse(-1,-1,1);
EOF
}


gen_specific_tests() {
    gen_section
    gen_specific
    gen_section
    echo '%!shared bf' >> $TESTS
    echo '%!test bf=realmin;' >> $TESTS
    gen_eat_zeros
    echo '%!test bf=realmin+realmin*1i;' >> $TESTS
    gen_eat_zeros
    cat >>$TESTS <<EOF
%!assert(nnz(sparse([-1,realmin,realmin]).^1.5),1);
%!assert(nnz(sparse([-1,realmin,realmin,1]).^1.5),2);

%!assert(nnz(sparse(1,1,0)),0); # Make sure scalar v==0 doesn't confuse matters
%!assert(nnz(sparse(eye(3))*0),0);
%!assert(nnz(sparse(eye(3))-sparse(eye(3))),0);

%!test
%! wdbz = warning ("query", "Octave:divide-by-zero");
%! warning ("off", "Octave:divide-by-zero");
%! assert(sparse(eye(3))/0,sparse(eye(3)/0,1));
%! warning (wdbz.state, "Octave:divide-by-zero");

EOF
}


# =======================================================
# Main function definition

gen_function() {
    if $preset; then
	cat >>$TESTS <<EOF
##
## test_sparse
##
##    run preset sparse tests.  All should pass.
function [passes,tests] = test_sparse
  disp("writing test output to sptest.log");
  test("test_sparse","normal","sptest.log");
endfunction

EOF
    else
	cat >>$TESTS <<EOF
##
## test_sprandom
##
##  total_passes=0; total_tests=0;
##  for i=1:10
##     [passes,tests] = sprandomtest;
##    total_passes += passes;
##    total_tests += tests;
##  end
##  The test log is appended to sprandomtest.log
function [passes,total] = test_sprandom
  warning("untested --- fix the source in buildtests.sh");
  disp("appending test output to sprandomtest.log");
  fid = fopen("sprandomtest.log","at");
  test("test_sprandom","normal",fid);
  ##[passes, total] = test("sprandomtest","normal",fid);
  fclose(fid);
endfunction

EOF
    fi
    
}


# =======================================================
# matrix ops

# test ordered comparisons: uses as,af,bs,bf
gen_ordering_tests() {
    cat >>$TESTS <<EOF
%% real values can be ordered (uses as,af)
%!assert(as<=bf,sparse(af<=bf,true))
%!assert(bf<=as,sparse(bf<=af,true))

%!assert(as>=bf,sparse(af>=bf,true))
%!assert(bf>=as,sparse(bf>=af,true))

%!assert(as<bf,sparse(af<bf,true))
%!assert(bf<as,sparse(bf<af,true))

%!assert(as>bf,sparse(af>bf,true))
%!assert(bf>as,sparse(bf>af,true))

EOF
}

gen_sparsesparse_ordering_tests() {
    cat >>$TESTS <<EOF
%!assert(as<=bs,sparse(af<=bf,true))
%!assert(as>=bs,sparse(af>=bf,true))
%!assert(as<bs,sparse(af<bf,true))
%!assert(as>bs,sparse(af>bf,true))
EOF
}

# test element-wise binary operations: uses as,af,bs,bf,scalar
gen_elementop_tests() {
    cat >>$TESTS <<EOF
%% Elementwise binary tests (uses as,af,bs,bf,scalar)
%!assert(as==bs,sparse(af==bf,true))
%!assert(bf==as,sparse(bf==af,true))

%!assert(as!=bf,sparse(af!=bf,true))
%!assert(bf!=as,sparse(bf!=af,true))

%!assert(as+bf,af+bf)
%!assert(bf+as,bf+af)

%!assert(as-bf,af-bf)
%!assert(bf-as,bf-af)

%!assert(as.*bf,sparse(af.*bf,true))
%!assert(bf.*as,sparse(bf.*af,true))

%!assert(as./bf,sparse(af./bf,true),100*eps)
%!assert(bf.\as,sparse(bf.\af,true),100*eps)

%!test
%! sv = as.^bf;
%! fv = af.^bf;
%! idx = find(af~=0);
%! assert(sv(:)(idx),sparse(fv(:)(idx),true),100*eps)

EOF
}

gen_sparsesparse_elementop_tests() {
    cat >>$TESTS <<EOF
%!assert(as==bs,sparse(af==bf,true))
%!assert(as!=bs,sparse(af!=bf,true))
%!assert(as+bs,sparse(af+bf,true))
%!assert(as-bs,sparse(af-bf,true))
%!assert(as.*bs,sparse(af.*bf,true))
%!xtest assert(as./bs,sparse(af./bf,true),100*eps);
%!test
%! sv = as.^bs;
%! fv = af.^bf;
%! idx = find(af~=0);
%! assert(sv(:)(idx),sparse(fv(:)(idx),true),100*eps)

EOF
}

# test matrix-matrix left and right division: uses as,af,bs,bf
gen_divop_tests() {
    cat >>$TESTS <<EOF
%% Matrix-matrix operators (uses af,as,bs,bf)
%!assert(as/bf,af/bf,100*eps)
%!assert(af/bs,af/bf,100*eps)
%!assert(as/bs,sparse(af/bf,true),100*eps)
%!assert(bs\af',bf\af',100*eps)
%!assert(bf\as',bf\af',100*eps)
%!assert(bs\as',sparse(bf\af',true),100*eps)

EOF
}

# test matrix-matrix left and right division: uses as,af,bs,bf
gen_square_divop_tests() {
    cat >>$TESTS <<EOF
%% Matrix-matrix operators (uses af,as,bs,bf)
%!assert(as/bf,af/bf,100*eps)
%!assert(af/bs,af/bf,100*eps)
%!assert(as/bs,sparse(af/bf,true),100*eps)
%!assert(bs\af',bf\af',100*eps)
%!assert(bf\as',bf\af',100*eps)
%!assert(bs\as',sparse(bf\af',true),100*eps)

EOF
}

# test matrix-matrix operations: uses as,af,bs,bf
gen_matrixop_tests() {
    cat >>$TESTS <<EOF
%% Matrix-matrix operators (uses af,as,bs,bf)
%!assert(as*bf',af*bf')
%!assert(af*bs',af*bf')
%!assert(as*bs',sparse(af*bf',true))

EOF
}

# test diagonal operations
gen_matrixdiag_tests() {
    cat >>$TESTS <<EOF
%% Matrix diagonal tests (uses af,as,bf,bs)
%!assert(spdiag(as),sparse(diag(af),true))
%!assert(spdiag(bs),sparse(diag(bf),true))
%!assert(spdiag(as,1),sparse(diag(af,1),true))
%!assert(spdiag(bs,1),sparse(diag(bf,1),true))
%!assert(spdiag(as,-1),sparse(diag(af,-1),true))
%!assert(spdiag(bs,-1),sparse(diag(bf,-1),true))
%!assert(spdiag(as(:)),sparse(diag(af(:)),true))
%!assert(spdiag(as(:),1),sparse(diag(af(:),1),true))
%!assert(spdiag(as(:),-1),sparse(diag(af(:),-1),true))
%!assert(spdiag(as(:)'),sparse(diag(af(:)'),true))
%!assert(spdiag(as(:)',1),sparse(diag(af(:)',1),true))
%!assert(spdiag(as(:)',-1),sparse(diag(af(:)',-1),true))
%!assert(spdiags(as,[0,1]),[diag(af,0),diag(af,1)])
%!test [tb,tc]=spdiags(as); 
%! assert(spdiags(tb,tc,sparse(zeros(size(as)))),as)
%! assert(spdiags(tb,tc,size(as,1),size(as,2)),as)

EOF
}

# test matrix reshape operations
gen_matrixreshape_tests() {
    cat >>$TESTS <<EOF
%% Matrix diagonal tests (uses af,as,bf,bs)
%!assert(reshape(as,1,prod(size(as))),sparse(reshape(af,1,prod(size(af))),true))
%!assert(reshape(as,prod(size(as)),1),sparse(reshape(af,prod(size(af)),1),true))
%!assert(reshape(as,fliplr(size(as))),sparse(reshape(af,fliplr(size(af))),true))
%!assert(reshape(bs,1,prod(size(as))),sparse(reshape(bf,1,prod(size(af))),true))
%!assert(reshape(bs,prod(size(as)),1),sparse(reshape(bf,prod(size(af)),1),true))
%!assert(reshape(bs,fliplr(size(as))),sparse(reshape(bf,fliplr(size(af))),true))

EOF
}

# test mapper matrix operations: uses as,af
print_mapper_test() {
echo "%!assert($1(as),sparse($1(af),1))" >>$TESTS
}

print_real_mapper_test() {
    cat >>$TESTS <<EOF
%!test
%! wn2s = warning ("query", "Octave:num-to-str");
%! warning ("off", "Octave:num-to-str");
%! if isreal(af)
%!   if ($2)
%!     assert($1(as),sparse($1(af),1))
%!   else
%!     assert($1(as),$1(af))
%!   endif
%! endif
%! warning (wn2s.state, "Octave:num-to-str");

EOF
}

gen_mapper_tests() {
echo "%% Unary matrix tests (uses af,as)">>$TESTS
print_mapper_test abs
print_mapper_test acos
print_mapper_test acosh
print_mapper_test angle
print_mapper_test arg
print_mapper_test asin
print_mapper_test asinh
print_mapper_test atan
print_mapper_test atanh
print_mapper_test ceil
print_mapper_test conj
print_mapper_test cos
print_mapper_test cosh
print_mapper_test exp
print_mapper_test finite
print_mapper_test fix
print_mapper_test floor
print_mapper_test imag
print_mapper_test isinf
print_mapper_test isna
print_mapper_test isnan
print_mapper_test log
#print_mapper_test log10   ## fails with different NaN, not a problem
print_mapper_test real
print_mapper_test round
print_mapper_test sign
print_mapper_test sin
print_mapper_test sinh
print_mapper_test sqrt
print_mapper_test tan
print_mapper_test tanh

# Specific tests for certain mapper functions
    cat >>$TESTS <<EOF
%!assert(issparse(abs(as))&&isreal(abs(as)))
%!assert(issparse(real(as))&&isreal(real(as)))
%!assert(issparse(imag(as))&&isreal(imag(as)))

EOF
}

gen_real_mapper_tests() {
echo "%% Unary matrix tests (uses af,as)">>$TESTS
print_real_mapper_test erf 1
print_real_mapper_test erfc 1
#print_real_mapper_test gamma 1
print_real_mapper_test isalnum 0
print_real_mapper_test isalpha 0
print_real_mapper_test isascii 0
print_real_mapper_test iscntrl 0
print_real_mapper_test isdigit 0
print_real_mapper_test isgraph 0
print_real_mapper_test islower 0
print_real_mapper_test isprint 0
print_real_mapper_test ispunct 0
print_real_mapper_test isspace 0
print_real_mapper_test isupper 0
print_real_mapper_test isxdigit 0
#print_real_mapper_test lgamma 1

# Specific tests for certain mapper functions
    cat >>$TESTS <<EOF

%% These mapper functions always return a full matrix
%!test
%! wn2s = warning ("query", "Octave:num-to-str");
%! warning ("off", "Octave:num-to-str");
%! if isreal(af)
%!    assert(toascii(as),toascii(af))
%!    assert(tolower(as),tolower(af))
%!    assert(toupper(as),toupper(af))
%! endif
%! warning (wn2s.state, "Octave:num-to-str");

EOF
}

# test matrix operations: uses as,af
gen_unaryop_tests() {
    cat >>$TESTS <<EOF
%% Unary matrix tests (uses af,as)
%!assert(issparse(as))
%!assert(!issparse(af))
%!assert(!(issparse(af)&&iscomplex(af)))
%!assert(!(issparse(af)&&isreal(af)))
%!assert(spsum(as),sparse(sum(af),true))
%!assert(spsum(as,1),sparse(sum(af,1),true))
%!assert(spsum(as,2),sparse(sum(af,2),true))
%!assert(spcumsum(as),sparse(cumsum(af),true))
%!assert(spcumsum(as,1),sparse(cumsum(af,1),true))
%!assert(spcumsum(as,2),sparse(cumsum(af,2),true))
%!assert(spsumsq(as),sparse(sumsq(af),true))
%!assert(spsumsq(as,1),sparse(sumsq(af,1),true))
%!assert(spsumsq(as,2),sparse(sumsq(af,2),true))
%!assert(spprod(as),sparse(prod(af),true))
%!assert(spprod(as,1),sparse(prod(af,1),true))
%!assert(spprod(as,2),sparse(prod(af,2),true))
%!assert(spcumprod(as),sparse(cumprod(af),true))
%!assert(spcumprod(as,1),sparse(cumprod(af,1),true))
%!assert(spcumprod(as,2),sparse(cumprod(af,2),true))

%!assert(spmin(as),sparse(min(af),true))
%!assert(spmin(as(:)),min(af(:)))
%!assert(spmin(as,[],1),sparse(min(af,[],1),true))
%!assert(spmin(as,[],2),sparse(min(af,[],2),true))
%!assert(spmin(as,[],1),sparse(min(af,[],1),true))
%!assert(spmin(as,0),sparse(min(af,0),true))
%!assert(spmin(as,bs),sparse(min(af,bf),true))
%!assert(spmax(as),sparse(max(af),true))
%!assert(spmax(as(:)),max(af(:)))
%!assert(spmax(as,[],1),sparse(max(af,[],1),true))
%!assert(spmax(as,[],2),sparse(max(af,[],2),true))
%!assert(spmax(as,[],1),sparse(max(af,[],1),true))
%!assert(spmax(as,0),sparse(max(af,0),true))
%!assert(spmax(as,bs),sparse(max(af,bf),true))

%!assert(as==as)
%!assert(as==af)
%!assert(af==as)
%!test
%! [ii,jj,vv,nr,nc] = spfind(as);
%! assert(af,full(sparse(ii,jj,vv,nr,nc)));
%!assert(nnz(as),sum(af(:)!=0))
%!assert(nnz(as),nnz(af))
%!assert(issparse(as.'))
%!assert(issparse(as'))
%!assert(issparse(-as))
%!assert(~as,sparse(~af,true))
%!assert(as.', sparse(af.',true));
%!assert(as',  sparse(af',true));
%!assert(-as, sparse(-af,true));
%!assert(~as, sparse(~af,true));
%!error [i,j]=size(af);as(i-1,j+1);
%!error [i,j]=size(af);as(i+1,j-1);
%!test
%! [Is,Js,Vs] = spfind(as);
%! [If,Jf,Vf] = find(af);
%! assert(Is,If);
%! assert(Js,Jf);
%! assert(Vs,Vf);
%!error as(0,1);
%!error as(1,0);
%!assert(spfind(as),find(af))
%!test
%! [i,j,v] = spfind(as);
%! [m,n] = size(as);
%! x = sparse(i,j,v,m,n);
%! assert(x,as);
%!test
%! [i,j,v,m,n] = spfind(as);
%! x = sparse(i,j,v,m,n);
%! assert(x,as);
%!assert(issparse(horzcat(as,as)));
%!assert(issparse(vertcat(as,as)));
%!assert(issparse(cat(1,as,as)));
%!assert(issparse(cat(2,as,as)));
%!assert(issparse([as,as]));
%!assert(issparse([as;as]));
%!assert(horzcat(as,as), sparse([af,af]));
%!assert(vertcat(as,as), sparse([af;af]));
%!assert(horzcat(as,as,as), sparse([af,af,af]));
%!assert(vertcat(as,as,as), sparse([af;af;af]));
%!assert([as,as], sparse([af,af]));
%!assert([as;as], sparse([af;af]));
%!assert([as,as,as], sparse([af,af,af]));
%!assert([as;as;as], sparse([af;af;af]));
%!assert(cat(2,as,as), sparse([af,af]));
%!assert(cat(1,as,as), sparse([af;af]));
%!assert(cat(2,as,as,as), sparse([af,af,af]));
%!assert(cat(1,as,as,as), sparse([af;af;af]));
%!assert(issparse([as,af]));
%!assert(issparse([af,as]));
%!assert([as,af], sparse([af,af]));
%!assert([as;af], sparse([af;af]));

EOF
}

# operations which require square matrices.
gen_square_tests() {
# The \ and / operator tests on square matrices
    gen_square_divop_tests

    cat >>$TESTS <<EOF
%!assert(spdet(bs+speye(size(bs))),det(bf+eye(size(bf))),100*eps*abs(det(bf+eye(size(bf)))))

%!test 
%! [l,u]=splu(sparse([1,1;1,1]));
%! assert(l*u,[1,1;1,1],10*eps);

%!test
%! [l,u]=splu(sparse([1,1;1,1+i]));
%! assert(l,sparse([1,2,2],[1,1,2],1),10*eps);
%! assert(u,sparse([1,1,2],[1,2,2],[1,1,1i]),10*eps);

%!test ;# permuted LU
%! [L,U] = splu(bs);
%! assert(L*U,bs,1e-10);

%!test ;# simple LU + row permutations
%! [L,U,P] = splu(bs);
%! assert(P'*L*U,bs,1e-10);
%! # triangularity
%! [i,j,v]=spfind(L);
%! assert(i-j>=0);
%! [i,j,v]=spfind(U);
%! assert(j-i>=0);

%!test ;# simple LU + row/col permutations
%! [L,U,P,Q] = splu(bs);
%! assert(P'*L*U*Q',bs,1e-10);
%! # triangularity
%! [i,j,v]=spfind(L);
%! assert(i-j>=0);
%! [i,j,v]=spfind(U);
%! assert(j-i>=0);

%!test ;# LU with fixed column permutation
%! [L,U,P] = splu(bs,colamd(bs));
%! assert(P'*L*U,bs,1e-10);
%! # triangularity
%! [i,j,v]=spfind(L);
%! assert(i-j>=0);
%! [i,j,v]=spfind(U(:,colamd(bs)));
%! assert(j-i>=0);

%!test ;# LU with initial column permutation
%! [L,U,P,Q] = splu(bs,colamd(bs));
%! assert(P'*L*U*Q',bs,1e-10);
%! # triangularity
%! [i,j,v]=spfind(L);
%! assert(i-j>=0);
%! [i,j,v]=spfind(U);
%! assert(j-i>=0);

%!test ;# inverse
%! assert(spinv(bs)*bs,sparse(eye(rows(bs))),1e-10);

%!assert(bf\as',bf\af',100*eps);
%!assert(bs\af',bf\af',100*eps);
%!assert(bs\as',sparse(bf\af'),100*eps);

EOF
}

# Cholesky tests
gen_cholesky_tests() {
    cat >>$TESTS <<EOF
%!assert(spchol(bs)'*spchol(bs),bs,1e-10);
%!assert(splchol(bs)*splchol(bs)',bs,1e-10);
%!assert(splchol(bs),spchol(bs)',1e-10);

%!test ;# Return Partial Cholesky factorization
%! [RS,PS] = spchol(bs);
%! assert(RS'*RS,bs,1e-10);
%! assert(PS,0);
%! [LS,PS] = splchol(bs);
%! assert(LS*LS',bs,1e-10);
%! assert(PS,0);

%!test ;# Permuted Cholesky factorization
%! [RS,PS,QS] = spchol(bs);
%! assert(RS'*RS,QS*bs*QS',1e-10);
%! assert(PS,0);
%! [LS,PS,QS] = splchol(bs);
%! assert(LS*LS',QS*bs*QS',1e-10);
%! assert(PS,0);

EOF
}

# test scalar operations: uses af and real scalar bf; modifies as,bf,bs
gen_scalar_tests() {
    echo '%!test as=sparse(af);' >> $TESTS
    echo '%!test bs=bf;' >> $TESTS
    gen_elementop_tests
    gen_ordering_tests
    echo '%!test bf=bf+1i;' >>$TESTS
    echo '%!test bs=bf;' >> $TESTS
    gen_elementop_tests
}

# test matrix operations: uses af and bf; modifies as,bs
gen_rectangular_tests() {
    echo '%!test as=sparse(af);' >> $TESTS
    echo '%!test bs=sparse(bf);' >>$TESTS
    gen_mapper_tests
    gen_real_mapper_tests
    gen_unaryop_tests
    gen_elementop_tests
    gen_sparsesparse_elementop_tests
    gen_matrixop_tests
    # gen_divop_tests # Disable rectangular \ and / for now
    gen_matrixdiag_tests
    gen_matrixreshape_tests
}


# =======================================================
# sparse assembly tests

gen_assembly_tests() {
cat >>$TESTS <<EOF
%%Assembly tests
%!test
%! m=max([m;r(:)]);
%! n=max([n;c(:)]);
%! funiq=fsum=zeros(m,n);
%! funiq(r(:) + m*(c(:)-1) ) = ones(size(r(:)));
%! funiq = sparse(funiq);
%! for k=1:length(r), fsum(r(k),c(k)) += 1; end
%! fsum = sparse(fsum);
%!assert(sparse(r,c,1),sparse(fsum(1:max(r),1:max(c))));
%!assert(sparse(r,c,1,"sum"),sparse(fsum(1:max(r),1:max(c))));
%!assert(sparse(r,c,1,"unique"),sparse(funiq(1:max(r),1:max(c))));
%!assert(sparse(r,c,1,m,n),sparse(fsum));
%!assert(sparse(r,c,1,m,n,"sum"),sparse(fsum));
%!assert(sparse(r,c,1,m,n,"unique"),sparse(funiq));

%!assert(sparse(r,c,1i),sparse(fsum(1:max(r),1:max(c))*1i));
%!assert(sparse(r,c,1i,"sum"),sparse(fsum(1:max(r),1:max(c))*1i));
%!assert(sparse(r,c,1i,"unique"),sparse(funiq(1:max(r),1:max(c))*1i));
%!assert(sparse(r,c,1i,m,n),sparse(fsum*1i));
%!assert(sparse(r,c,1i,m,n,"sum"),sparse(fsum*1i));
%!assert(sparse(r,c,1i,m,n,"unique"),sparse(funiq*1i));

%!test
%! if (issparse(funiq))
%!  assert(sparse(full(1i*funiq)),sparse(1i*funiq));
%! endif

%!assert(sparse(full(funiq)),funiq);


EOF
}

# =======================================================
# sparse selection tests

gen_select_tests() {
    cat >>$TESTS <<EOF
%!test as=sparse(af);

%% Point tests
%!test idx=ridx(:)+rows(as)*(cidx(:)-1);
%!assert(sparse(as(idx),true),sparse(af(idx),true));
%!assert(as(idx),sparse(af(idx),true));
%!assert(as(idx'),sparse(af(idx'),true));
%!assert(as(flipud(idx(:))),sparse(af(flipud(idx(:))),true))
%!assert(as([idx,idx]),sparse(af([idx,idx]),true));
%!error(as(reshape([idx;idx],[1,length(idx),2])));

%% Slice tests
%!assert(as(ridx,cidx), sparse(af(ridx,cidx),true))
%!assert(as(ridx,:), sparse(af(ridx,:),true))
%!assert(as(:,cidx), sparse(af(:,cidx),true))
%!assert(as(:,:), sparse(af(:,:),true))
%!assert(as((size(as,1):-1:1),:),sparse(af((size(af,1):-1:1),:),true))
%!assert(as(:,(size(as,2):-1:1)),sparse(af(:,(size(af,2):-1:1)),true))

%% Assignment test
%!test
%! ts=as;ts(:,:)=ts(fliplr(1:size(as,1)),:);
%! tf=af;tf(:,:)=tf(fliplr(1:size(af,1)),:);
%! assert(ts,sparse(tf,true));
%!test
%! ts=as;ts(fliplr(1:size(as,1)),:)=ts;
%! tf=af;tf(fliplr(1:size(af,1)),:)=tf;
%! assert(ts,sparse(tf,true));
%!test
%! ts=as;ts(:,fliplr(1:size(as,2)))=ts;
%! tf=af;tf(:,fliplr(1:size(af,2)))=tf;
%! assert(ts,sparse(tf,true));
%!test
%! ts(fliplr(1:size(as,1)))=as(:,1);tf(fliplr(1:size(af,1)))=af(:,1);
%! assert(ts,sparse(tf,true));

%% Deletion tests
%!test
%! ts=as;ts(1,:)=[];tf=af;tf(1,:)=[];
%! assert(ts,sparse(tf,true));
%!test
%! ts=as;ts(:,1)=[];tf=af;tf(:,1)=[];
%! assert(ts,sparse(tf,true));

%% Test 'end' keyword
%!assert(as(end),af(end))
%!assert(as(1,end), af(1,end))
%!assert(as(end,1), af(end,1))
%!assert(as(end,end), af(end,end))
%!assert(as(2:end,2:end), sparse(af(2:end,2:end),true))
%!assert(as(1:end-1,1:end-1), sparse(af(1:end-1,1:end-1),true))
EOF
}

# =======================================================
# sparse save and load tests

gen_save_tests() {
    cat >>$TESTS <<EOF
%!test # save ascii
%! savefile= tmpnam();
%! as_save=as; save("-text",savefile,"bf","as_save","af");
%! clear as_save;
%! load(savefile,"as_save");
%! unlink(savefile);
%! assert(as_save,sparse(af));
%!test # save binary
%! savefile= tmpnam();
%! as_save=as; save("-binary",savefile,"bf","as_save","af");
%! clear as_save;
%! load(savefile,"as_save");
%! unlink(savefile);
%! assert(as_save,sparse(af));
%!test # save hdf5
%! savefile= tmpnam();
%! as_save=as; save("-hdf5",savefile,"bf","as_save","af");
%! clear as_save;
%! load(savefile,"as_save");
%! unlink(savefile);
%! assert(as_save,sparse(af));
%!test # save matlab
%! savefile= tmpnam();
%! as_save=as; save("-mat",savefile,"bf","as_save","af");
%! clear as_save;
%! load(savefile,"as_save");
%! unlink(savefile);
%! assert(as_save,sparse(af));
EOF
}

# =============================================================
# Specific solver tests for matrices that will test all of the solver
# code. Uses alpha and beta
gen_solver_tests() {

if $preset; then
  cat >>$TESTS <<EOF
%! n=8;
%! lf=diag(1:n);lf(n-1,1)=0.5*alpha;lf(n,2)=0.25*alpha;ls=sparse(lf);
%! uf=diag(1:n);uf(1,n-1)=2*alpha;uf(2,n)=alpha;us=sparse(uf);
%! ts=spdiags(ones(n,3),-1:1,n,n)+diag(1:n); tf = full(ts);
EOF
else
  cat >>$TESTS <<EOF
%! n=floor(lognormal_rnd(8,2)+1)'
%! ls = tril(sprandn(8,8,0.2),-1).*alpha + n*speye(8); lf = full(ls);
%! us = triu(sprandn(8,8,0.2),1).*alpha + n*speye(8); uf = full(us);
%! ts = spdiags(randn(8,3),-1:1,8,8).*alpha; tf = full(ts);
EOF
fi

cat >>$TESTS <<EOF
%! df = diag(1:n).* alpha; ds = sparse(df);
%! pdf = df(randperm(n),randperm(n)); pds = sparse(pdf);
%! plf = lf(randperm(n),randperm(n)); pls = sparse(plf);
%! puf = uf(randperm(n),randperm(n)); pus = sparse(puf);
%! bs = spdiags(repmat([1:n]',1,4),-2:1,n,n).*alpha; bf = full(bs);
%! cf = lf + lf'; cs = sparse(cf);
%! bcf = bf + bf'; bcs = sparse(bcf);
%! tcf = tf + tf'; tcs = sparse(tcf);
%! xf = diag(1:n) + fliplr(diag(1:n)).*beta; xs = sparse(xf);
%!assert(ds\xf,df\xf,1e-10);
%!assert(ds\xs,sparse(df\xf,1),1e-10);
%!assert(pds\xf,pdf\xf,1e-10);
%!assert(pds\xs,sparse(pdf\xf,1),1e-10);
%!assert(ls\xf,lf\xf,1e-10);
%!assert(sparse(ls\xs),sparse(lf\xf),1e-10);
%!assert(pls\xf,plf\xf,1e-10);
%!assert(sparse(pls\xs),sparse(plf\xf),1e-10);
%!assert(us\xf,uf\xf,1e-10);
%!assert(sparse(us\xs),sparse(uf\xf),1e-10);
%!assert(pus\xf,puf\xf,1e-10);
%!assert(sparse(pus\xs),sparse(puf\xf),1e-10);
%!assert(bs\xf,bf\xf,1e-10);
%!assert(sparse(bs\xs),sparse(bf\xf),1e-10);
%!assert(cs\xf,cf\xf,1e-10);
%!assert(sparse(cs\xs),sparse(cf\xf),1e-10);
%!assert(bcs\xf,bcf\xf,1e-10);
%!assert(sparse(bcs\xs),sparse(bcf\xf),1e-10);
%!assert(ts\xf,tf\xf,1e-10);
%!assert(sparse(ts\xs),sparse(tf\xf),1e-10);
%!assert(tcs\xf,tcf\xf,1e-10);
%!assert(sparse(tcs\xs),sparse(tcf\xf),1e-10);

EOF

cat >>$TESTS <<EOF
%% QR solver tests

%!function f(a, sz, feps)
%! b = randn(sz); x = a \b; 
%! assert (a * x, b, feps);
%! b = randn(sz)+1i*randn(sz); x = a \ b;  
%! assert (a * x, b, feps);
%! b = sprandn(sz(1),sz(2),0.2); x = a \b;
%! assert (sparse(a * x), b, feps);
%! b = sprandn(sz(1),sz(2),0.2)+1i*sprandn(sz(1),sz(2),0.2); x = a \b; 
%! assert (sparse(a * x), b, feps);
%!test
%! a = alpha*sprandn(10,11,0.2)+speye(10,11); f(a,[10,2],1e-10);
%! ## Test this by forcing matrix_type, as can't get a certain 
%! ## result for over-determined systems.
%! a = alpha*sprandn(10,10,0.2)+speye(10,10); matrix_type(a, "Singular");
%! f(a,[10,2],1e-10);

%% Rectanguar solver tests that don't use QR

%!test
%! ds = alpha * spdiags([1:11]',0,10,11);
%! df = full(ds);
%! xf = beta * ones(10,2);
%! xs = speye(10,10);
%!assert(ds\xf,df\xf,100*eps)
%!assert(ds\xs,sparse(df\xs,true),100*eps)
%!test
%! pds = ds([2,1,3:10],:);
%! pdf = full(pds);
%!assert(pds\xf,pdf\xf,100*eps)
%!assert(pds\xs,sparse(pdf\xs,true),100*eps)
%!test
%! ds = alpha * spdiags([1:11]',0,11,10);
%! df = full(ds);
%! xf = beta * ones(11,2);
%! xs = speye(11,11);
%!assert(ds\xf,df\xf,100*eps)
%!assert(ds\xs,sparse(df\xs,true),100*eps)
%!test
%! pds = ds([2,1,3:11],:);
%! pdf = full(pds);
%!assert(pds\xf,pdf\xf,100*eps)
%!assert(pds\xs,sparse(pdf\xs,true),100*eps)
%!test
%! us = alpha*[[speye(10,10);sparse(1,10)],[[1,1];sparse(9,2);[1,1]]];
%!assert(us*(us\xf),xf,100*eps)
%!assert(us*(us\xs),xs,100*eps)
%!test
%! pus = us(:,[2,1,3:12]);
%!assert(pus*(pus\xf),xf,100*eps)
%!assert(pus*(pus\xs),xs,100*eps)
%!test
%! us = alpha*[speye(11,9),[1;sparse(8,1);1;0]];
%!test
%! [c,r] = spqr (us, xf);
%! assert(us\xf,r\c,100*eps)
%!test
%! [c,r] = spqr (us, xs);
%! r = matrix_type(r,"Singular"); ## Force Matrix Type
%! assert(us\xs,r\c,100*eps)
%!test
%! pus = us(:,[1:8,10,9]);
%!test
%! [c,r] = spqr (pus, xf);
%! r = matrix_type(r,"Singular"); ## Force Matrix Type
%! assert(pus\xf,r\c,100*eps)
%!test
%! [c,r] = spqr (pus, xs);
%! r = matrix_type(r,"Singular"); ## Force Matrix Type
%! assert(pus\xs,r\c,100*eps)
%!test
%! ls = alpha*[speye(9,11);[1,sparse(1,8),1,0]];
%! xf = beta * ones(10,2);
%! xs = speye(10,10);
%!assert(ls*(ls\xf),xf,100*eps)
%!assert(ls*(ls\xs),xs,100*eps)
%!test
%! pls = ls([1:8,10,9],:);
%!assert(pls*(pls\xf),xf,100*eps)
%!assert(pls*(pls\xs),xs,100*eps)
%!test
%! ls = alpha*[speye(10,10),sparse(10,1);[1;1],sparse(2,9),[1;1]];
%! xf = beta * ones(12,2);
%! xs = speye(12,12);
%!test
%! [c,r] = spqr (ls, xf);
%! assert(ls\xf,r\c,100*eps)
%!test
%! [c,r] = spqr (ls, xs);
%! r = matrix_type(r,"Singular"); ## Force Matrix Type
%! assert(ls\xs,r\c,100*eps)
%!test
%! pls = ls(:,[1:8,10,9]);
%!test
%! [c,r] = spqr (pls, xf);
%! r = matrix_type(r,"Singular"); ## Force Matrix Type
%! assert(pls\xf,r\c,100*eps)
%!test
%! [c,r] = spqr (pls, xs);
%! r = matrix_type(r,"Singular"); ## Force Matrix Type
%! assert(pls\xs,r\c,100*eps)

EOF
}


# =============================================================
# Putting it all together: defining the combined tests


# initial function
gen_function
gen_section

# specific tests
if $preset; then 
    gen_specific_tests
    gen_section
fi

# scalar operations
echo '%!shared as,af,bs,bf' >> $TESTS
if $preset; then
    echo '%!test af=[1+1i,2-1i,0,0;0,0,0,3+2i;0,0,0,4];' >> $TESTS
    echo '%!test bf=3;' >>$TESTS
else
    cat >>$TESTS <<EOF
%!test
%! % generate m,n from 1 to <5000
%! m=floor(lognormal_rnd(8,2)+1);
%! n=floor(lognormal_rnd(8,2)+1);
%! as=sprandn(m,n,0.3); af = full(as+1i*sprandn(as));
%! bf = randn;
EOF
fi

gen_scalar_tests
gen_section

# rectangular operations
if $preset; then
    echo '%!test af=[1+1i,2-1i,0,0;0,0,0,3+2i;0,0,0,4];' >> $TESTS
    echo '%!test bf=[0,1-1i,0,0;2+1i,0,0,0;3-1i,2+3i,0,0];' >> $TESTS
else
    cat >>$TESTS <<EOF
%!test
%! m=floor(lognormal_rnd(8,2)+1);
%! n=floor(lognormal_rnd(8,2)+1);
%! as=sprandn(m,n,0.3); af = full(as+1i*sprandn(as));
%! bs=sprandn(m,n,0.3); bf = full(bs+1i*sprandn(bs));
EOF
fi

gen_rectangular_tests
gen_section
gen_save_tests
gen_section
echo '%!test bf=real(bf);' >> $TESTS
gen_rectangular_tests
gen_section
gen_sparsesparse_ordering_tests
gen_section
echo '%!test af=real(af);' >> $TESTS
gen_rectangular_tests
gen_section
gen_save_tests
gen_section
echo '%!test bf=bf+1i*(bf~=0);' >> $TESTS
gen_rectangular_tests
gen_section

# square operations
if $preset; then
    echo '%!test af=[1+1i,2-1i,0,0;0,0,0,3+2i;0,0,0,4];' >> $TESTS
    echo '%! as=sparse(af);' >> $TESTS
    echo '%!test bf=[0,1-1i,0,0;2+1i,0,0,0;3-1i,2+3i,0,0];' >> $TESTS
else
    cat >>$TESTS <<EOF
%!test
%! m=floor(lognormal_rnd(8,2)+1);
%! n=floor(lognormal_rnd(8,2)+1);
%! as=sprandn(m,n,0.3); af = full(as+1i*sprandn(as));
%! bs=sprandn(m,n,0.3); bf = full(bs+1i*sprandn(bs));
EOF
fi

cat >>$TESTS <<EOF
%!test ;# invertible matrix
%! bf=af'*bf+max(abs([af(:);bf(:)]))*sparse(eye(columns(as)));
%! bs=sparse(bf);

EOF

gen_square_tests
gen_section
echo '%!test bf=real(bf);' >> $TESTS
echo '%! bs=sparse(bf);' >> $TESTS
gen_square_tests
gen_section
echo '%!test af=real(af);' >> $TESTS
echo '%! as=sparse(af);' >> $TESTS
gen_square_tests
gen_section
echo '%!test bf=bf+1i*(bf~=0);' >> $TESTS
echo '%! bs=sparse(bf);' >> $TESTS
gen_square_tests
gen_section

# cholesky tests
if $preset; then
  echo '%!test bf=[5,0,1+1i,0;0,5,0,1-2i;1-1i,0,5,0;0,1+2i,0,5];' >> $TESTS
  echo '%! bs=sparse(bf);' >> $TESTS
else
  echo '# This has a small chance of failing to create a positive definite matrix' >> $TESTS
  echo '%!test n=floor(lognormal_rnd(8,2)+1)' >> $TESTS
  echo '%! bs = n*speye(n,n) + sprandn(n,n,0.3); bf = full(bs);' >> $TESTS
fi

gen_cholesky_tests
gen_section
echo '%!test bf=real(bf);' >> $TESTS
echo '%! bs=sparse(bf);' >> $TESTS
gen_cholesky_tests
gen_section

# assembly tests
echo '%!shared r,c,m,n,fsum,funiq' >>$TESTS
if $use_preset; then
    cat >>$TESTS <<EOF
%!test
%! r=[1,1,2,1,2,3];
%! c=[2,1,1,1,2,1];
%! m=n=0;
EOF
else
    cat >>$TESTS <<EOF
%!test
%! % generate m,n from 1 to <5000
%! m=floor(lognormal_rnd(8,2)+1);
%! n=floor(lognormal_rnd(8,2)+1);
%! nz=ceil((m+n)/2);
%! r=floor(rand(5,nz)*n)+1;
%! c=floor(rand(5,nn)*m)+1;
EOF
fi
gen_assembly_tests #includes real and complex tests
gen_section

# slicing tests
echo '%!shared ridx,cidx,idx,as,af' >>$TESTS
if $use_preset; then
    cat >>$TESTS <<EOF
%!test
%! af=[1+1i,2-1i,0,0;0,0,0,3+2i;0,0,0,4];
%! ridx=[1,3]; cidx=[2,3];
EOF
else
    cat >>$TESTS <<EOF
%!test
%! % generate m,n from 1 to <5000
%! m=floor(lognormal_rnd(8,2)+1);
%! n=floor(lognormal_rnd(8,2)+1);
%! as=sprandn(m,n,0.3); af = full(as+1i*sprandn(as));
%! ridx = ceil(m*rand(1,ceil(rand*m))
%! cidx = ceil(n*rand(1,ceil(rand*n))
EOF
fi
gen_select_tests
echo '%!test af=real(af);' >> $TESTS
gen_select_tests
gen_section
echo '%!shared alpha,beta,df,pdf,lf,plf,uf,puf,bf,cf,bcf,tf,tcf,xf,ds,pds,ls,pls,us,pus,bs,cs,bcs,ts,tcs,xs' >>$TESTS
echo '%!test alpha=1;beta=1;' >> $TESTS
gen_solver_tests
echo '%!test alpha=1;beta=1i;' >> $TESTS
gen_solver_tests
echo '%!test alpha=1i;beta=1;' >> $TESTS
gen_solver_tests
echo '%!test alpha=1i;beta=1i;' >> $TESTS
gen_solver_tests
gen_section
