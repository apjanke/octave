      SUBROUTINE ZTRSYL( TRANA, TRANB, ISGN, M, N, A, LDA, B, LDB, C,
     $                   LDC, SCALE, INFO )
*
*  -- LAPACK routine (version 3.0) --
*     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
*     Courant Institute, Argonne National Lab, and Rice University
*     June 30, 1999
*
*     .. Scalar Arguments ..
      CHARACTER          TRANA, TRANB
      INTEGER            INFO, ISGN, LDA, LDB, LDC, M, N
      DOUBLE PRECISION   SCALE
*     ..
*     .. Array Arguments ..
      COMPLEX*16         A( LDA, * ), B( LDB, * ), C( LDC, * )
*     ..
*
*  Purpose
*  =======
*
*  ZTRSYL solves the complex Sylvester matrix equation:
*
*     op(A)*X + X*op(B) = scale*C or
*     op(A)*X - X*op(B) = scale*C,
*
*  where op(A) = A or A**H, and A and B are both upper triangular. A is
*  M-by-M and B is N-by-N; the right hand side C and the solution X are
*  M-by-N; and scale is an output scale factor, set <= 1 to avoid
*  overflow in X.
*
*  Arguments
*  =========
*
*  TRANA   (input) CHARACTER*1
*          Specifies the option op(A):
*          = 'N': op(A) = A    (No transpose)
*          = 'C': op(A) = A**H (Conjugate transpose)
*
*  TRANB   (input) CHARACTER*1
*          Specifies the option op(B):
*          = 'N': op(B) = B    (No transpose)
*          = 'C': op(B) = B**H (Conjugate transpose)
*
*  ISGN    (input) INTEGER
*          Specifies the sign in the equation:
*          = +1: solve op(A)*X + X*op(B) = scale*C
*          = -1: solve op(A)*X - X*op(B) = scale*C
*
*  M       (input) INTEGER
*          The order of the matrix A, and the number of rows in the
*          matrices X and C. M >= 0.
*
*  N       (input) INTEGER
*          The order of the matrix B, and the number of columns in the
*          matrices X and C. N >= 0.
*
*  A       (input) COMPLEX*16 array, dimension (LDA,M)
*          The upper triangular matrix A.
*
*  LDA     (input) INTEGER
*          The leading dimension of the array A. LDA >= max(1,M).
*
*  B       (input) COMPLEX*16 array, dimension (LDB,N)
*          The upper triangular matrix B.
*
*  LDB     (input) INTEGER
*          The leading dimension of the array B. LDB >= max(1,N).
*
*  C       (input/output) COMPLEX*16 array, dimension (LDC,N)
*          On entry, the M-by-N right hand side matrix C.
*          On exit, C is overwritten by the solution matrix X.
*
*  LDC     (input) INTEGER
*          The leading dimension of the array C. LDC >= max(1,M)
*
*  SCALE   (output) DOUBLE PRECISION
*          The scale factor, scale, set <= 1 to avoid overflow in X.
*
*  INFO    (output) INTEGER
*          = 0: successful exit
*          < 0: if INFO = -i, the i-th argument had an illegal value
*          = 1: A and B have common or very close eigenvalues; perturbed
*               values were used to solve the equation (but the matrices
*               A and B are unchanged).
*
*  =====================================================================
*
*     .. Parameters ..
      DOUBLE PRECISION   ONE
      PARAMETER          ( ONE = 1.0D+0 )
*     ..
*     .. Local Scalars ..
      LOGICAL            NOTRNA, NOTRNB
      INTEGER            J, K, L
      DOUBLE PRECISION   BIGNUM, DA11, DB, EPS, SCALOC, SGN, SMIN,
     $                   SMLNUM
      COMPLEX*16         A11, SUML, SUMR, VEC, X11
*     ..
*     .. Local Arrays ..
      DOUBLE PRECISION   DUM( 1 )
*     ..
*     .. External Functions ..
      LOGICAL            LSAME
      DOUBLE PRECISION   DLAMCH, ZLANGE
      COMPLEX*16         ZDOTC, ZDOTU, ZLADIV
      EXTERNAL           LSAME, DLAMCH, ZLANGE, ZDOTC, ZDOTU, ZLADIV
*     ..
*     .. External Subroutines ..
      EXTERNAL           XERBLA, ZDSCAL
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC          ABS, DBLE, DCMPLX, DCONJG, DIMAG, MAX, MIN
*     ..
*     .. Executable Statements ..
*
*     Decode and Test input parameters
*
      NOTRNA = LSAME( TRANA, 'N' )
      NOTRNB = LSAME( TRANB, 'N' )
