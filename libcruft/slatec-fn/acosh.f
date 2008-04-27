*DECK ACOSH
      FUNCTION ACOSH (X)
C***BEGIN PROLOGUE  ACOSH
C***PURPOSE  Compute the arc hyperbolic cosine.
C***LIBRARY   SLATEC (FNLIB)
C***CATEGORY  C4C
C***TYPE      SINGLE PRECISION (ACOSH-S, DACOSH-D, CACOSH-C)
C***KEYWORDS  ACOSH, ARC HYPERBOLIC COSINE, ELEMENTARY FUNCTIONS, FNLIB,
C             INVERSE HYPERBOLIC COSINE
C***AUTHOR  Fullerton, W., (LANL)
C***DESCRIPTION
C
C ACOSH(X) computes the arc hyperbolic cosine of X.
C
C***REFERENCES  (NONE)
C***ROUTINES CALLED  R1MACH, XERMSG
C***REVISION HISTORY  (YYMMDD)
C   770401  DATE WRITTEN
C   890531  Changed all specific intrinsics to generic.  (WRB)
C   890531  REVISION DATE from Version 3.2
C   891214  Prologue converted to Version 4.0 format.  (BAB)
C   900315  CALLs to XERROR changed to CALLs to XERMSG.  (THJ)
C   900326  Removed duplicate information from DESCRIPTION section.
C           (WRB)
C***END PROLOGUE  ACOSH
      SAVE ALN2,XMAX
      DATA ALN2 / 0.6931471805 5994530942E0/
      DATA XMAX /0./
C***FIRST EXECUTABLE STATEMENT  ACOSH
      IF (XMAX.EQ.0.) XMAX = 1.0/SQRT(R1MACH(3))
C
      IF (X .LT. 1.0) CALL XERMSG ('SLATEC', 'ACOSH', 'X LESS THAN 1',
     +   1, 2)
C
      IF (X.LT.XMAX) ACOSH = LOG (X + SQRT(X*X-1.0))
      IF (X.GE.XMAX) ACOSH = ALN2 + LOG(X)
C
      RETURN
      END
