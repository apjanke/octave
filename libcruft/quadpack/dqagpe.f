      SUBROUTINE DQAGPE(F,A,B,NPTS2,POINTS,EPSABS,EPSREL,LIMIT,RESULT,
     *   ABSERR,NEVAL,IER,ALIST,BLIST,RLIST,ELIST,PTS,IORD,LEVEL,NDIN,
     *   LAST)
C***BEGIN PROLOGUE  DQAGPE
C***DATE WRITTEN   800101   (YYMMDD)
C***REVISION DATE  830518   (YYMMDD)
C***CATEGORY NO.  H2A2A1
C***KEYWORDS  AUTOMATIC INTEGRATOR, GENERAL-PURPOSE,
C             SINGULARITIES AT USER SPECIFIED POINTS,
C             EXTRAPOLATION, GLOBALLY ADAPTIVE.
C***AUTHOR  PIESSENS,ROBERT ,APPL. MATH. & PROGR. DIV. - K.U.LEUVEN
C           DE DONCKER,ELISE,APPL. MATH. & PROGR. DIV. - K.U.LEUVEN
C***PURPOSE  THE ROUTINE CALCULATES AN APPROXIMATION RESULT TO A GIVEN
C            DEFINITE INTEGRAL I = INTEGRAL OF F OVER (A,B), HOPEFULLY
C            SATISFYING FOLLOWING CLAIM FOR ACCURACY ABS(I-RESULT).LE.
C            MAX(EPSABS,EPSREL*ABS(I)). BREAK POINTS OF THE INTEGRATION
C            INTERVAL, WHERE LOCAL DIFFICULTIES OF THE INTEGRAND MAY
C            OCCUR(E.G. SINGULARITIES,DISCONTINUITIES),PROVIDED BY USER.
C***DESCRIPTION
C
C        COMPUTATION OF A DEFINITE INTEGRAL
C        STANDARD FORTRAN SUBROUTINE
C        DOUBLE PRECISION VERSION
C
C        PARAMETERS
C         ON ENTRY
C            F      - SUBROUTINE F(X,IERR,RESULT) DEFINING THE INTEGRAND
C                     FUNCTION F(X). THE ACTUAL NAME FOR F NEEDS TO BE
C                     DECLARED E X T E R N A L IN THE DRIVER PROGRAM.
C
C            A      - DOUBLE PRECISION
C                     LOWER LIMIT OF INTEGRATION
C
C            B      - DOUBLE PRECISION
C                     UPPER LIMIT OF INTEGRATION
C
C            NPTS2  - INTEGER
C                     NUMBER EQUAL TO TWO MORE THAN THE NUMBER OF
C                     USER-SUPPLIED BREAK POINTS WITHIN THE INTEGRATION
C                     RANGE, NPTS2.GE.2.
C                     IF NPTS2.LT.2, THE ROUTINE WILL END WITH IER = 6.
C
C            POINTS - DOUBLE PRECISION
C                     VECTOR OF DIMENSION NPTS2, THE FIRST (NPTS2-2)
C                     ELEMENTS OF WHICH ARE THE USER PROVIDED BREAK
C                     POINTS. IF THESE POINTS DO NOT CONSTITUTE AN
C                     ASCENDING SEQUENCE THERE WILL BE AN AUTOMATIC
C                     SORTING.
C
C            EPSABS - DOUBLE PRECISION
C                     ABSOLUTE ACCURACY REQUESTED
C            EPSREL - DOUBLE PRECISION
C                     RELATIVE ACCURACY REQUESTED
C                     IF  EPSABS.LE.0
C                     AND EPSREL.LT.MAX(50*REL.MACH.ACC.,0.5D-28),
C                     THE ROUTINE WILL END WITH IER = 6.
C
C            LIMIT  - INTEGER
C                     GIVES AN UPPER BOUND ON THE NUMBER OF SUBINTERVALS
C                     IN THE PARTITION OF (A,B), LIMIT.GE.NPTS2
C                     IF LIMIT.LT.NPTS2, THE ROUTINE WILL END WITH
C                     IER = 6.
C
C         ON RETURN
C            RESULT - DOUBLE PRECISION
C                     APPROXIMATION TO THE INTEGRAL
C
C            ABSERR - DOUBLE PRECISION
C                     ESTIMATE OF THE MODULUS OF THE ABSOLUTE ERROR,
C                     WHICH SHOULD EQUAL OR EXCEED ABS(I-RESULT)
C
C            NEVAL  - INTEGER
C                     NUMBER OF INTEGRAND EVALUATIONS
C
C            IER    - INTEGER
C                     IER = 0 NORMAL AND RELIABLE TERMINATION OF THE
C                             ROUTINE. IT IS ASSUMED THAT THE REQUESTED
C                             ACCURACY HAS BEEN ACHIEVED.
C                     IER.GT.0 ABNORMAL TERMINATION OF THE ROUTINE.
C                             THE ESTIMATES FOR INTEGRAL AND ERROR ARE
C                             LESS RELIABLE. IT IS ASSUMED THAT THE
C                             REQUESTED ACCURACY HAS NOT BEEN ACHIEVED.
C                      IER.LT.0 EXIT REQUESTED FROM USER-SUPPLIED
C                             FUNCTION.
C
C            ERROR MESSAGES
C                     IER = 1 MAXIMUM NUMBER OF SUBDIVISIONS ALLOWED
C                             HAS BEEN ACHIEVED. ONE CAN ALLOW MORE
C                             SUBDIVISIONS BY INCREASING THE VALUE OF
C                             LIMIT (AND TAKING THE ACCORDING DIMENSION
C                             ADJUSTMENTS INTO ACCOUNT). HOWEVER, IF
C                             THIS YIELDS NO IMPROVEMENT IT IS ADVISED
C                             TO ANALYZE THE INTEGRAND IN ORDER TO
C                             DETERMINE THE INTEGRATION DIFFICULTIES. IF
C                             THE POSITION OF A LOCAL DIFFICULTY CAN BE
C                             DETERMINED (I.E. SINGULARITY,
C                             DISCONTINUITY WITHIN THE INTERVAL), IT
C                             SHOULD BE SUPPLIED TO THE ROUTINE AS AN
C                             ELEMENT OF THE VECTOR POINTS. IF NECESSARY
C                             AN APPROPRIATE SPECIAL-PURPOSE INTEGRATOR
C                             MUST BE USED, WHICH IS DESIGNED FOR
C                             HANDLING THE TYPE OF DIFFICULTY INVOLVED.
C                         = 2 THE OCCURRENCE OF ROUNDOFF ERROR IS
C                             DETECTED, WHICH PREVENTS THE REQUESTED
C                             TOLERANCE FROM BEING ACHIEVED.
C                             THE ERROR MAY BE UNDER-ESTIMATED.
C                         = 3 EXTREMELY BAD INTEGRAND BEHAVIOUR OCCURS
C                             AT SOME POINTS OF THE INTEGRATION
C                             INTERVAL.
C                         = 4 THE ALGORITHM DOES NOT CONVERGE.
C                             ROUNDOFF ERROR IS DETECTED IN THE
C                             EXTRAPOLATION TABLE. IT IS PRESUMED THAT
C                             THE REQUESTED TOLERANCE CANNOT BE
C                             ACHIEVED, AND THAT THE RETURNED RESULT IS
C                             THE BEST WHICH CAN BE OBTAINED.
C                         = 5 THE INTEGRAL IS PROBABLY DIVERGENT, OR
C                             SLOWLY CONVERGENT. IT MUST BE NOTED THAT
C                             DIVERGENCE CAN OCCUR WITH ANY OTHER VALUE
C                             OF IER.GT.0.
C                         = 6 THE INPUT IS INVALID BECAUSE
C                             NPTS2.LT.2 OR
C                             BREAK POINTS ARE SPECIFIED OUTSIDE
C                             THE INTEGRATION RANGE OR
C                             (EPSABS.LE.0 AND
C                              EPSREL.LT.MAX(50*REL.MACH.ACC.,0.5D-28))
C                             OR LIMIT.LT.NPTS2.
C                             RESULT, ABSERR, NEVAL, LAST, RLIST(1),
C                             AND ELIST(1) ARE SET TO ZERO. ALIST(1) AND
C                             BLIST(1) ARE SET TO A AND B RESPECTIVELY.
C
C            ALIST  - DOUBLE PRECISION
C                     VECTOR OF DIMENSION AT LEAST LIMIT, THE FIRST
C                      LAST  ELEMENTS OF WHICH ARE THE LEFT END POINTS
C                     OF THE SUBINTERVALS IN THE PARTITION OF THE GIVEN
C                     INTEGRATION RANGE (A,B)
C
C            BLIST  - DOUBLE PRECISION
C                     VECTOR OF DIMENSION AT LEAST LIMIT, THE FIRST
C                      LAST  ELEMENTS OF WHICH ARE THE RIGHT END POINTS
C                     OF THE SUBINTERVALS IN THE PARTITION OF THE GIVEN
C                     INTEGRATION RANGE (A,B)
C
C            RLIST  - DOUBLE PRECISION
C                     VECTOR OF DIMENSION AT LEAST LIMIT, THE FIRST
C                      LAST  ELEMENTS OF WHICH ARE THE INTEGRAL
C                     APPROXIMATIONS ON THE SUBINTERVALS
C
C            ELIST  - DOUBLE PRECISION
C                     VECTOR OF DIMENSION AT LEAST LIMIT, THE FIRST
C                      LAST  ELEMENTS OF WHICH ARE THE MODULI OF THE
C                     ABSOLUTE ERROR ESTIMATES ON THE SUBINTERVALS
C
C            PTS    - DOUBLE PRECISION
C                     VECTOR OF DIMENSION AT LEAST NPTS2, CONTAINING THE
C                     INTEGRATION LIMITS AND THE BREAK POINTS OF THE
C                     INTERVAL IN ASCENDING SEQUENCE.
C
C            LEVEL  - INTEGER
C                     VECTOR OF DIMENSION AT LEAST LIMIT, CONTAINING THE
C                     SUBDIVISION LEVELS OF THE SUBINTERVAL, I.E. IF
C                     (AA,BB) IS A SUBINTERVAL OF (P1,P2) WHERE P1 AS
C                     WELL AS P2 IS A USER-PROVIDED BREAK POINT OR
C                     INTEGRATION LIMIT, THEN (AA,BB) HAS LEVEL L IF
C                     ABS(BB-AA) = ABS(P2-P1)*2**(-L).
C
C            NDIN   - INTEGER
C                     VECTOR OF DIMENSION AT LEAST NPTS2, AFTER FIRST
C                     INTEGRATION OVER THE INTERVALS (PTS(I)),PTS(I+1),
C                     I = 0,1, ..., NPTS2-2, THE ERROR ESTIMATES OVER
C                     SOME OF THE INTERVALS MAY HAVE BEEN INCREASED
C                     ARTIFICIALLY, IN ORDER TO PUT THEIR SUBDIVISION
C                     FORWARD. IF THIS HAPPENS FOR THE SUBINTERVAL
C                     NUMBERED K, NDIN(K) IS PUT TO 1, OTHERWISE
C                     NDIN(K) = 0.
C
C            IORD   - INTEGER
C                     VECTOR OF DIMENSION AT LEAST LIMIT, THE FIRST K
C                     ELEMENTS OF WHICH ARE POINTERS TO THE
C                     ERROR ESTIMATES OVER THE SUBINTERVALS,
C                     SUCH THAT ELIST(IORD(1)), ..., ELIST(IORD(K))
C                     FORM A DECREASING SEQUENCE, WITH K = LAST
C                     IF LAST.LE.(LIMIT/2+2), AND K = LIMIT+1-LAST
C                     OTHERWISE
C
C            LAST   - INTEGER
C                     NUMBER OF SUBINTERVALS ACTUALLY PRODUCED IN THE
C                     SUBDIVISIONS PROCESS
C
C***REFERENCES  (NONE)
C***ROUTINES CALLED  D1MACH,DQELG,DQK21,DQPSRT
C***END PROLOGUE  DQAGPE
      DOUBLE PRECISION A,ABSEPS,ABSERR,ALIST,AREA,AREA1,AREA12,AREA2,A1,
     *  A2,B,BLIST,B1,B2,CORREC,DABS,DEFABS,DEFAB1,DEFAB2,DMAX1,DMIN1,
     *  DRES,D1MACH,ELIST,EPMACH,EPSABS,EPSREL,ERLARG,ERLAST,ERRBND,
     *  ERRMAX,ERROR1,ERRO12,ERROR2,ERRSUM,ERTEST,OFLOW,POINTS,PTS,
     *  RESA,RESABS,RESEPS,RESULT,RES3LA,RLIST,RLIST2,SIGN,TEMP,UFLOW
      INTEGER I,ID,IER,IERRO,IND1,IND2,IORD,IP1,IROFF1,IROFF2,IROFF3,J,
     *  JLOW,JUPBND,K,KSGN,KTMIN,LAST,LEVCUR,LEVEL,LEVMAX,LIMIT,MAXERR,
     *  NDIN,NEVAL,NINT,NINTP1,NPTS,NPTS2,NRES,NRMAX,NUMRL2
      LOGICAL EXTRAP,NOEXT
