      SUBROUTINE ODESSA_STESA (NEQ, Y, NROW, NCOL, YH, WM, IWM, EWT, 
     1  SAVF, ACOR, PAR, NRS, F, JAC, DF, PJAC, PDF, SOLVE)
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      EXTERNAL F, JAC, DF, PJAC, PDF, SOLVE
      DIMENSION NEQ(*), Y(NROW,*), YH(NROW,NCOL,*), WM(*), IWM(*),
     1   EWT(NROW,*), SAVF(*), ACOR(NROW,*), PAR(*), NRS(*)
      PARAMETER (ONE=1.0D0,ZERO=0.0D0)
      COMMON /ODE001/ ROWND, ROWNS(173),
     1   TESCO(3,12), RDUM1, EL0, H, RDUM2(4), TN, RDUM3,
     2   IOWND1(14), IOWNS(4),
     3   IALTH, LMAX, IDUM1, IERPJ, IERSL, JCUR, IDUM2, KFLAG, L, IDUM3,
     4   MITER, IDUM4(4), N, NQ, IDUM5, NFE, IDUM6(2)
      COMMON /ODE002/ DUPS, DSMS, DDNS,
     1   IOWND2(3), IDUM7, NSV, IDUM8(2), IDF, IDUM9, JOPT, KFLAGS
C-----------------------------------------------------------------------
C ODESSA_STESA IS CALLED BY ODESSA_STODE TO PERFORM AN EXPLICIT
C CALCULATION FOR THE FIRST-ORDER SENSITIVITY COEFFICIENTS DY(I)/DP(J),
C I = 1,N; J = 1,NPAR. 
C
C IN ADDITION TO THE VARIABLES DESCRIBED PREVIOUSLY, COMMUNICATION
C WITH ODESSA_STESA USES THE FOLLOWING..
C Y      = AN NROW (=N) BY NCOL (=NSV) REAL ARRAY CONTAINING THE
C          CORRECTED DEPENDENT VARIABLES ON OUTPUT..
C                  Y(I,1) , I = 1,N = STATE VARIABLES (INPUT);
C                  Y(I,J) , I = 1,N , J = 2,NSV ,
C                           = SENSITIVITY COEFFICIENTS, DY(I)/DP(J).
C YH     = AN N BY NSV BY LMAX REAL ARRAY CONTAINING THE PREDICTED
C          DEPENDENT VARIABLES AND THEIR APPROXIMATE SCALED DERIVATIVES.
C SAVF   = A REAL ARRAY OF LENGTH N USED TO STORE FIRST DERIVATIVES
C          OF DEPENDENT VARIABLES IF MITER = 2 OR 5.
C PAR    = A REAL ARRAY OF LENGTH NPAR CONTAINING THE EQUATION
C          PARAMETERS OF INTEREST.
C NRS    = AN INTEGER ARRAY OF LENGTH NPAR + 1 CONTAINING THE NUMBER
C          OF REPEATED STEPS (KFLAGS .LT. 0) DUE TO THE SENSITIVITY
C          CALCULATIONS..
C                  NRS(1) = TOTAL NUMBER OF REPEATED STEPS
C                  NRS(I) , I = 2,NPAR = NUMBER OF REPEATED STEPS DUE
C                                        TO PARAMETER I.
C NSV    = NUMBER OF SOLUTION VECTORS = NPAR + 1.
C KFLAGS = LOCAL ERROR TEST FLAG, = 0 IF TEST PASSES, .LT. 0 IF TEST
C          FAILS, AND STEP NEEDS TO BE REPEATED. ERROR TEST IS APPLIED
C          TO EACH SOLUTION VECTOR INDEPENDENTLY.
C DUPS, DSMS, DDNS = REAL SCALARS USED FOR COMPUTING RHUP, RHSM, RHDN,
C                    ON RETURN TO ODESSA_STODE (IALTH .EQ. 1).
C THIS ROUTINE ALSO USES THE COMMON VARIABLES EL0, H, TN, IALTH, LMAX,
C IERPJ, IERSL, JCUR, KFLAG, L, MITER, N, NQ, NFE, AND JOPT.
C-----------------------------------------------------------------------
      DUPS = ZERO
      DSMS = ZERO
      DDNS = ZERO
      HL0 = H*EL0
      EL0I = ONE/EL0
      TI2 = ONE/TESCO(2,NQ)
      TI3 = ONE/TESCO(3,NQ)
C IF MITER = 2 OR 5 (OR IDF = 0), SUPPLY DERIVATIVES AT CORRECTED
C Y(*,1) VALUES FOR NUMERICAL DIFFERENTIATION IN PJAC AND/OR PDF.
      IF (MITER .EQ. 2  .OR.  MITER .EQ. 5  .OR.  IDF .EQ. 0)  GO TO 10
      GO TO 15
 10   CALL F (NEQ, TN, Y, PAR, SAVF)
      NFE = NFE + 1
