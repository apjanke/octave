      SUBROUTINE ZLAQR3( WANTT, WANTZ, N, KTOP, KBOT, NW, H, LDH, ILOZ,
     $                   IHIZ, Z, LDZ, NS, ND, SH, V, LDV, NH, T, LDT,
     $                   NV, WV, LDWV, WORK, LWORK )
*
*  -- LAPACK auxiliary routine (version 3.1) --
*     Univ. of Tennessee, Univ. of California Berkeley and NAG Ltd..
*     November 2006
*
*     .. Scalar Arguments ..
      INTEGER            IHIZ, ILOZ, KBOT, KTOP, LDH, LDT, LDV, LDWV,
     $                   LDZ, LWORK, N, ND, NH, NS, NV, NW
      LOGICAL            WANTT, WANTZ
*     ..
*     .. Array Arguments ..
      COMPLEX*16         H( LDH, * ), SH( * ), T( LDT, * ), V( LDV, * ),
     $                   WORK( * ), WV( LDWV, * ), Z( LDZ, * )
*     ..
*
*     ******************************************************************
*     Aggressive early deflation:
*
*     This subroutine accepts as input an upper Hessenberg matrix
*     H and performs an unitary similarity transformation
*     designed to detect and deflate fully converged eigenvalues from
*     a trailing principal submatrix.  On output H has been over-
*     written by a new Hessenberg matrix that is a perturbation of
*     an unitary similarity transformation of H.  It is to be
*     hoped that the final version of H has many zero subdiagonal
*     entries.
*
*     ******************************************************************
*     WANTT   (input) LOGICAL
*          If .TRUE., then the Hessenberg matrix H is fully updated
*          so that the triangular Schur factor may be
*          computed (in cooperation with the calling subroutine).
*          If .FALSE., then only enough of H is updated to preserve
*          the eigenvalues.
*
*     WANTZ   (input) LOGICAL
*          If .TRUE., then the unitary matrix Z is updated so
*          so that the unitary Schur factor may be computed
*          (in cooperation with the calling subroutine).
*          If .FALSE., then Z is not referenced.
*
*     N       (input) INTEGER
*          The order of the matrix H and (if WANTZ is .TRUE.) the
*          order of the unitary matrix Z.
*
*     KTOP    (input) INTEGER
*          It is assumed that either KTOP = 1 or H(KTOP,KTOP-1)=0.
*          KBOT and KTOP together determine an isolated block
*          along the diagonal of the Hessenberg matrix.
*
*     KBOT    (input) INTEGER
*          It is assumed without a check that either
*          KBOT = N or H(KBOT+1,KBOT)=0.  KBOT and KTOP together
*          determine an isolated block along the diagonal of the
*          Hessenberg matrix.
*
*     NW      (input) INTEGER
*          Deflation window size.  1 .LE. NW .LE. (KBOT-KTOP+1).
*
*     H       (input/output) COMPLEX*16 array, dimension (LDH,N)
*          On input the initial N-by-N section of H stores the
*          Hessenberg matrix undergoing aggressive early deflation.
*          On output H has been transformed by a unitary
*          similarity transformation, perturbed, and the returned
*          to Hessenberg form that (it is to be hoped) has some
*          zero subdiagonal entries.
*
*     LDH     (input) integer
*          Leading dimension of H just as declared in the calling
*          subroutine.  N .LE. LDH
*
*     ILOZ    (input) INTEGER
*     IHIZ    (input) INTEGER
*          Specify the rows of Z to which transformations must be
*          applied if WANTZ is .TRUE.. 1 .LE. ILOZ .LE. IHIZ .LE. N.
*
*     Z       (input/output) COMPLEX*16 array, dimension (LDZ,IHI)
*          IF WANTZ is .TRUE., then on output, the unitary
*          similarity transformation mentioned above has been
*          accumulated into Z(ILOZ:IHIZ,ILO:IHI) from the right.
*          If WANTZ is .FALSE., then Z is unreferenced.
*
*     LDZ     (input) integer
*          The leading dimension of Z just as declared in the
*          calling subroutine.  1 .LE. LDZ.
*
*     NS      (output) integer
*          The number of unconverged (ie approximate) eigenvalues
*          returned in SR and SI that may be used as shifts by the
*          calling subroutine.
*
*     ND      (output) integer
*          The number of converged eigenvalues uncovered by this
*          subroutine.
*
*     SH      (output) COMPLEX*16 array, dimension KBOT
*          On output, approximate eigenvalues that may
*          be used for shifts are stored in SH(KBOT-ND-NS+1)
*          through SR(KBOT-ND).  Converged eigenvalues are
*          stored in SH(KBOT-ND+1) through SH(KBOT).
*
*     V       (workspace) COMPLEX*16 array, dimension (LDV,NW)
*          An NW-by-NW work array.
*
*     LDV     (input) integer scalar
*          The leading dimension of V just as declared in the
*          calling subroutine.  NW .LE. LDV
*
*     NH      (input) integer scalar
*          The number of columns of T.  NH.GE.NW.
*
*     T       (workspace) COMPLEX*16 array, dimension (LDT,NW)
*
*     LDT     (input) integer
*          The leading dimension of T just as declared in the
*          calling subroutine.  NW .LE. LDT
*
*     NV      (input) integer
*          The number of rows of work array WV available for
*          workspace.  NV.GE.NW.
*
*     WV      (workspace) COMPLEX*16 array, dimension (LDWV,NW)
*
*     LDWV    (input) integer
*          The leading dimension of W just as declared in the
*          calling subroutine.  NW .LE. LDV
*
*     WORK    (workspace) COMPLEX*16 array, dimension LWORK.
*          On exit, WORK(1) is set to an estimate of the optimal value
*          of LWORK for the given values of N, NW, KTOP and KBOT.
*
*     LWORK   (input) integer
*          The dimension of the work array WORK.  LWORK = 2*NW
*          suffices, but greater efficiency may result from larger
*          values of LWORK.
*
*          If LWORK = -1, then a workspace query is assumed; ZLAQR3
*          only estimates the optimal workspace size for the given
*          values of N, NW, KTOP and KBOT.  The estimate is returned
*          in WORK(1).  No error message related to LWORK is issued
*          by XERBLA.  Neither H nor Z are accessed.
*
*     ================================================================
*     Based on contributions by
*        Karen Braman and Ralph Byers, Department of Mathematics,
*        University of Kansas, USA
*
*     ==================================================================
*     .. Parameters ..
      COMPLEX*16         ZERO, ONE
      PARAMETER          ( ZERO = ( 0.0d0, 0.0d0 ),
     $                   ONE = ( 1.0d0, 0.0d0 ) )
      DOUBLE PRECISION   RZERO, RONE
      PARAMETER          ( RZERO = 0.0d0, RONE = 1.0d0 )