C
C
      DIMENSION ALIST(LIMIT),BLIST(LIMIT),ELIST(LIMIT),IORD(LIMIT),
     *  LEVEL(LIMIT),NDIN(NPTS2),POINTS(NPTS2),PTS(NPTS2),RES3LA(3),
     *  RLIST(LIMIT),RLIST2(52)
C
      EXTERNAL F
C
C            THE DIMENSION OF RLIST2 IS DETERMINED BY THE VALUE OF
C            LIMEXP IN SUBROUTINE EPSALG (RLIST2 SHOULD BE OF DIMENSION
C            (LIMEXP+2) AT LEAST).
C
C
C            LIST OF MAJOR VARIABLES
C            -----------------------
C
C           ALIST     - LIST OF LEFT END POINTS OF ALL SUBINTERVALS
C                       CONSIDERED UP TO NOW
C           BLIST     - LIST OF RIGHT END POINTS OF ALL SUBINTERVALS
C                       CONSIDERED UP TO NOW
C           RLIST(I)  - APPROXIMATION TO THE INTEGRAL OVER
C                       (ALIST(I),BLIST(I))
C           RLIST2    - ARRAY OF DIMENSION AT LEAST LIMEXP+2
C                       CONTAINING THE PART OF THE EPSILON TABLE WHICH
C                       IS STILL NEEDED FOR FURTHER COMPUTATIONS
C           ELIST(I)  - ERROR ESTIMATE APPLYING TO RLIST(I)
C           MAXERR    - POINTER TO THE INTERVAL WITH LARGEST ERROR
C                       ESTIMATE
C           ERRMAX    - ELIST(MAXERR)
C           ERLAST    - ERROR ON THE INTERVAL CURRENTLY SUBDIVIDED
C                       (BEFORE THAT SUBDIVISION HAS TAKEN PLACE)
C           AREA      - SUM OF THE INTEGRALS OVER THE SUBINTERVALS
C           ERRSUM    - SUM OF THE ERRORS OVER THE SUBINTERVALS
C           ERRBND    - REQUESTED ACCURACY MAX(EPSABS,EPSREL*
C                       ABS(RESULT))
C           *****1    - VARIABLE FOR THE LEFT SUBINTERVAL
C           *****2    - VARIABLE FOR THE RIGHT SUBINTERVAL
C           LAST      - INDEX FOR SUBDIVISION
C           NRES      - NUMBER OF CALLS TO THE EXTRAPOLATION ROUTINE
C           NUMRL2    - NUMBER OF ELEMENTS IN RLIST2. IF AN APPROPRIATE
C                       APPROXIMATION TO THE COMPOUNDED INTEGRAL HAS
C                       BEEN OBTAINED, IT IS PUT IN RLIST2(NUMRL2) AFTER
C                       NUMRL2 HAS BEEN INCREASED BY ONE.
C           ERLARG    - SUM OF THE ERRORS OVER THE INTERVALS LARGER
C                       THAN THE SMALLEST INTERVAL CONSIDERED UP TO NOW
C           EXTRAP    - LOGICAL VARIABLE DENOTING THAT THE ROUTINE
C                       IS ATTEMPTING TO PERFORM EXTRAPOLATION. I.E.
C                       BEFORE SUBDIVIDING THE SMALLEST INTERVAL WE
C                       TRY TO DECREASE THE VALUE OF ERLARG.
C           NOEXT     - LOGICAL VARIABLE DENOTING THAT EXTRAPOLATION IS
C                       NO LONGER ALLOWED (TRUE-VALUE)
C
C            MACHINE DEPENDENT CONSTANTS
C            ---------------------------
C
C           EPMACH IS THE LARGEST RELATIVE SPACING.
C           UFLOW IS THE SMALLEST POSITIVE MAGNITUDE.
C           OFLOW IS THE LARGEST POSITIVE MAGNITUDE.
C
C***FIRST EXECUTABLE STATEMENT  DQAGPE
      EPMACH = D1MACH(4)