C IF JCUR = 0, UPDATE THE JACOBIAN MATRIX.
C IF MITER = 5, LOAD CORRECTED Y(*,1) VALUES INTO Y(*,2).
 15   IF (JCUR .EQ. 1) GO TO 30
      IF (MITER .NE. 5) GO TO 25
      DO 20 I = 1,N
 20     Y(I,2) = Y(I,1)
 25   CALL PJAC (NEQ, Y, Y(1,2), N, WM, IWM, EWT, SAVF, ACOR(1,2),
     1   PAR, F, JAC, JOPT)
      IF (IERPJ .NE. 0) RETURN
C-----------------------------------------------------------------------
C THIS IS A LOOPING POINT FOR THE SENSITIVITY CALCULATIONS.
C-----------------------------------------------------------------------
C FOR EACH PARAMETER PAR(*), A SENSITIVITY SOLUTION VECTOR IS COMPUTED
C USING THE SAME STEP SIZE (H) AND ORDER (NQ) AS IN ODESSA_STODE.
C A LOCAL ERROR TEST IS APPLIED INDEPENDENTLY TO EACH SOLUTION VECTOR.
C-----------------------------------------------------------------------
 30   DO 100 J = 2,NSV
        JPAR = J - 1
C EVALUATE INHOMOGENEITY TERM, TEMPORARILY LOAD INTO Y(*,JPAR+1). ------
        CALL PDF(NEQ, Y, WM, SAVF, ACOR(1,J), Y(1,J), PAR,
     1     F, DF, JPAR)
C-----------------------------------------------------------------------
C LOAD RHS OF SENSITIVITY SOLUTION (CORRECTOR) EQUATION..
C
C       RHS = DY/DP - EL(1)*H*D(DY/DP)/DT + EL(1)*H*DF/DP
C
C-----------------------------------------------------------------------
        DO 40 I = 1,N
 40       Y(I,J) = YH(I,J,1) - EL0*YH(I,J,2) + HL0*Y(I,J)
C-----------------------------------------------------------------------
C SOLVE CORRECTOR EQUATION: THE SOLUTIONS ARE LOCATED IN Y(*,JPAR+1).
C THE EXPLICIT FORMULA IS..
C
C       (I - EL(1)*H*JAC) * DY/DP(CORRECTED) = RHS
C
C-----------------------------------------------------------------------
        CALL SOLVE (WM, IWM, Y(1,J), DUM)
        IF (IERSL .NE. 0) RETURN
C ESTIMATE LOCAL TRUNCATION ERROR. -------------------------------------
        DO 50 I = 1,N
 50       ACOR(I,J) = (Y(I,J) - YH(I,J,1))*EL0I
        ERR = ODESSA_VNORM(N, ACOR(1,J), EWT(1,J))*TI2
        IF (ERR .GT. ONE) GO TO 200
C-----------------------------------------------------------------------
C LOCAL ERROR TEST PASSED. SET KFLAGS TO 0 TO INDICATE THIS.
C IF IALTH = 1, COMPUTE DSMS, DDNS, AND DUPS (IF L .LT. LMAX).
C-----------------------------------------------------------------------
        KFLAGS = 0
        IF (IALTH .GT. 1) GO TO 100
        IF (L .EQ. LMAX) GO TO 70
        DO 60 I= 1,N
 60       Y(I,J) = ACOR(I,J) - YH(I,J,LMAX)
        DUPS = MAX(DUPS,ODESSA_VNORM(N,Y(1,J),EWT(1,J))*TI3)
 70     DSMS = MAX(DSMS,ERR)
 100  CONTINUE
      RETURN
C-----------------------------------------------------------------------
C THIS SECTION IS REACHED IF THE ERROR TOLERANCE FOR SENSITIVITY
C SOLUTION VECTOR JPAR HAS BEEN VIOLATED. KFLAGS IS MADE NEGATIVE TO
C INDICATE THIS. IF KFLAGS = -1, SET KFLAG EQUAL TO ZERO SO THAT KFLAG
C IS SET TO -1 ON RETURN TO ODESSA_STODE BEFORE REPEATING THE STEP.
C INCREMENT NRS(1) (= TOTAL NUMBER OF REPEATED STEPS DUE TO ALL
C SENSITIVITY SOLUTION VECTORS) BY ONE.
C INCREMENT NRS(JPAR+1) (= TOTAL NUMBER OF REPEATED STEPS DUE TO
C SOLUTION VECTOR JPAR+1) BY ONE.
C LOAD DSMS FOR RH CALCULATION IN ODESSA_STODE.
C-----------------------------------------------------------------------
 200  KFLAGS = KFLAGS - 1
      IF (KFLAGS .EQ. -1) KFLAG = 0
      NRS(1) = NRS(1) + 1
      NRS(J) = NRS(J) + 1
      DSMS = ERR
      RETURN
C-------------------- END OF SUBROUTINE ODESSA_STESA ----------------------
      END