*     ..
*     .. Local Scalars ..
      COMPLEX*16         BETA, CDUM, S, TAU
      DOUBLE PRECISION   FOO, SAFMAX, SAFMIN, SMLNUM, ULP
      INTEGER            I, IFST, ILST, INFO, INFQR, J, JW, KCOL, KLN,
     $                   KNT, KROW, KWTOP, LTOP, LWK1, LWK2, LWK3,
     $                   LWKOPT, NMIN
*     ..
*     .. External Functions ..
      DOUBLE PRECISION   DLAMCH
      INTEGER            ILAENV
      EXTERNAL           DLAMCH, ILAENV
*     ..
*     .. External Subroutines ..
      EXTERNAL           DLABAD, ZCOPY, ZGEHRD, ZGEMM, ZLACPY, ZLAHQR,
     $                   ZLAQR4, ZLARF, ZLARFG, ZLASET, ZTREXC, ZUNGHR
*     ..
*     .. Intrinsic Functions ..
      INTRINSIC          ABS, DBLE, DCMPLX, DCONJG, DIMAG, INT, MAX, MIN
*     ..
*     .. Statement Functions ..
      DOUBLE PRECISION   CABS1
*     ..
*     .. Statement Function definitions ..
      CABS1( CDUM ) = ABS( DBLE( CDUM ) ) + ABS( DIMAG( CDUM ) )
*     ..
*     .. Executable Statements ..
*
*     ==== Estimate optimal workspace. ====
*
      JW = MIN( NW, KBOT-KTOP+1 )
      IF( JW.LE.2 ) THEN
         LWKOPT = 1
      ELSE
*
*        ==== Workspace query call to ZGEHRD ====
*
         CALL ZGEHRD( JW, 1, JW-1, T, LDT, WORK, WORK, -1, INFO )
         LWK1 = INT( WORK( 1 ) )
*
*        ==== Workspace query call to ZUNGHR ====
*
         CALL ZUNGHR( JW, 1, JW-1, T, LDT, WORK, WORK, -1, INFO )
         LWK2 = INT( WORK( 1 ) )
*
*        ==== Workspace query call to ZLAQR4 ====
*
         CALL ZLAQR4( .true., .true., JW, 1, JW, T, LDT, SH, 1, JW, V,
     $                LDV, WORK, -1, INFQR )
         LWK3 = INT( WORK( 1 ) )
*
*        ==== Optimal workspace ====
*
         LWKOPT = MAX( JW+MAX( LWK1, LWK2 ), LWK3 )
      END IF
*
*     ==== Quick return in case of workspace query. ====
*
      IF( LWORK.EQ.-1 ) THEN
         WORK( 1 ) = DCMPLX( LWKOPT, 0 )
         RETURN
      END IF