C
C            TEST ON VALIDITY OF PARAMETERS
C            -----------------------------
C
      IER = 0
      NEVAL = 0
      LAST = 0
      RESULT = 0.0D+00
      ABSERR = 0.0D+00
      ALIST(1) = A
      BLIST(1) = B
      RLIST(1) = 0.0D+00
      ELIST(1) = 0.0D+00
      IORD(1) = 0
      LEVEL(1) = 0
      NPTS = NPTS2-2
      IF(NPTS2.LT.2.OR.LIMIT.LE.NPTS.OR.(EPSABS.LE.0.0D+00.AND.
     *  EPSREL.LT.DMAX1(0.5D+02*EPMACH,0.5D-28))) IER = 6
      IF(IER.EQ.6) GO TO 999
C
C            IF ANY BREAK POINTS ARE PROVIDED, SORT THEM INTO AN
C            ASCENDING SEQUENCE.
C
      SIGN = 1.0D+00
      IF(A.GT.B) SIGN = -1.0D+00
      PTS(1) = DMIN1(A,B)
      IF(NPTS.EQ.0) GO TO 15
      DO 10 I = 1,NPTS
        PTS(I+1) = POINTS(I)
   10 CONTINUE
   15 PTS(NPTS+2) = DMAX1(A,B)
      NINT = NPTS+1
      A1 = PTS(1)
      IF(NPTS.EQ.0) GO TO 40
      NINTP1 = NINT+1
      DO 20 I = 1,NINT
        IP1 = I+1
        DO 20 J = IP1,NINTP1
          IF(PTS(I).LE.PTS(J)) GO TO 20
          TEMP = PTS(I)
          PTS(I) = PTS(J)
          PTS(J) = TEMP
   20 CONTINUE
      IF(PTS(1).NE.DMIN1(A,B).OR.PTS(NINTP1).NE.DMAX1(A,B)) IER = 6
      IF(IER.EQ.6) GO TO 999
