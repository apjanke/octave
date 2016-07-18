      SUBROUTINE STODE (NEQ, Y, YH, NYH, YH1, EWT, SAVF, ACOR,
     1   WM, IWM, F, JAC, PJAC, SLVS, IERR)
CLLL. OPTIMIZE
      EXTERNAL F, JAC, PJAC, SLVS
      INTEGER NEQ, NYH, IWM
      INTEGER ILLIN, INIT, LYH, LEWT, LACOR, LSAVF, LWM, LIWM,
     1   MXSTEP, MXHNIL, NHNIL, NTREP, NSLAST, CNYH,
     2   IALTH, IPUP, LMAX, MEO, NQNYH, NSLP
      INTEGER ICF, IERPJ, IERSL, JCUR, JSTART, KFLAG, L, METH, MITER,
     1   MAXORD, MAXCOR, MSBP, MXNCF, N, NQ, NST, NFE, NJE, NQU
      INTEGER I, I1, IREDO, IRET, J, JB, M, NCF, NEWQ
      DOUBLE PRECISION Y, YH, YH1, EWT, SAVF, ACOR, WM
      DOUBLE PRECISION CONIT, CRATE, EL, ELCO, HOLD, RMAX, TESCO,
     2   CCMAX, EL0, H, HMIN, HMXI, HU, RC, TN, UROUND
      DOUBLE PRECISION DCON, DDN, DEL, DELP, DSM, DUP, EXDN, EXSM, EXUP,
     1   R, RH, RHDN, RHSM, RHUP, TOLD, VNORM
      DIMENSION NEQ(*), Y(*), YH(NYH,*), YH1(*), EWT(*), SAVF(*),
     1   ACOR(*), WM(*), IWM(*)
      COMMON /LS0001/ CONIT, CRATE, EL(13), ELCO(13,12),
     1   HOLD, RMAX, TESCO(3,12),
     2   CCMAX, EL0, H, HMIN, HMXI, HU, RC, TN, UROUND,
     2   ILLIN, INIT, LYH, LEWT, LACOR, LSAVF, LWM, LIWM,
     3   MXSTEP, MXHNIL, NHNIL, NTREP, NSLAST, CNYH,
     3   IALTH, IPUP, LMAX, MEO, NQNYH, NSLP,
     4   ICF, IERPJ, IERSL, JCUR, JSTART, KFLAG, L, METH, MITER,
     5   MAXORD, MAXCOR, MSBP, MXNCF, N, NQ, NST, NFE, NJE, NQU