*
*     ==== Nothing to do ...
*     ... for an empty active block ... ====
      NS = 0
      ND = 0
      IF( KTOP.GT.KBOT )
     $   RETURN
*     ... nor for an empty deflation window. ====
      IF( NW.LT.1 )
     $   RETURN
*
*     ==== Machine constants ====
*
      SAFMIN = DLAMCH( 'SAFE MINIMUM' )
      SAFMAX = RONE / SAFMIN
      CALL DLABAD( SAFMIN, SAFMAX )
      ULP = DLAMCH( 'PRECISION' )
      SMLNUM = SAFMIN*( DBLE( N ) / ULP )
*
*     ==== Setup deflation window ====
*
      JW = MIN( NW, KBOT-KTOP+1 )
      KWTOP = KBOT - JW + 1
      IF( KWTOP.EQ.KTOP ) THEN
         S = ZERO
      ELSE
         S = H( KWTOP, KWTOP-1 )
      END IF
*
      IF( KBOT.EQ.KWTOP ) THEN
*
*        ==== 1-by-1 deflation window: not much to do ====
*
         SH( KWTOP ) = H( KWTOP, KWTOP )
         NS = 1
         ND = 0
         IF( CABS1( S ).LE.MAX( SMLNUM, ULP*CABS1( H( KWTOP,
     $       KWTOP ) ) ) ) THEN

            NS = 0
            ND = 1
            IF( KWTOP.GT.KTOP )
     $         H( KWTOP, KWTOP-1 ) = ZERO
         END IF
         RETURN
      END IF
*
*     ==== Convert to spike-triangular form.  (In case of a
*     .    rare QR failure, this routine continues to do
*     .    aggressive early deflation using that part of
*     .    the deflation window that converged using INFQR
*     .    here and there to keep track.) ====
*
      CALL ZLACPY( 'U', JW, JW, H( KWTOP, KWTOP ), LDH, T, LDT )
      CALL ZCOPY( JW-1, H( KWTOP+1, KWTOP ), LDH+1, T( 2, 1 ), LDT+1 )
*
      CALL ZLASET( 'A', JW, JW, ZERO, ONE, V, LDV )
      NMIN = ILAENV( 12, 'ZLAQR3', 'SV', JW, 1, JW, LWORK )
      IF( JW.GT.NMIN ) THEN
         CALL ZLAQR4( .true., .true., JW, 1, JW, T, LDT, SH( KWTOP ), 1,
     $                JW, V, LDV, WORK, LWORK, INFQR )
      ELSE
         CALL ZLAHQR( .true., .true., JW, 1, JW, T, LDT, SH( KWTOP ), 1,
     $                JW, V, LDV, INFQR )
      END IF
*
*     ==== Deflation detection loop ====
*
      NS = JW
      ILST = INFQR + 1
      DO 10 KNT = INFQR + 1, JW
*
*        ==== Small spike tip deflation test ====
*
         FOO = CABS1( T( NS, NS ) )
         IF( FOO.EQ.RZERO )
     $      FOO = CABS1( S )
         IF( CABS1( S )*CABS1( V( 1, NS ) ).LE.MAX( SMLNUM, ULP*FOO ) )
     $        THEN
*
*           ==== One more converged eigenvalue ====
*
            NS = NS - 1
         ELSE
*
*           ==== One undflatable eigenvalue.  Move it up out of the
*           .    way.   (ZTREXC can not fail in this case.) ====
*
            IFST = NS
            CALL ZTREXC( 'V', JW, T, LDT, V, LDV, IFST, ILST, INFO )
            ILST = ILST + 1
         END IF
   10 CONTINUE
*
*        ==== Return to Hessenberg form ====
*
      IF( NS.EQ.0 )
     $   S = ZERO
*
      IF( NS.LT.JW ) THEN
*
*        ==== sorting the diagonal of T improves accuracy for
*        .    graded matrices.  ====
*
         DO 30 I = INFQR + 1, NS
            IFST = I
            DO 20 J = I + 1, NS
               IF( CABS1( T( J, J ) ).GT.CABS1( T( IFST, IFST ) ) )
     $            IFST = J
   20       CONTINUE
            ILST = I
            IF( IFST.NE.ILST )
     $         CALL ZTREXC( 'V', JW, T, LDT, V, LDV, IFST, ILST, INFO )
   30    CONTINUE
      END IF
*
*     ==== Restore shift/eigenvalue array from T ====
*
      DO 40 I = INFQR + 1, JW
         SH( KWTOP+I-1 ) = T( I, I )
   40 CONTINUE
*
*
      IF( NS.LT.JW .OR. S.EQ.ZERO ) THEN
         IF( NS.GT.1 .AND. S.NE.ZERO ) THEN