C
C            COMPUTE FIRST INTEGRAL AND ERROR APPROXIMATIONS.
C            ------------------------------------------------
C
   40 RESABS = 0.0D+00
      DO 50 I = 1,NINT
        B1 = PTS(I+1)
        CALL DQK21(F,A1,B1,AREA1,ERROR1,DEFABS,RESA,IER)
        IF (IER .LT. 0) RETURN
        ABSERR = ABSERR+ERROR1
        RESULT = RESULT+AREA1
        NDIN(I) = 0
        IF(ERROR1.EQ.RESA.AND.ERROR1.NE.0.0D+00) NDIN(I) = 1
        RESABS = RESABS+DEFABS
        LEVEL(I) = 0
        ELIST(I) = ERROR1
        ALIST(I) = A1
        BLIST(I) = B1
        RLIST(I) = AREA1
        IORD(I) = I
        A1 = B1
   50 CONTINUE
      ERRSUM = 0.0D+00
      DO 55 I = 1,NINT
        IF(NDIN(I).EQ.1) ELIST(I) = ABSERR
        ERRSUM = ERRSUM+ELIST(I)
   55 CONTINUE
C
C           TEST ON ACCURACY.
C
      LAST = NINT
      NEVAL = 21*NINT
      DRES = DABS(RESULT)
      ERRBND = DMAX1(EPSABS,EPSREL*DRES)
      IF(ABSERR.LE.0.1D+03*EPMACH*RESABS.AND.ABSERR.GT.ERRBND) IER = 2
      IF(NINT.EQ.1) GO TO 80
      DO 70 I = 1,NPTS
        JLOW = I+1
        IND1 = IORD(I)
        DO 60 J = JLOW,NINT
          IND2 = IORD(J)
          IF(ELIST(IND1).GT.ELIST(IND2)) GO TO 60
          IND1 = IND2
          K = J
   60   CONTINUE
        IF(IND1.EQ.IORD(I)) GO TO 70
        IORD(K) = IORD(I)
        IORD(I) = IND1
   70 CONTINUE
      IF(LIMIT.LT.NPTS2) IER = 1
   80 IF(IER.NE.0.OR.ABSERR.LE.ERRBND) GO TO 210