C-----------------------------------------------------------------------
C STODE PERFORMS ONE STEP OF THE INTEGRATION OF AN INITIAL VALUE
C PROBLEM FOR A SYSTEM OF ORDINARY DIFFERENTIAL EQUATIONS.
C NOTE.. STODE IS INDEPENDENT OF THE VALUE OF THE ITERATION METHOD
C INDICATOR MITER, WHEN THIS IS .NE. 0, AND HENCE IS INDEPENDENT
C OF THE TYPE OF CHORD METHOD USED, OR THE JACOBIAN STRUCTURE.
C COMMUNICATION WITH STODE IS DONE WITH THE FOLLOWING VARIABLES..
C
C NEQ    = INTEGER ARRAY CONTAINING PROBLEM SIZE IN NEQ(1), AND
C          PASSED AS THE NEQ ARGUMENT IN ALL CALLS TO F AND JAC.
C Y      = AN ARRAY OF LENGTH .GE. N USED AS THE Y ARGUMENT IN
C          ALL CALLS TO F AND JAC.
C YH     = AN NYH BY LMAX ARRAY CONTAINING THE DEPENDENT VARIABLES
C          AND THEIR APPROXIMATE SCALED DERIVATIVES, WHERE
C          LMAX = MAXORD + 1.  YH(I,J+1) CONTAINS THE APPROXIMATE
C          J-TH DERIVATIVE OF Y(I), SCALED BY H**J/FACTORIAL(J)
C          (J = 0,1,...,NQ).  ON ENTRY FOR THE FIRST STEP, THE FIRST
C          TWO COLUMNS OF YH MUST BE SET FROM THE INITIAL VALUES.
C NYH    = A CONSTANT INTEGER .GE. N, THE FIRST DIMENSION OF YH.
C YH1    = A ONE-DIMENSIONAL ARRAY OCCUPYING THE SAME SPACE AS YH.
C EWT    = AN ARRAY OF LENGTH N CONTAINING MULTIPLICATIVE WEIGHTS
C          FOR LOCAL ERROR MEASUREMENTS.  LOCAL ERRORS IN Y(I) ARE
C          COMPARED TO 1.0/EWT(I) IN VARIOUS ERROR TESTS.
C SAVF   = AN ARRAY OF WORKING STORAGE, OF LENGTH N.
C          ALSO USED FOR INPUT OF YH(*,MAXORD+2) WHEN JSTART = -1
C          AND MAXORD .LT. THE CURRENT ORDER NQ.
C ACOR   = A WORK ARRAY OF LENGTH N, USED FOR THE ACCUMULATED
C          CORRECTIONS.  ON A SUCCESSFUL RETURN, ACOR(I) CONTAINS
C          THE ESTIMATED ONE-STEP LOCAL ERROR IN Y(I).
C WM,IWM = REAL AND INTEGER WORK ARRAYS ASSOCIATED WITH MATRIX
C          OPERATIONS IN CHORD ITERATION (MITER .NE. 0).
C PJAC   = NAME OF ROUTINE TO EVALUATE AND PREPROCESS JACOBIAN MATRIX
C          AND P = I - H*EL0*JAC, IF A CHORD METHOD IS BEING USED.
C SLVS   = NAME OF ROUTINE TO SOLVE LINEAR SYSTEM IN CHORD ITERATION.
C CCMAX  = MAXIMUM RELATIVE CHANGE IN H*EL0 BEFORE PJAC IS CALLED.
C H      = THE STEP SIZE TO BE ATTEMPTED ON THE NEXT STEP.
C          H IS ALTERED BY THE ERROR CONTROL ALGORITHM DURING THE
C          PROBLEM.  H CAN BE EITHER POSITIVE OR NEGATIVE, BUT ITS
C          SIGN MUST REMAIN CONSTANT THROUGHOUT THE PROBLEM.
C HMIN   = THE MINIMUM ABSOLUTE VALUE OF THE STEP SIZE H TO BE USED.
C HMXI   = INVERSE OF THE MAXIMUM ABSOLUTE VALUE OF H TO BE USED.
C          HMXI = 0.0 IS ALLOWED AND CORRESPONDS TO AN INFINITE HMAX.
C          HMIN AND HMXI MAY BE CHANGED AT ANY TIME, BUT WILL NOT
C          TAKE EFFECT UNTIL THE NEXT CHANGE OF H IS CONSIDERED.
C TN     = THE INDEPENDENT VARIABLE. TN IS UPDATED ON EACH STEP TAKEN.
C JSTART = AN INTEGER USED FOR INPUT ONLY, WITH THE FOLLOWING
C          VALUES AND MEANINGS..
C               0  PERFORM THE FIRST STEP.
C           .GT.0  TAKE A NEW STEP CONTINUING FROM THE LAST.
C              -1  TAKE THE NEXT STEP WITH A NEW VALUE OF H, MAXORD,
C                    N, METH, MITER, AND/OR MATRIX PARAMETERS.
C              -2  TAKE THE NEXT STEP WITH A NEW VALUE OF H,
C                    BUT WITH OTHER INPUTS UNCHANGED.
C          ON RETURN, JSTART IS SET TO 1 TO FACILITATE CONTINUATION.
C KFLAG  = A COMPLETION CODE WITH THE FOLLOWING MEANINGS..
C               0  THE STEP WAS SUCCESFUL.
C              -1  THE REQUESTED ERROR COULD NOT BE ACHIEVED.
C              -2  CORRECTOR CONVERGENCE COULD NOT BE ACHIEVED.
C              -3  FATAL ERROR IN PJAC OR SLVS.
C          A RETURN WITH KFLAG = -1 OR -2 MEANS EITHER
C          ABS(H) = HMIN OR 10 CONSECUTIVE FAILURES OCCURRED.
C          ON A RETURN WITH KFLAG NEGATIVE, THE VALUES OF TN AND
C          THE YH ARRAY ARE AS OF THE BEGINNING OF THE LAST
C          STEP, AND H IS THE LAST STEP SIZE ATTEMPTED.
C MAXORD = THE MAXIMUM ORDER OF INTEGRATION METHOD TO BE ALLOWED.
C MAXCOR = THE MAXIMUM NUMBER OF CORRECTOR ITERATIONS ALLOWED.
C MSBP   = MAXIMUM NUMBER OF STEPS BETWEEN PJAC CALLS (MITER .GT. 0).
C MXNCF  = MAXIMUM NUMBER OF CONVERGENCE FAILURES ALLOWED.
C METH/MITER = THE METHOD FLAGS.  SEE DESCRIPTION IN DRIVER.
C N      = THE NUMBER OF FIRST-ORDER DIFFERENTIAL EQUATIONS.
C IERR   = ERROR FLAG FROM USER-SUPPLIED FUNCTION
C-----------------------------------------------------------------------
      KFLAG = 0
      TOLD = TN
      NCF = 0
      IERPJ = 0
      IERSL = 0
      JCUR = 0
      ICF = 0
      DELP = 0.0D0
      IF (JSTART .GT. 0) GO TO 200
      IF (JSTART .EQ. -1) GO TO 100
      IF (JSTART .EQ. -2) GO TO 160