*
*           ==== Reflect spike back into lower triangle ====
*
            CALL ZCOPY( NS, V, LDV, WORK, 1 )
            DO 50 I = 1, NS
               WORK( I ) = DCONJG( WORK( I ) )
   50       CONTINUE
            BETA = WORK( 1 )
            CALL ZLARFG( NS, BETA, WORK( 2 ), 1, TAU )
            WORK( 1 ) = ONE
*
            CALL ZLASET( 'L', JW-2, JW-2, ZERO, ZERO, T( 3, 1 ), LDT )
*
            CALL ZLARF( 'L', NS, JW, WORK, 1, DCONJG( TAU ), T, LDT,
     $                  WORK( JW+1 ) )
            CALL ZLARF( 'R', NS, NS, WORK, 1, TAU, T, LDT,
     $                  WORK( JW+1 ) )
            CALL ZLARF( 'R', JW, NS, WORK, 1, TAU, V, LDV,
     $                  WORK( JW+1 ) )
*
            CALL ZGEHRD( JW, 1, NS, T, LDT, WORK, WORK( JW+1 ),
     $                   LWORK-JW, INFO )
         END IF
*
*        ==== Copy updated reduced window into place ====
*
         IF( KWTOP.GT.1 )
     $      H( KWTOP, KWTOP-1 ) = S*DCONJG( V( 1, 1 ) )
         CALL ZLACPY( 'U', JW, JW, T, LDT, H( KWTOP, KWTOP ), LDH )
         CALL ZCOPY( JW-1, T( 2, 1 ), LDT+1, H( KWTOP+1, KWTOP ),
     $               LDH+1 )
*
*        ==== Accumulate orthogonal matrix in order update
*        .    H and Z, if requested.  (A modified version
*        .    of  ZUNGHR that accumulates block Householder
*        .    transformations into V directly might be
*        .    marginally more efficient than the following.) ====
*
         IF( NS.GT.1 .AND. S.NE.ZERO ) THEN
            CALL ZUNGHR( JW, 1, NS, T, LDT, WORK, WORK( JW+1 ),
     $                   LWORK-JW, INFO )
            CALL ZGEMM( 'N', 'N', JW, NS, NS, ONE, V, LDV, T, LDT, ZERO,
     $                  WV, LDWV )
            CALL ZLACPY( 'A', JW, NS, WV, LDWV, V, LDV )
         END IF
*
*        ==== Update vertical slab in H ====
*
         IF( WANTT ) THEN
            LTOP = 1
         ELSE
            LTOP = KTOP
         END IF
         DO 60 KROW = LTOP, KWTOP - 1, NV
            KLN = MIN( NV, KWTOP-KROW )
            CALL ZGEMM( 'N', 'N', KLN, JW, JW, ONE, H( KROW, KWTOP ),
     $                  LDH, V, LDV, ZERO, WV, LDWV )
            CALL ZLACPY( 'A', KLN, JW, WV, LDWV, H( KROW, KWTOP ), LDH )
   60    CONTINUE
*
*        ==== Update horizontal slab in H ====
*
         IF( WANTT ) THEN
            DO 70 KCOL = KBOT + 1, N, NH
               KLN = MIN( NH, N-KCOL+1 )
               CALL ZGEMM( 'C', 'N', JW, KLN, JW, ONE, V, LDV,
     $                     H( KWTOP, KCOL ), LDH, ZERO, T, LDT )
               CALL ZLACPY( 'A', JW, KLN, T, LDT, H( KWTOP, KCOL ),
     $                      LDH )
   70       CONTINUE
         END IF
*
*        ==== Update vertical slab in Z ====
*
         IF( WANTZ ) THEN
            DO 80 KROW = ILOZ, IHIZ, NV
               KLN = MIN( NV, IHIZ-KROW+1 )
               CALL ZGEMM( 'N', 'N', KLN, JW, JW, ONE, Z( KROW, KWTOP ),
     $                     LDZ, V, LDV, ZERO, WV, LDWV )
               CALL ZLACPY( 'A', KLN, JW, WV, LDWV, Z( KROW, KWTOP ),
     $                      LDZ )
   80       CONTINUE
         END IF
      END IF
*
*     ==== Return the number of deflations ... ====
*
      ND = JW - NS
*
*     ==== ... and the number of shifts. (Subtracting
*     .    INFQR from the spike length takes care
*     .    of the case of a rare QR failure while
*     .    calculating eigenvalues of the deflation
*     .    window.)  ====
*
      NS = NS - INFQR
*
*      ==== Return optimal workspace. ====
*
      WORK( 1 ) = DCMPLX( LWKOPT, 0 )
*
*     ==== End of ZLAQR3 ====
*
      END