C
C           INITIALIZATION
C           --------------
C
      RLIST2(1) = RESULT
      MAXERR = IORD(1)
      ERRMAX = ELIST(MAXERR)
      AREA = RESULT
      NRMAX = 1
      NRES = 0
      NUMRL2 = 1
      KTMIN = 0
      EXTRAP = .FALSE.
      NOEXT = .FALSE.
      ERLARG = ERRSUM
      ERTEST = ERRBND
      LEVMAX = 1
      IROFF1 = 0
      IROFF2 = 0
      IROFF3 = 0
      IERRO = 0
      UFLOW = D1MACH(1)
      OFLOW = D1MACH(2)
      ABSERR = OFLOW
      KSGN = -1
      IF(DRES.GE.(0.1D+01-0.5D+02*EPMACH)*RESABS) KSGN = 1
C
C           MAIN DO-LOOP
C           ------------
C
      DO 160 LAST = NPTS2,LIMIT
C
C           BISECT THE SUBINTERVAL WITH THE NRMAX-TH LARGEST ERROR
C           ESTIMATE.
C
        LEVCUR = LEVEL(MAXERR)+1
        A1 = ALIST(MAXERR)
        B1 = 0.5D+00*(ALIST(MAXERR)+BLIST(MAXERR))
        A2 = B1
        B2 = BLIST(MAXERR)
        ERLAST = ERRMAX
        CALL DQK21(F,A1,B1,AREA1,ERROR1,RESA,DEFAB1,IER)
        IF (IER .LT. 0) RETURN
        CALL DQK21(F,A2,B2,AREA2,ERROR2,RESA,DEFAB2,IER)
        IF (IER .LT. 0) RETURN