*
      INFO = 0
      IF( .NOT.NOTRNA .AND. .NOT.LSAME( TRANA, 'T' ) .AND. .NOT.
     $    LSAME( TRANA, 'C' ) ) THEN
         INFO = -1
      ELSE IF( .NOT.NOTRNB .AND. .NOT.LSAME( TRANB, 'T' ) .AND. .NOT.
     $         LSAME( TRANB, 'C' ) ) THEN
         INFO = -2
      ELSE IF( ISGN.NE.1 .AND. ISGN.NE.-1 ) THEN
         INFO = -3
      ELSE IF( M.LT.0 ) THEN
         INFO = -4
      ELSE IF( N.LT.0 ) THEN
         INFO = -5
      ELSE IF( LDA.LT.MAX( 1, M ) ) THEN
         INFO = -7
      ELSE IF( LDB.LT.MAX( 1, N ) ) THEN
         INFO = -9
      ELSE IF( LDC.LT.MAX( 1, M ) ) THEN
         INFO = -11
      END IF
      IF( INFO.NE.0 ) THEN
         CALL XERBLA( 'ZTRSYL', -INFO )
         RETURN
      END IF
*
*     Quick return if possible
*
      IF( M.EQ.0 .OR. N.EQ.0 )
     $   RETURN
*
*     Set constants to control overflow
*
      EPS = DLAMCH( 'P' )
      SMLNUM = DLAMCH( 'S' )
      BIGNUM = ONE / SMLNUM
      CALL DLABAD( SMLNUM, BIGNUM )
      SMLNUM = SMLNUM*DBLE( M*N ) / EPS
      BIGNUM = ONE / SMLNUM
      SMIN = MAX( SMLNUM, EPS*ZLANGE( 'M', M, M, A, LDA, DUM ),
     $       EPS*ZLANGE( 'M', N, N, B, LDB, DUM ) )
      SCALE = ONE
      SGN = ISGN
*
      IF( NOTRNA .AND. NOTRNB ) THEN
*
*        Solve    A*X + ISGN*X*B = scale*C.
*
*        The (K,L)th block of X is determined starting from
*        bottom-left corner column by column by
*
*            A(K,K)*X(K,L) + ISGN*X(K,L)*B(L,L) = C(K,L) - R(K,L)
*
*        Where
*                    M                        L-1
*          R(K,L) = SUM [A(K,I)*X(I,L)] +ISGN*SUM [X(K,J)*B(J,L)].
*                  I=K+1                      J=1
*
         DO 30 L = 1, N
            DO 20 K = M, 1, -1
*
               SUML = ZDOTU( M-K, A( K, MIN( K+1, M ) ), LDA,
     $                C( MIN( K+1, M ), L ), 1 )
               SUMR = ZDOTU( L-1, C( K, 1 ), LDC, B( 1, L ), 1 )
               VEC = C( K, L ) - ( SUML+SGN*SUMR )
*
               SCALOC = ONE
               A11 = A( K, K ) + SGN*B( L, L )
               DA11 = ABS( DBLE( A11 ) ) + ABS( DIMAG( A11 ) )
               IF( DA11.LE.SMIN ) THEN
                  A11 = SMIN
                  DA11 = SMIN
                  INFO = 1
               END IF
               DB = ABS( DBLE( VEC ) ) + ABS( DIMAG( VEC ) )
               IF( DA11.LT.ONE .AND. DB.GT.ONE ) THEN
                  IF( DB.GT.BIGNUM*DA11 )
     $               SCALOC = ONE / DB
               END IF
               X11 = ZLADIV( VEC*DCMPLX( SCALOC ), A11 )
*
               IF( SCALOC.NE.ONE ) THEN
                  DO 10 J = 1, N
                     CALL ZDSCAL( M, SCALOC, C( 1, J ), 1 )
   10             CONTINUE
                  SCALE = SCALE*SCALOC
               END IF
               C( K, L ) = X11
*
   20       CONTINUE
   30    CONTINUE
*
      ELSE IF( .NOT.NOTRNA .AND. NOTRNB ) THEN
