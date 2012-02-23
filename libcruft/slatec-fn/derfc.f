*DECK DERFC
      DOUBLE PRECISION FUNCTION DERFC (X)
C***BEGIN PROLOGUE  DERFC
C***PURPOSE  Compute the complementary error function.
C***LIBRARY   SLATEC (FNLIB)
C***CATEGORY  C8A, L5A1E
C***TYPE      DOUBLE PRECISION (ERFC-S, DERFC-D)
C***KEYWORDS  COMPLEMENTARY ERROR FUNCTION, ERFC, FNLIB,
C             SPECIAL FUNCTIONS
C***AUTHOR  Fullerton, W., (LANL)
C***DESCRIPTION
C
C DERFC(X) calculates the double precision complementary error function
C for double precision argument X.
C
C Series for ERF        on the interval  0.          to  1.00000E+00
C                                        with weighted Error   1.28E-32
C                                         log weighted Error  31.89
C                               significant figures required  31.05
C                                    decimal places required  32.55
C
C Series for ERC2       on the interval  2.50000E-01 to  1.00000E+00
C                                        with weighted Error   2.67E-32
C                                         log weighted Error  31.57
C                               significant figures required  30.31
C                                    decimal places required  32.42
C
C Series for ERFC       on the interval  0.          to  2.50000E-01
C                                        with weighted error   1.53E-31
C                                         log weighted error  30.82
C                               significant figures required  29.47
C                                    decimal places required  31.70
C
C***REFERENCES  (NONE)
C***ROUTINES CALLED  D1MACH, DCSEVL, INITDS, XERMSG
C***REVISION HISTORY  (YYMMDD)
C   770701  DATE WRITTEN
C   890531  Changed all specific intrinsics to generic.  (WRB)
C   890531  REVISION DATE from Version 3.2
C   891214  Prologue converted to Version 4.0 format.  (BAB)
C   900315  CALLs to XERROR changed to CALLs to XERMSG.  (THJ)
C   920618  Removed space from variable names.  (RWC, WRB)
C***END PROLOGUE  DERFC
      DOUBLE PRECISION X, ERFCS(21), ERFCCS(59), ERC2CS(49), SQEPS,
     1  SQRTPI, XMAX, TXMAX, XSML, Y, D1MACH, DCSEVL
      LOGICAL FIRST
      SAVE ERFCS, ERC2CS, ERFCCS, SQRTPI, NTERF,
     1 NTERFC, NTERC2, XSML, XMAX, SQEPS, FIRST
      DATA ERFCS(  1) / -.4904612123 4691808039 9845440333 76 D-1     /
      DATA ERFCS(  2) / -.1422612051 0371364237 8247418996 31 D+0     /
      DATA ERFCS(  3) / +.1003558218 7599795575 7546767129 33 D-1     /
      DATA ERFCS(  4) / -.5768764699 7674847650 8270255091 67 D-3     /
      DATA ERFCS(  5) / +.2741993125 2196061034 4221607914 71 D-4     /
      DATA ERFCS(  6) / -.1104317550 7344507604 1353812959 05 D-5     /
      DATA ERFCS(  7) / +.3848875542 0345036949 9613114981 74 D-7     /
      DATA ERFCS(  8) / -.1180858253 3875466969 6317518015 81 D-8     /
      DATA ERFCS(  9) / +.3233421582 6050909646 4029309533 54 D-10    /
      DATA ERFCS( 10) / -.7991015947 0045487581 6073747085 95 D-12    /
      DATA ERFCS( 11) / +.1799072511 3961455611 9672454866 34 D-13    /
      DATA ERFCS( 12) / -.3718635487 8186926382 3168282094 93 D-15    /
      DATA ERFCS( 13) / +.7103599003 7142529711 6899083946 66 D-17    /
      DATA ERFCS( 14) / -.1261245511 9155225832 4954248533 33 D-18    /
      DATA ERFCS( 15) / +.2091640694 1769294369 1705002666 66 D-20    /
      DATA ERFCS( 16) / -.3253973102 9314072982 3641600000 00 D-22    /
      DATA ERFCS( 17) / +.4766867209 7976748332 3733333333 33 D-24    /
      DATA ERFCS( 18) / -.6598012078 2851343155 1999999999 99 D-26    /
      DATA ERFCS( 19) / +.8655011469 9637626197 3333333333 33 D-28    /
      DATA ERFCS( 20) / -.1078892517 7498064213 3333333333 33 D-29    /
      DATA ERFCS( 21) / +.1281188399 3017002666 6666666666 66 D-31    /
      DATA ERC2CS(  1) / -.6960134660 2309501127 3915082619 7 D-1      /
      DATA ERC2CS(  2) / -.4110133936 2620893489 8221208466 6 D-1      /
      DATA ERC2CS(  3) / +.3914495866 6896268815 6114370524 4 D-2      /
      DATA ERC2CS(  4) / -.4906395650 5489791612 8093545077 4 D-3      /
      DATA ERC2CS(  5) / +.7157479001 3770363807 6089414182 5 D-4      /
      DATA ERC2CS(  6) / -.1153071634 1312328338 0823284791 2 D-4      /
      DATA ERC2CS(  7) / +.1994670590 2019976350 5231486770 9 D-5      /
      DATA ERC2CS(  8) / -.3642666471 5992228739 3611843071 1 D-6      /
      DATA ERC2CS(  9) / +.6944372610 0050125899 3127721463 3 D-7      /
      DATA ERC2CS( 10) / -.1371220902 1043660195 3460514121 0 D-7      /
      DATA ERC2CS( 11) / +.2788389661 0071371319 6386034808 7 D-8      /
      DATA ERC2CS( 12) / -.5814164724 3311615518 6479105031 6 D-9      /
      DATA ERC2CS( 13) / +.1238920491 7527531811 8016881795 0 D-9      /
      DATA ERC2CS( 14) / -.2690639145 3067434323 9042493788 9 D-10     /
      DATA ERC2CS( 15) / +.5942614350 8479109824 4470968384 0 D-11     /
      DATA ERC2CS( 16) / -.1332386735 7581195792 8775442057 0 D-11     /
      DATA ERC2CS( 17) / +.3028046806 1771320171 7369724330 4 D-12     /
      DATA ERC2CS( 18) / -.6966648814 9410325887 9586758895 4 D-13     /
      DATA ERC2CS( 19) / +.1620854541 0539229698 1289322762 8 D-13     /
      DATA ERC2CS( 20) / -.3809934465 2504919998 7691305772 9 D-14     /
      DATA ERC2CS( 21) / +.9040487815 9788311493 6897101297 5 D-15     /
      DATA ERC2CS( 22) / -.2164006195 0896073478 0981204700 3 D-15     /
      DATA ERC2CS( 23) / +.5222102233 9958549846 0798024417 2 D-16     /
      DATA ERC2CS( 24) / -.1269729602 3645553363 7241552778 0 D-16     /
      DATA ERC2CS( 25) / +.3109145504 2761975838 3622741295 1 D-17     /
      DATA ERC2CS( 26) / -.7663762920 3203855240 0956671481 1 D-18     /
      DATA ERC2CS( 27) / +.1900819251 3627452025 3692973329 0 D-18     /
      DATA ERC2CS( 28) / -.4742207279 0690395452 2565599996 5 D-19     /
      DATA ERC2CS( 29) / +.1189649200 0765283828 8068307845 1 D-19     /
      DATA ERC2CS( 30) / -.3000035590 3257802568 4527131306 6 D-20     /
      DATA ERC2CS( 31) / +.7602993453 0432461730 1938527709 8 D-21     /
      DATA ERC2CS( 32) / -.1935909447 6068728815 6981104913 0 D-21     /
      DATA ERC2CS( 33) / +.4951399124 7733378810 0004238677 3 D-22     /
      DATA ERC2CS( 34) / -.1271807481 3363718796 0862198988 8 D-22     /
      DATA ERC2CS( 35) / +.3280049600 4695130433 1584165205 3 D-23     /
      DATA ERC2CS( 36) / -.8492320176 8228965689 2479242239 9 D-24     /
      DATA ERC2CS( 37) / +.2206917892 8075602235 1987998719 9 D-24     /
      DATA ERC2CS( 38) / -.5755617245 6965284983 1281950719 9 D-25     /
      DATA ERC2CS( 39) / +.1506191533 6392342503 5414405119 9 D-25     /
      DATA ERC2CS( 40) / -.3954502959 0187969531 0428569599 9 D-26     /
      DATA ERC2CS( 41) / +.1041529704 1515009799 8464505173 3 D-26     /
      DATA ERC2CS( 42) / -.2751487795 2787650794 5017890133 3 D-27     /
      DATA ERC2CS( 43) / +.7290058205 4975574089 9770368000 0 D-28     /
      DATA ERC2CS( 44) / -.1936939645 9159478040 7750109866 6 D-28     /
      DATA ERC2CS( 45) / +.5160357112 0514872983 7005482666 6 D-29     /
      DATA ERC2CS( 46) / -.1378419322 1930940993 8964480000 0 D-29     /
      DATA ERC2CS( 47) / +.3691326793 1070690422 5109333333 3 D-30     /
      DATA ERC2CS( 48) / -.9909389590 6243654206 5322666666 6 D-31     /
      DATA ERC2CS( 49) / +.2666491705 1953884133 2394666666 6 D-31     /
      DATA ERFCCS(  1) / +.7151793102 0292477450 3697709496 D-1        /
      DATA ERFCCS(  2) / -.2653243433 7606715755 8893386681 D-1        /
      DATA ERFCCS(  3) / +.1711153977 9208558833 2699194606 D-2        /
      DATA ERFCCS(  4) / -.1637516634 5851788416 3746404749 D-3        /
      DATA ERFCCS(  5) / +.1987129350 0552036499 5974806758 D-4        /
      DATA ERFCCS(  6) / -.2843712412 7665550875 0175183152 D-5        /
      DATA ERFCCS(  7) / +.4606161308 9631303696 9379968464 D-6        /
      DATA ERFCCS(  8) / -.8227753025 8792084205 7766536366 D-7        /
      DATA ERFCCS(  9) / +.1592141872 7709011298 9358340826 D-7        /
      DATA ERFCCS( 10) / -.3295071362 2528432148 6631665072 D-8        /
      DATA ERFCCS( 11) / +.7223439760 4005554658 1261153890 D-9        /
      DATA ERFCCS( 12) / -.1664855813 3987295934 4695966886 D-9        /
      DATA ERFCCS( 13) / +.4010392588 2376648207 7671768814 D-10       /
      DATA ERFCCS( 14) / -.1004816214 4257311327 2170176283 D-10       /
      DATA ERFCCS( 15) / +.2608275913 3003338085 9341009439 D-11       /
      DATA ERFCCS( 16) / -.6991110560 4040248655 7697812476 D-12       /
      DATA ERFCCS( 17) / +.1929492333 2617070862 4205749803 D-12       /
      DATA ERFCCS( 18) / -.5470131188 7543310649 0125085271 D-13       /
      DATA ERFCCS( 19) / +.1589663309 7626974483 9084032762 D-13       /
      DATA ERFCCS( 20) / -.4726893980 1975548392 0369584290 D-14       /
      DATA ERFCCS( 21) / +.1435873376 7849847867 2873997840 D-14       /
      DATA ERFCCS( 22) / -.4449510561 8173583941 7250062829 D-15       /
      DATA ERFCCS( 23) / +.1404810884 7682334373 7305537466 D-15       /
      DATA ERFCCS( 24) / -.4513818387 7642108962 5963281623 D-16       /
      DATA ERFCCS( 25) / +.1474521541 0451330778 7018713262 D-16       /
      DATA ERFCCS( 26) / -.4892621406 9457761543 6841552532 D-17       /
      DATA ERFCCS( 27) / +.1647612141 4106467389 5301522827 D-17       /
      DATA ERFCCS( 28) / -.5626817176 3294080929 9928521323 D-18       /
      DATA ERFCCS( 29) / +.1947443382 2320785142 9197867821 D-18       /
      DATA ERFCCS( 30) / -.6826305642 9484207295 6664144723 D-19       /
      DATA ERFCCS( 31) / +.2421988887 2986492401 8301125438 D-19       /
      DATA ERFCCS( 32) / -.8693414133 5030704256 3800861857 D-20       /
      DATA ERFCCS( 33) / +.3155180346 2280855712 2363401262 D-20       /
      DATA ERFCCS( 34) / -.1157372324 0496087426 1239486742 D-20       /
      DATA ERFCCS( 35) / +.4288947161 6056539462 3737097442 D-21       /
      DATA ERFCCS( 36) / -.1605030742 0576168500 5737770964 D-21       /
      DATA ERFCCS( 37) / +.6063298757 4538026449 5069923027 D-22       /
      DATA ERFCCS( 38) / -.2311404251 6979584909 8840801367 D-22       /
      DATA ERFCCS( 39) / +.8888778540 6618855255 4702955697 D-23       /
      DATA ERFCCS( 40) / -.3447260576 6513765223 0718495566 D-23       /
      DATA ERFCCS( 41) / +.1347865460 2069650682 7582774181 D-23       /
      DATA ERFCCS( 42) / -.5311794071 1250217364 5873201807 D-24       /
      DATA ERFCCS( 43) / +.2109341058 6197831682 8954734537 D-24       /
      DATA ERFCCS( 44) / -.8438365587 9237891159 8133256738 D-25       /
      DATA ERFCCS( 45) / +.3399982524 9452089062 7359576337 D-25       /
      DATA ERFCCS( 46) / -.1379452388 0732420900 2238377110 D-25       /
      DATA ERFCCS( 47) / +.5634490311 8332526151 3392634811 D-26       /
      DATA ERFCCS( 48) / -.2316490434 4770654482 3427752700 D-26       /
      DATA ERFCCS( 49) / +.9584462844 6018101526 3158381226 D-27       /
      DATA ERFCCS( 50) / -.3990722880 3301097262 4224850193 D-27       /
      DATA ERFCCS( 51) / +.1672129225 9444773601 7228709669 D-27       /
      DATA ERFCCS( 52) / -.7045991522 7660138563 8803782587 D-28       /
      DATA ERFCCS( 53) / +.2979768402 8642063541 2357989444 D-28       /
      DATA ERFCCS( 54) / -.1262522466 4606192972 2422632994 D-28       /
      DATA ERFCCS( 55) / +.5395438704 5424879398 5299653154 D-29       /
      DATA ERFCCS( 56) / -.2380992882 5314591867 5346190062 D-29       /
      DATA ERFCCS( 57) / +.1099052830 1027615735 9726683750 D-29       /
      DATA ERFCCS( 58) / -.4867713741 6449657273 2518677435 D-30       /
      DATA ERFCCS( 59) / +.1525877264 1103575676 3200828211 D-30       /
      DATA SQRTPI / 1.772453850 9055160272 9816748334 115D0 /
      DATA FIRST /.TRUE./