C
C           IMPROVE PREVIOUS APPROXIMATIONS TO INTEGRAL
C           AND ERROR AND TEST FOR ACCURACY.
C
        NEVAL = NEVAL+42
        AREA12 = AREA1+AREA2
        ERRO12 = ERROR1+ERROR2
        ERRSUM = ERRSUM+ERRO12-ERRMAX
        AREA = AREA+AREA12-RLIST(MAXERR)
        IF(DEFAB1.EQ.ERROR1.OR.DEFAB2.EQ.ERROR2) GO TO 95
        IF(DABS(RLIST(MAXERR)-AREA12).GT.0.1D-04*DABS(AREA12)
     *  .OR.ERRO12.LT.0.99D+00*ERRMAX) GO TO 90
        IF(EXTRAP) IROFF2 = IROFF2+1
        IF(.NOT.EXTRAP) IROFF1 = IROFF1+1
   90   IF(LAST.GT.10.AND.ERRO12.GT.ERRMAX) IROFF3 = IROFF3+1
   95   LEVEL(MAXERR) = LEVCUR
        LEVEL(LAST) = LEVCUR
        RLIST(MAXERR) = AREA1
        RLIST(LAST) = AREA2
        ERRBND = DMAX1(EPSABS,EPSREL*DABS(AREA))