C-----------------------------------------------------------------------
C ON THE FIRST CALL, THE ORDER IS SET TO 1, AND OTHER VARIABLES ARE
C INITIALIZED.  RMAX IS THE MAXIMUM RATIO BY WHICH H CAN BE INCREASED
C IN A SINGLE STEP.  IT IS INITIALLY 1.E4 TO COMPENSATE FOR THE SMALL
C INITIAL H, BUT THEN IS NORMALLY EQUAL TO 10.  IF A FAILURE
C OCCURS (IN CORRECTOR CONVERGENCE OR ERROR TEST), RMAX IS SET AT 2
C FOR THE NEXT INCREASE.
C-----------------------------------------------------------------------
      LMAX = MAXORD + 1
      NQ = 1
      L = 2
      IALTH = 2
      RMAX = 10000.0D0
      RC = 0.0D0
      EL0 = 1.0D0
      CRATE = 0.7D0
      HOLD = H
      MEO = METH
      NSLP = 0
      IPUP = MITER
      IRET = 3
      GO TO 140
C-----------------------------------------------------------------------
C THE FOLLOWING BLOCK HANDLES PRELIMINARIES NEEDED WHEN JSTART = -1.
C IPUP IS SET TO MITER TO FORCE A MATRIX UPDATE.
C IF AN ORDER INCREASE IS ABOUT TO BE CONSIDERED (IALTH = 1),
C IALTH IS RESET TO 2 TO POSTPONE CONSIDERATION ONE MORE STEP.
C IF THE CALLER HAS CHANGED METH, CFODE IS CALLED TO RESET
C THE COEFFICIENTS OF THE METHOD.
C IF THE CALLER HAS CHANGED MAXORD TO A VALUE LESS THAN THE CURRENT
C ORDER NQ, NQ IS REDUCED TO MAXORD, AND A NEW H CHOSEN ACCORDINGLY.
C IF H IS TO BE CHANGED, YH MUST BE RESCALED.
C IF H OR METH IS BEING CHANGED, IALTH IS RESET TO L = NQ + 1
C TO PREVENT FURTHER CHANGES IN H FOR THAT MANY STEPS.
C-----------------------------------------------------------------------
 100  IPUP = MITER
      LMAX = MAXORD + 1
      IF (IALTH .EQ. 1) IALTH = 2
      IF (METH .EQ. MEO) GO TO 110
      CALL CFODE (METH, ELCO, TESCO)
      MEO = METH
      IF (NQ .GT. MAXORD) GO TO 120
      IALTH = L
      IRET = 1
      GO TO 150
 110  IF (NQ .LE. MAXORD) GO TO 160
 120  NQ = MAXORD
      L = LMAX
      DO 125 I = 1,L
 125    EL(I) = ELCO(I,NQ)
      NQNYH = NQ*NYH
      RC = RC*EL(1)/EL0
      EL0 = EL(1)
      CONIT = 0.5D0/DBLE(NQ+2)
      DDN = VNORM (N, SAVF, EWT)/TESCO(1,L)
      EXDN = 1.0D0/DBLE(L)
      RHDN = 1.0D0/(1.3D0*DDN**EXDN + 0.0000013D0)
      RH = DMIN1(RHDN,1.0D0)
      IREDO = 3
      IF (H .EQ. HOLD) GO TO 170
      RH = DMIN1(RH,DABS(H/HOLD))
      H = HOLD
      GO TO 175