C***FIRST EXECUTABLE STATEMENT  DERFC
      IF (FIRST) THEN
         ETA = 0.1*REAL(D1MACH(3))
         NTERF = INITDS (ERFCS, 21, ETA)
         NTERFC = INITDS (ERFCCS, 59, ETA)
         NTERC2 = INITDS (ERC2CS, 49, ETA)
C
         XSML = -SQRT(-LOG(SQRTPI*D1MACH(3)))
         TXMAX = SQRT(-LOG(SQRTPI*D1MACH(1)))
         XMAX = TXMAX - 0.5D0*LOG(TXMAX)/TXMAX - 0.01D0
         SQEPS = SQRT(2.0D0*D1MACH(3))
      ENDIF
      FIRST = .FALSE.
C
      IF (ISNAN(X)) THEN
         DERFC = X
         RETURN
      ENDIF
C
      IF (X.GT.XSML) GO TO 20
C
C ERFC(X) = 1.0 - ERF(X)  FOR  X .LT. XSML
C
      DERFC = 2.0D0
      RETURN
C
 20   IF (X.GT.XMAX) GO TO 40
      Y = ABS(X)
      IF (Y.GT.1.0D0) GO TO 30
C
C ERFC(X) = 1.0 - ERF(X)  FOR ABS(X) .LE. 1.0
C
      IF (Y.LT.SQEPS) DERFC = 1.0D0 - 2.0D0*X/SQRTPI
      IF (Y.GE.SQEPS) DERFC = 1.0D0 - X*(1.0D0 + DCSEVL (2.D0*X*X-1.D0,
     1  ERFCS, NTERF))
      RETURN
C
C ERFC(X) = 1.0 - ERF(X)  FOR  1.0 .LT. ABS(X) .LE. XMAX
C
 30   Y = Y*Y
      IF (Y.LE.4.D0) DERFC = EXP(-Y)/ABS(X) * (0.5D0 + DCSEVL (
     1  (8.D0/Y-5.D0)/3.D0, ERC2CS, NTERC2) )
      IF (Y.GT.4.D0) DERFC = EXP(-Y)/ABS(X) * (0.5D0 + DCSEVL (
     1  8.D0/Y-1.D0, ERFCCS, NTERFC) )
      IF (X.LT.0.D0) DERFC = 2.0D0 - DERFC
      RETURN
C
 40   DERFC = 0.D0
      RETURN
C
      END