C
C           TEST FOR ROUNDOFF ERROR AND EVENTUALLY SET ERROR FLAG.
C
        IF(IROFF1+IROFF2.GE.10.OR.IROFF3.GE.20) IER = 2
        IF(IROFF2.GE.5) IERRO = 3
C
C           SET ERROR FLAG IN THE CASE THAT THE NUMBER OF
C           SUBINTERVALS EQUALS LIMIT.
C
        IF(LAST.EQ.LIMIT) IER = 1
C
C           SET ERROR FLAG IN THE CASE OF BAD INTEGRAND BEHAVIOUR
C           AT A POINT OF THE INTEGRATION RANGE
C
        IF(DMAX1(DABS(A1),DABS(B2)).LE.(0.1D+01+0.1D+03*EPMACH)*
     *  (DABS(A2)+0.1D+04*UFLOW)) IER = 4
C
C           APPEND THE NEWLY-CREATED INTERVALS TO THE LIST.
C
        IF(ERROR2.GT.ERROR1) GO TO 100
        ALIST(LAST) = A2
        BLIST(MAXERR) = B1
        BLIST(LAST) = B2
        ELIST(MAXERR) = ERROR1
        ELIST(LAST) = ERROR2
        GO TO 110
  100   ALIST(MAXERR) = A2
        ALIST(LAST) = A1
        BLIST(LAST) = B1
        RLIST(MAXERR) = AREA2
        RLIST(LAST) = AREA1
        ELIST(MAXERR) = ERROR2
        ELIST(LAST) = ERROR1
C
C           CALL SUBROUTINE DQPSRT TO MAINTAIN THE DESCENDING ORDERING
C           IN THE LIST OF ERROR ESTIMATES AND SELECT THE SUBINTERVAL
C           WITH NRMAX-TH LARGEST ERROR ESTIMATE (TO BE BISECTED NEXT).
C
  110   CALL DQPSRT(LIMIT,LAST,MAXERR,ERRMAX,ELIST,IORD,NRMAX)
C ***JUMP OUT OF DO-LOOP
        IF(ERRSUM.LE.ERRBND) GO TO 190
C ***JUMP OUT OF DO-LOOP
        IF(IER.NE.0) GO TO 170
        IF(NOEXT) GO TO 160
        ERLARG = ERLARG-ERLAST
        IF(LEVCUR+1.LE.LEVMAX) ERLARG = ERLARG+ERRO12
        IF(EXTRAP) GO TO 120