C-----------------------------------------------------------------------
C CFODE IS CALLED TO GET ALL THE INTEGRATION COEFFICIENTS FOR THE
C CURRENT METH.  THEN THE EL VECTOR AND RELATED CONSTANTS ARE RESET
C WHENEVER THE ORDER NQ IS CHANGED, OR AT THE START OF THE PROBLEM.
C-----------------------------------------------------------------------
 140  CALL CFODE (METH, ELCO, TESCO)
 150  DO 155 I = 1,L
 155    EL(I) = ELCO(I,NQ)
      NQNYH = NQ*NYH
      RC = RC*EL(1)/EL0
      EL0 = EL(1)
      CONIT = 0.5D0/DBLE(NQ+2)
      GO TO (160, 170, 200), IRET
C-----------------------------------------------------------------------
C IF H IS BEING CHANGED, THE H RATIO RH IS CHECKED AGAINST
C RMAX, HMIN, AND HMXI, AND THE YH ARRAY RESCALED.  IALTH IS SET TO
C L = NQ + 1 TO PREVENT A CHANGE OF H FOR THAT MANY STEPS, UNLESS
C FORCED BY A CONVERGENCE OR ERROR TEST FAILURE.
C-----------------------------------------------------------------------
 160  IF (H .EQ. HOLD) GO TO 200
      RH = H/HOLD
      H = HOLD
      IREDO = 3
      GO TO 175
 170  RH = DMAX1(RH,HMIN/DABS(H))
 175  RH = DMIN1(RH,RMAX)
      RH = RH/DMAX1(1.0D0,DABS(H)*HMXI*RH)
      R = 1.0D0
      DO 180 J = 2,L
        R = R*RH
        DO 180 I = 1,N
 180      YH(I,J) = YH(I,J)*R
      H = H*RH
      RC = RC*RH
      IALTH = L
      IF (IREDO .EQ. 0) GO TO 690
C-----------------------------------------------------------------------
C THIS SECTION COMPUTES THE PREDICTED VALUES BY EFFECTIVELY
C MULTIPLYING THE YH ARRAY BY THE PASCAL TRIANGLE MATRIX.
C RC IS THE RATIO OF NEW TO OLD VALUES OF THE COEFFICIENT  H*EL(1).
C WHEN RC DIFFERS FROM 1 BY MORE THAN CCMAX, IPUP IS SET TO MITER
C TO FORCE PJAC TO BE CALLED, IF A JACOBIAN IS INVOLVED.
C IN ANY CASE, PJAC IS CALLED AT LEAST EVERY MSBP STEPS.
C-----------------------------------------------------------------------
 200  IF (DABS(RC-1.0D0) .GT. CCMAX) IPUP = MITER
      IF (NST .GE. NSLP+MSBP) IPUP = MITER
      TN = TN + H
      I1 = NQNYH + 1
      DO 215 JB = 1,NQ
        I1 = I1 - NYH
CDIR$ IVDEP
        DO 210 I = I1,NQNYH
 210      YH1(I) = YH1(I) + YH1(I+NYH)
 215    CONTINUE
C-----------------------------------------------------------------------
C UP TO MAXCOR CORRECTOR ITERATIONS ARE TAKEN.  A CONVERGENCE TEST IS
C MADE ON THE R.M.S. NORM OF EACH CORRECTION, WEIGHTED BY THE ERROR
C WEIGHT VECTOR EWT.  THE SUM OF THE CORRECTIONS IS ACCUMULATED IN THE
C VECTOR ACOR(I).  THE YH ARRAY IS NOT ALTERED IN THE CORRECTOR LOOP.
C-----------------------------------------------------------------------
 220  M = 0
      DO 230 I = 1,N
 230    Y(I) = YH(I,1)
      IERR = 0
      CALL F (NEQ, TN, Y, SAVF, IERR)
      IF (IERR .LT. 0) RETURN
      NFE = NFE + 1
      IF (IPUP .LE. 0) GO TO 250
C-----------------------------------------------------------------------
C IF INDICATED, THE MATRIX P = I - H*EL(1)*J IS REEVALUATED AND
C PREPROCESSED BEFORE STARTING THE CORRECTOR ITERATION.  IPUP IS SET
C TO 0 AS AN INDICATOR THAT THIS HAS BEEN DONE.
C-----------------------------------------------------------------------
      IERR = 0
      CALL PJAC (NEQ, Y, YH, NYH, EWT, ACOR, SAVF, WM, IWM, F, JAC,
     1   IERR)
      IF (IERR .LT. 0) RETURN
      IPUP = 0
      RC = 1.0D0
      NSLP = NST
      CRATE = 0.7D0
      IF (IERPJ .NE. 0) GO TO 430
 250  DO 260 I = 1,N
 260    ACOR(I) = 0.0D0
 270  IF (MITER .NE. 0) GO TO 350
