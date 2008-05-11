      SUBROUTINE SSUBSP(NMAX, N, A, B, Z, FTEST, EPS, NDIM, FAIL, IND)
      INTEGER NMAX, N, FTEST, NDIM, IND(N)
      LOGICAL FAIL
      REAL A(NMAX,N), B(NMAX,N), Z(NMAX,N), EPS
C*
C* GIVEN THE UPPER TRIANGULAR MATRIX B AND UPPER HESSENBERG MATRIX A
C* WITH 1X1 OR 2X2 DIAGONAL BLOCKS, THIS ROUTINE REORDERS THE DIAGONAL
C* BLOCKS ALONG WITH THEIR GENERALIZED EIGENVALUES BY CONSTRUCTING EQUI-
C* VALENCE TRANSFORMATIONS QT AND ZT. THE ROW TRANSFORMATION ZT IS ALSO
C* PERFORMED ON THE GIVEN (INITIAL) TRANSFORMATION Z (RESULTING FROM A
C* POSSIBLE PREVIOUS STEP OR INITIALIZED WITH THE IDENTITY MATRIX).
C* AFTER REORDERING, THE EIGENVALUES INSIDE THE REGION SPECIFIED BY THE
C* FUNCTION FTEST APPEAR AT THE TOP. IF NDIM IS THEIR NUMBER THEN THE
C* NDIM FIRST COLUMNS OF Z SPAN THE REQUESTED SUBSPACE. DSUBSP REQUIRES
C* THE SUBROUTINE EXCHQZ AND THE INTEGER FUNCTION FTEST WHICH HAS TO BE
C* PROVIDED BY THE USER. THE PARAMETERS IN THE CALLING SEQUENCE ARE :
C* (STARRED PARAMETERS ARE ALTERED BY THE SUBROUTINE)
C*
C*    NMAX     THE FIRST DIMENSION OF A, B AND Z
C*    N        THE ORDER OF A, B AND Z
C*   *A,*B     THE MATRIX PAIR WHOSE BLOCKS ARE TO BE REORDERED.
C*   *Z        UPON RETURN THIS ARRAY IS MULTIPLIED BY THE COLUMN
C*             TRANSFORMATION ZT.
C*    FTEST(LS,ALPHA,BETA,S,P) AN INTEGER FUNCTION DESCRIBING THE
C*             SPECTRUM OF THE DEFLATING SUBSPACE TO BE COMPUTED:
C*             WHEN LS=1 FTEST CHECKS IF ALPHA/BETA IS IN THAT SPECTRUM
C*             WHEN LS=2 FTEST CHECKS IF THE TWO COMPLEX CONJUGATE
C*             ROOTS WITH SUM S AND PRODUCT P ARE IN THAT SPECTRUM
C*             IF THE ANSWER IS POSITIVE, FTEST=1, OTHERWISE FTEST=-1
C*    EPS      THE REQUIRED ABSOLUTE ACCURACY OF THE RESULT
C*   *NDIM     AN INTEGER GIVING THE DIMENSION OF THE COMPUTED
C*             DEFLATING SUBSPACE
C*   *FAIL     A LOGICAL VARIABLE WHICH IS FALSE ON A NORMAL RETURN,
C*             TRUE OTHERWISE (WHEN SEXCHQZ FAILS)
C*   *IND      AN INTEGER WORKING ARRAY OF DIMENSION AT LEAST N
C*
      INTEGER L, LS, LS1, LS2, L1, LL, NUM, IS, L2I, L2K, I, K, II,
     * ISTEP, IFIRST
      REAL S, P, D, ALPHA, BETA
      FAIL = .TRUE.
      NDIM = 0
      NUM = 0
      L = 0
      LS = 1
C*** CONSTRUCT ARRAY IND(I) WHERE :
C***     IABS(IND(I)) IS THE SIZE OF THE BLOCK I
C***     SIGN(IND(I)) INDICATES THE LOCATION OF ITS EIGENVALUES
C***                  (AS DETERMINED BY FTEST).
C*** NUM IS THE NUMBER OF ELEMENTS IN THIS ARRAY
      DO 30 LL=1,N
        L = L + LS
        IF (L.GT.N) GO TO 40
        L1 = L + 1
        IF (L1.GT.N) GO TO 10
        IF (A(L1,L).EQ.0.) GO TO 10
C* HERE A 2X2  BLOCK IS CHECKED *
        LS = 2
        D = B(L,L)*B(L1,L1)
        S = (A(L,L)*B(L1,L1)+A(L1,L1)*B(L,L)-A(L1,L)*B(L,L1))/D
        P = (A(L,L)*A(L1,L1)-A(L,L1)*A(L1,L))/D
        IS = FTEST(LS,ALPHA,BETA,S,P)
        GO TO 20
C* HERE A 1X1  BLOCK IS CHECKED *
   10   LS = 1
        IS = FTEST(LS,A(L,L),B(L,L),S,P)
   20   NUM = NUM + 1
        IF (IS.EQ.1) NDIM = NDIM + LS
        IND(NUM) = LS*IS
   30 CONTINUE
C***  REORDER BLOCKS SUCH THAT THOSE WITH POSITIVE VALUE
C***    OF IND(.) APPEAR FIRST.
   40 L2I = 1
      DO 100 I=1,NUM
        IF (IND(I).GT.0) GO TO 90
C* IF A NEGATIVE IND(I) IS ENCOUNTERED, THEN SEARCH FOR THE FIRST
C* POSITIVE IND(K) FOLLOWING ON IT
        L2K = L2I
        DO 60 K=I,NUM
          IF (IND(K).LT.0) GO TO 50
          GO TO 70
   50     L2K = L2K - IND(K)
   60   CONTINUE
C* IF THERE ARE NO POSITIVE INDICES FOLLOWING ON A NEGATIVE ONE
C* THEN STOP
        GO TO 110
C* IF A POSITIVE IND(K) FOLLOWS ON A NEGATIVE IND(I) THEN
C* INTERCHANGE BLOCK K BEFORE BLOCK I BY PERFORMING K-I SWAPS
   70   ISTEP = K - I
        LS2 = IND(K)
        L = L2K
        DO 80 II=1,ISTEP
          IFIRST = K - II
          LS1 = -IND(IFIRST)
          L = L - LS1
          CALL SEXCHQZ(NMAX, N, A, B, Z, L, LS1, LS2, EPS, FAIL)
          IF (FAIL) RETURN
          IND(IFIRST+1) = IND(IFIRST)
   80   CONTINUE
        IND(I) = LS2
   90   L2I = L2I + IND(I)
  100 CONTINUE
  110 FAIL = .FALSE.
      RETURN
      END