C
C           TEST WHETHER THE INTERVAL TO BE BISECTED NEXT IS THE
C           SMALLEST INTERVAL.
C
        IF(LEVEL(MAXERR)+1.LE.LEVMAX) GO TO 160
        EXTRAP = .TRUE.
        NRMAX = 2
  120   IF(IERRO.EQ.3.OR.ERLARG.LE.ERTEST) GO TO 140
C
C           THE SMALLEST INTERVAL HAS THE LARGEST ERROR.
C           BEFORE BISECTING DECREASE THE SUM OF THE ERRORS OVER
C           THE LARGER INTERVALS (ERLARG) AND PERFORM EXTRAPOLATION.
C
        ID = NRMAX
        JUPBND = LAST
        IF(LAST.GT.(2+LIMIT/2)) JUPBND = LIMIT+3-LAST
        DO 130 K = ID,JUPBND
          MAXERR = IORD(NRMAX)
          ERRMAX = ELIST(MAXERR)
C ***JUMP OUT OF DO-LOOP
          IF(LEVEL(MAXERR)+1.LE.LEVMAX) GO TO 160
          NRMAX = NRMAX+1
  130   CONTINUE
C
C           PERFORM EXTRAPOLATION.
C
  140   NUMRL2 = NUMRL2+1
        RLIST2(NUMRL2) = AREA
        IF(NUMRL2.LE.2) GO TO 155
        CALL DQELG(NUMRL2,RLIST2,RESEPS,ABSEPS,RES3LA,NRES)
        KTMIN = KTMIN+1
        IF(KTMIN.GT.5.AND.ABSERR.LT.0.1D-02*ERRSUM) IER = 5
        IF(ABSEPS.GE.ABSERR) GO TO 150
        KTMIN = 0
        ABSERR = ABSEPS
        RESULT = RESEPS
        CORREC = ERLARG
        ERTEST = DMAX1(EPSABS,EPSREL*DABS(RESEPS))
C ***JUMP OUT OF DO-LOOP
        IF(ABSERR.LT.ERTEST) GO TO 170
C
C           PREPARE BISECTION OF THE SMALLEST INTERVAL.
C
  150   IF(NUMRL2.EQ.1) NOEXT = .TRUE.
        IF(IER.GE.5) GO TO 170
  155   MAXERR = IORD(1)
        ERRMAX = ELIST(MAXERR)
        NRMAX = 1
        EXTRAP = .FALSE.
        LEVMAX = LEVMAX+1
        ERLARG = ERRSUM
  160 CONTINUE
C
C           SET THE FINAL RESULT.
C           ---------------------
C
C
  170 IF(ABSERR.EQ.OFLOW) GO TO 190
      IF((IER+IERRO).EQ.0) GO TO 180
      IF(IERRO.EQ.3) ABSERR = ABSERR+CORREC
      IF(IER.EQ.0) IER = 3
      IF(RESULT.NE.0.0D+00.AND.AREA.NE.0.0D+00)GO TO 175
      IF(ABSERR.GT.ERRSUM)GO TO 190
      IF(AREA.EQ.0.0D+00) GO TO 210
      GO TO 180
  175 IF(ABSERR/DABS(RESULT).GT.ERRSUM/DABS(AREA))GO TO 190
C
C           TEST ON DIVERGENCE.
C
  180 IF(KSGN.EQ.(-1).AND.DMAX1(DABS(RESULT),DABS(AREA)).LE.
     *  RESABS*0.1D-01) GO TO 210
      IF(0.1D-01.GT.(RESULT/AREA).OR.(RESULT/AREA).GT.0.1D+03.OR.
     *  ERRSUM.GT.DABS(AREA)) IER = 6
      GO TO 210
C
C           COMPUTE GLOBAL INTEGRAL SUM.
C
  190 RESULT = 0.0D+00
      DO 200 K = 1,LAST
        RESULT = RESULT+RLIST(K)
  200 CONTINUE
      ABSERR = ERRSUM
  210 IF(IER.GT.2) IER = IER-1
      RESULT = RESULT*SIGN
  999 RETURN
      END