C-----------------------------------------------------------------------
C IN THE CASE OF FUNCTIONAL ITERATION, UPDATE Y DIRECTLY FROM
C THE RESULT OF THE LAST FUNCTION EVALUATION.
C-----------------------------------------------------------------------
      DO 290 I = 1,N
        SAVF(I) = H*SAVF(I) - YH(I,2)
 290    Y(I) = SAVF(I) - ACOR(I)
      DEL = VNORM (N, Y, EWT)
      DO 300 I = 1,N
        Y(I) = YH(I,1) + EL(1)*SAVF(I)
 300    ACOR(I) = SAVF(I)
      GO TO 400
C-----------------------------------------------------------------------
C IN THE CASE OF THE CHORD METHOD, COMPUTE THE CORRECTOR ERROR,
C AND SOLVE THE LINEAR SYSTEM WITH THAT AS RIGHT-HAND SIDE AND
C P AS COEFFICIENT MATRIX.
C-----------------------------------------------------------------------
 350  DO 360 I = 1,N
 360    Y(I) = H*SAVF(I) - (YH(I,2) + ACOR(I))
      CALL SLVS (WM, IWM, Y, SAVF)
      IF (IERSL .LT. 0) GO TO 430
      IF (IERSL .GT. 0) GO TO 410
      DEL = VNORM (N, Y, EWT)
      DO 380 I = 1,N
        ACOR(I) = ACOR(I) + Y(I)
 380    Y(I) = YH(I,1) + EL(1)*ACOR(I)
C-----------------------------------------------------------------------
C TEST FOR CONVERGENCE.  IF M.GT.0, AN ESTIMATE OF THE CONVERGENCE
C RATE CONSTANT IS STORED IN CRATE, AND THIS IS USED IN THE TEST.
C-----------------------------------------------------------------------
 400  IF (M .NE. 0) CRATE = DMAX1(0.2D0*CRATE,DEL/DELP)
      DCON = DEL*DMIN1(1.0D0,1.5D0*CRATE)/(TESCO(2,NQ)*CONIT)
      IF (DCON .LE. 1.0D0) GO TO 450
      M = M + 1
      IF (M .EQ. MAXCOR) GO TO 410
      IF (M .GE. 2 .AND. DEL .GT. 2.0D0*DELP) GO TO 410
      DELP = DEL
      IERR = 0
      CALL F (NEQ, TN, Y, SAVF, IERR)
      IF (IERR .LT. 0) RETURN
      NFE = NFE + 1
      GO TO 270
C-----------------------------------------------------------------------
C THE CORRECTOR ITERATION FAILED TO CONVERGE.
C IF MITER .NE. 0 AND THE JACOBIAN IS OUT OF DATE, PJAC IS CALLED FOR
C THE NEXT TRY.  OTHERWISE THE YH ARRAY IS RETRACTED TO ITS VALUES
C BEFORE PREDICTION, AND H IS REDUCED, IF POSSIBLE.  IF H CANNOT BE
C REDUCED OR MXNCF FAILURES HAVE OCCURRED, EXIT WITH KFLAG = -2.
C-----------------------------------------------------------------------
 410  IF (MITER .EQ. 0 .OR. JCUR .EQ. 1) GO TO 430
      ICF = 1
      IPUP = MITER
      GO TO 220
 430  ICF = 2
      NCF = NCF + 1
      RMAX = 2.0D0
      TN = TOLD
      I1 = NQNYH + 1
      DO 445 JB = 1,NQ
        I1 = I1 - NYH
CDIR$ IVDEP
        DO 440 I = I1,NQNYH
 440      YH1(I) = YH1(I) - YH1(I+NYH)
 445    CONTINUE
      IF (IERPJ .LT. 0 .OR. IERSL .LT. 0) GO TO 680
      IF (DABS(H) .LE. HMIN*1.00001D0) GO TO 670
      IF (NCF .EQ. MXNCF) GO TO 670
      RH = 0.25D0
      IPUP = MITER
      IREDO = 1
      GO TO 170