*
*        Solve    A' *X + ISGN*X*B = scale*C.
*
*        The (K,L)th block of X is determined starting from
*        upper-left corner column by column by
*
*            A'(K,K)*X(K,L) + ISGN*X(K,L)*B(L,L) = C(K,L) - R(K,L)
*
*        Where
*                   K-1                         L-1
*          R(K,L) = SUM [A'(I,K)*X(I,L)] + ISGN*SUM [X(K,J)*B(J,L)]
*                   I=1                         J=1
*
         DO 60 L = 1, N
            DO 50 K = 1, M
*
               SUML = ZDOTC( K-1, A( 1, K ), 1, C( 1, L ), 1 )
               SUMR = ZDOTU( L-1, C( K, 1 ), LDC, B( 1, L ), 1 )
               VEC = C( K, L ) - ( SUML+SGN*SUMR )
*
               SCALOC = ONE
               A11 = DCONJG( A( K, K ) ) + SGN*B( L, L )
               DA11 = ABS( DBLE( A11 ) ) + ABS( DIMAG( A11 ) )
               IF( DA11.LE.SMIN ) THEN
                  A11 = SMIN
                  DA11 = SMIN
                  INFO = 1
               END IF
               DB = ABS( DBLE( VEC ) ) + ABS( DIMAG( VEC ) )
               IF( DA11.LT.ONE .AND. DB.GT.ONE ) THEN
                  IF( DB.GT.BIGNUM*DA11 )
     $               SCALOC = ONE / DB
               END IF
*
               X11 = ZLADIV( VEC*DCMPLX( SCALOC ), A11 )
*
               IF( SCALOC.NE.ONE ) THEN
                  DO 40 J = 1, N
                     CALL ZDSCAL( M, SCALOC, C( 1, J ), 1 )
   40             CONTINUE
                  SCALE = SCALE*SCALOC
               END IF
               C( K, L ) = X11
*
   50       CONTINUE
   60    CONTINUE
*
      ELSE IF( .NOT.NOTRNA .AND. .NOT.NOTRNB ) THEN
*
*        Solve    A'*X + ISGN*X*B' = C.
*
*        The (K,L)th block of X is determined starting from
*        upper-right corner column by column by
*
*            A'(K,K)*X(K,L) + ISGN*X(K,L)*B'(L,L) = C(K,L) - R(K,L)
*
*        Where
*                    K-1
*           R(K,L) = SUM [A'(I,K)*X(I,L)] +
*                    I=1
*                           N
*                     ISGN*SUM [X(K,J)*B'(L,J)].
*                          J=L+1
*
         DO 90 L = N, 1, -1
            DO 80 K = 1, M
*
               SUML = ZDOTC( K-1, A( 1, K ), 1, C( 1, L ), 1 )
               SUMR = ZDOTC( N-L, C( K, MIN( L+1, N ) ), LDC,
     $                B( L, MIN( L+1, N ) ), LDB )
               VEC = C( K, L ) - ( SUML+SGN*DCONJG( SUMR ) )
*
               SCALOC = ONE
               A11 = DCONJG( A( K, K )+SGN*B( L, L ) )
               DA11 = ABS( DBLE( A11 ) ) + ABS( DIMAG( A11 ) )
               IF( DA11.LE.SMIN ) THEN
                  A11 = SMIN
                  DA11 = SMIN
                  INFO = 1
               END IF
               DB = ABS( DBLE( VEC ) ) + ABS( DIMAG( VEC ) )
               IF( DA11.LT.ONE .AND. DB.GT.ONE ) THEN
                  IF( DB.GT.BIGNUM*DA11 )
     $               SCALOC = ONE / DB
               END IF
*
               X11 = ZLADIV( VEC*DCMPLX( SCALOC ), A11 )
*
               IF( SCALOC.NE.ONE ) THEN
                  DO 70 J = 1, N
                     CALL ZDSCAL( M, SCALOC, C( 1, J ), 1 )
   70             CONTINUE
                  SCALE = SCALE*SCALOC
               END IF
               C( K, L ) = X11
*
   80       CONTINUE
   90    CONTINUE
*
      ELSE IF( NOTRNA .AND. .NOT.NOTRNB ) THEN
*
*        Solve    A*X + ISGN*X*B' = C.
*
*        The (K,L)th block of X is determined starting from
*        bottom-left corner column by column by
*
*           A(K,K)*X(K,L) + ISGN*X(K,L)*B'(L,L) = C(K,L) - R(K,L)
*
*        Where
*                    M                          N
*          R(K,L) = SUM [A(K,I)*X(I,L)] + ISGN*SUM [X(K,J)*B'(L,J)]
*                  I=K+1                      J=L+1
*
         DO 120 L = N, 1, -1
            DO 110 K = M, 1, -1
*
               SUML = ZDOTU( M-K, A( K, MIN( K+1, M ) ), LDA,
     $                C( MIN( K+1, M ), L ), 1 )
               SUMR = ZDOTC( N-L, C( K, MIN( L+1, N ) ), LDC,
     $                B( L, MIN( L+1, N ) ), LDB )
               VEC = C( K, L ) - ( SUML+SGN*DCONJG( SUMR ) )
*
               SCALOC = ONE
               A11 = A( K, K ) + SGN*DCONJG( B( L, L ) )
               DA11 = ABS( DBLE( A11 ) ) + ABS( DIMAG( A11 ) )
               IF( DA11.LE.SMIN ) THEN
                  A11 = SMIN
                  DA11 = SMIN
                  INFO = 1
               END IF
               DB = ABS( DBLE( VEC ) ) + ABS( DIMAG( VEC ) )
               IF( DA11.LT.ONE .AND. DB.GT.ONE ) THEN
                  IF( DB.GT.BIGNUM*DA11 )
     $               SCALOC = ONE / DB
               END IF
*
               X11 = ZLADIV( VEC*DCMPLX( SCALOC ), A11 )
*
               IF( SCALOC.NE.ONE ) THEN
                  DO 100 J = 1, N
                     CALL ZDSCAL( M, SCALOC, C( 1, J ), 1 )
  100             CONTINUE
                  SCALE = SCALE*SCALOC
               END IF
               C( K, L ) = X11
*
  110       CONTINUE
  120    CONTINUE
*
      END IF
*
      RETURN
*
*     End of ZTRSYL
*
      END