C-----------------------------------------------------------------------
C THE CORRECTOR HAS CONVERGED.  JCUR IS SET TO 0
C TO SIGNAL THAT THE JACOBIAN INVOLVED MAY NEED UPDATING LATER.
C THE LOCAL ERROR TEST IS MADE AND CONTROL PASSES TO STATEMENT 500
C IF IT FAILS.
C-----------------------------------------------------------------------
 450  JCUR = 0
      IF (M .EQ. 0) DSM = DEL/TESCO(2,NQ)
      IF (M .GT. 0) DSM = VNORM (N, ACOR, EWT)/TESCO(2,NQ)
      IF (DSM .GT. 1.0D0) GO TO 500
C-----------------------------------------------------------------------
C AFTER A SUCCESSFUL STEP, UPDATE THE YH ARRAY.
C CONSIDER CHANGING H IF IALTH = 1.  OTHERWISE DECREASE IALTH BY 1.
C IF IALTH IS THEN 1 AND NQ .LT. MAXORD, THEN ACOR IS SAVED FOR
C USE IN A POSSIBLE ORDER INCREASE ON THE NEXT STEP.
C IF A CHANGE IN H IS CONSIDERED, AN INCREASE OR DECREASE IN ORDER
C BY ONE IS CONSIDERED ALSO.  A CHANGE IN H IS MADE ONLY IF IT IS BY A
C FACTOR OF AT LEAST 1.1.  IF NOT, IALTH IS SET TO 3 TO PREVENT
C TESTING FOR THAT MANY STEPS.
C-----------------------------------------------------------------------
      KFLAG = 0
      IREDO = 0
      NST = NST + 1
      HU = H
      NQU = NQ
      DO 470 J = 1,L
        DO 470 I = 1,N
 470      YH(I,J) = YH(I,J) + EL(J)*ACOR(I)
      IALTH = IALTH - 1
      IF (IALTH .EQ. 0) GO TO 520
      IF (IALTH .GT. 1) GO TO 700
      IF (L .EQ. LMAX) GO TO 700
      DO 490 I = 1,N
 490    YH(I,LMAX) = ACOR(I)
      GO TO 700
C-----------------------------------------------------------------------
C THE ERROR TEST FAILED.  KFLAG KEEPS TRACK OF MULTIPLE FAILURES.
C RESTORE TN AND THE YH ARRAY TO THEIR PREVIOUS VALUES, AND PREPARE
C TO TRY THE STEP AGAIN.  COMPUTE THE OPTIMUM STEP SIZE FOR THIS OR
C ONE LOWER ORDER.  AFTER 2 OR MORE FAILURES, H IS FORCED TO DECREASE
C BY A FACTOR OF 0.2 OR LESS.
C-----------------------------------------------------------------------
 500  KFLAG = KFLAG - 1
      TN = TOLD
      I1 = NQNYH + 1
      DO 515 JB = 1,NQ
        I1 = I1 - NYH
CDIR$ IVDEP
        DO 510 I = I1,NQNYH
 510      YH1(I) = YH1(I) - YH1(I+NYH)
 515    CONTINUE
      RMAX = 2.0D0
      IF (DABS(H) .LE. HMIN*1.00001D0) GO TO 660
      IF (KFLAG .LE. -3) GO TO 640
      IREDO = 2
      RHUP = 0.0D0
      GO TO 540
C-----------------------------------------------------------------------
C REGARDLESS OF THE SUCCESS OR FAILURE OF THE STEP, FACTORS
C RHDN, RHSM, AND RHUP ARE COMPUTED, BY WHICH H COULD BE MULTIPLIED
C AT ORDER NQ - 1, ORDER NQ, OR ORDER NQ + 1, RESPECTIVELY.
C IN THE CASE OF FAILURE, RHUP = 0.0 TO AVOID AN ORDER INCREASE.
C THE LARGEST OF THESE IS DETERMINED AND THE NEW ORDER CHOSEN
C ACCORDINGLY.  IF THE ORDER IS TO BE INCREASED, WE COMPUTE ONE
C ADDITIONAL SCALED DERIVATIVE.
C-----------------------------------------------------------------------
 520  RHUP = 0.0D0
      IF (L .EQ. LMAX) GO TO 540
      DO 530 I = 1,N
 530    SAVF(I) = ACOR(I) - YH(I,LMAX)
      DUP = VNORM (N, SAVF, EWT)/TESCO(3,NQ)
      EXUP = 1.0D0/DBLE(L+1)
      RHUP = 1.0D0/(1.4D0*DUP**EXUP + 0.0000014D0)
 540  EXSM = 1.0D0/DBLE(L)
      RHSM = 1.0D0/(1.2D0*DSM**EXSM + 0.0000012D0)
      RHDN = 0.0D0
      IF (NQ .EQ. 1) GO TO 560
      DDN = VNORM (N, YH(1,L), EWT)/TESCO(1,NQ)
      EXDN = 1.0D0/DBLE(NQ)
      RHDN = 1.0D0/(1.3D0*DDN**EXDN + 0.0000013D0)
 560  IF (RHSM .GE. RHUP) GO TO 570
      IF (RHUP .GT. RHDN) GO TO 590
      GO TO 580
 570  IF (RHSM .LT. RHDN) GO TO 580
      NEWQ = NQ
      RH = RHSM
      GO TO 620
 580  NEWQ = NQ - 1
      RH = RHDN
      IF (KFLAG .LT. 0 .AND. RH .GT. 1.0D0) RH = 1.0D0
      GO TO 620
 590  NEWQ = L
      RH = RHUP
      IF (RH .LT. 1.1D0) GO TO 610
      R = EL(L)/DBLE(L)
      DO 600 I = 1,N
 600    YH(I,NEWQ+1) = ACOR(I)*R
      GO TO 630
 610  IALTH = 3
      GO TO 700
 620  IF ((KFLAG .EQ. 0) .AND. (RH .LT. 1.1D0)) GO TO 610
      IF (KFLAG .LE. -2) RH = DMIN1(RH,0.2D0)
C-----------------------------------------------------------------------
C IF THERE IS A CHANGE OF ORDER, RESET NQ, L, AND THE COEFFICIENTS.
C IN ANY CASE H IS RESET ACCORDING TO RH AND THE YH ARRAY IS RESCALED.
C THEN EXIT FROM 690 IF THE STEP WAS OK, OR REDO THE STEP OTHERWISE.
C-----------------------------------------------------------------------
      IF (NEWQ .EQ. NQ) GO TO 170
 630  NQ = NEWQ
      L = NQ + 1
      IRET = 2
      GO TO 150
C-----------------------------------------------------------------------
C CONTROL REACHES THIS SECTION IF 3 OR MORE FAILURES HAVE OCCURRED.
C IF 10 FAILURES HAVE OCCURRED, EXIT WITH KFLAG = -1.
C IT IS ASSUMED THAT THE DERIVATIVES THAT HAVE ACCUMULATED IN THE
C YH ARRAY HAVE ERRORS OF THE WRONG ORDER.  HENCE THE FIRST
C DERIVATIVE IS RECOMPUTED, AND THE ORDER IS SET TO 1.  THEN
C H IS REDUCED BY A FACTOR OF 10, AND THE STEP IS RETRIED,
C UNTIL IT SUCCEEDS OR H REACHES HMIN.
C-----------------------------------------------------------------------
 640  IF (KFLAG .EQ. -10) GO TO 660
      RH = 0.1D0
      RH = DMAX1(HMIN/DABS(H),RH)
      H = H*RH
      DO 645 I = 1,N
 645    Y(I) = YH(I,1)
      IERR = 0
      CALL F (NEQ, TN, Y, SAVF, IERR)
      IF (IERR .LT. 0) RETURN
      NFE = NFE + 1
      DO 650 I = 1,N
 650    YH(I,2) = H*SAVF(I)
      IPUP = MITER
      IALTH = 5
      IF (NQ .EQ. 1) GO TO 200
      NQ = 1
      L = 2
      IRET = 3
      GO TO 150
C-----------------------------------------------------------------------
C ALL RETURNS ARE MADE THROUGH THIS SECTION.  H IS SAVED IN HOLD
C TO ALLOW THE CALLER TO CHANGE H ON THE NEXT STEP.
C-----------------------------------------------------------------------
 660  KFLAG = -1
      GO TO 720
 670  KFLAG = -2
      GO TO 720
 680  KFLAG = -3
      GO TO 720
 690  RMAX = 10.0D0
 700  R = 1.0D0/TESCO(2,NQU)
      DO 710 I = 1,N
 710    ACOR(I) = ACOR(I)*R
 720  HOLD = H
      JSTART = 1
      RETURN
C----------------------- END OF SUBROUTINE STODE -----------------------
      END
