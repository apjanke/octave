/* c-proto.h: macros to include or discard prototypes.

Copyright (C) 1992, 93, 95, 96 Free Software Foundation, Inc.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#ifndef KPATHSEA_C_PROTO_H
#define KPATHSEA_C_PROTO_H

#if !defined(WIN32) || (defined(_DLL) && !defined(_IMPORT))
#define DllImport
#else
#define DllImport __declspec(dllimport)
#endif

/* These macros munge function declarations to make them work in both
   cases.  The P?H macros are used for declarations, the P?C for
   definitions.  Cf. <ansidecl.h> from the GNU C library.  P1H(void)
   also works for definitions of routines which take no args.  */

#if __STDC__

#define AA(args) args /* For an arbitrary number; ARGS must be in parens.  */

#define P1H(p1) (p1)
#define P2H(p1,p2) (p1, p2)
#define P3H(p1,p2,p3) (p1, p2, p3)
#define P4H(p1,p2,p3,p4) (p1, p2, p3, p4)
#define P5H(p1,p2,p3,p4,p5) (p1, p2, p3, p4, p5)
#define P6H(p1,p2,p3,p4,p5,p6) (p1, p2, p3, p4, p5, p6)

#define P1C(t1,n1)(t1 n1)
#define P2C(t1,n1, t2,n2)(t1 n1, t2 n2)
#define P3C(t1,n1, t2,n2, t3,n3)(t1 n1, t2 n2, t3 n3)
#define P4C(t1,n1, t2,n2, t3,n3, t4,n4)(t1 n1, t2 n2, t3 n3, t4 n4)
#define P5C(t1,n1, t2,n2, t3,n3, t4,n4, t5,n5) \
  (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5)
#define P6C(t1,n1, t2,n2, t3,n3, t4,n4, t5,n5, t6,n6) \
  (t1 n1, t2 n2, t3 n3, t4 n4, t5 n5, t6 n6)

#else /* not __STDC__ */

#define AA(args) ()

#define P1H(p1) ()
#define P2H(p1, p2) ()
#define P3H(p1, p2, p3) ()
#define P4H(p1, p2, p3, p4) ()
#define P5H(p1, p2, p3, p4, p5) ()
#define P6H(p1, p2, p3, p4, p5, p6) ()

#define P1C(t1,n1) (n1) t1 n1;
#define P2C(t1,n1, t2,n2) (n1,n2) t1 n1; t2 n2;
#define P3C(t1,n1, t2,n2, t3,n3) (n1,n2,n3) t1 n1; t2 n2; t3 n3;
#define P4C(t1,n1, t2,n2, t3,n3, t4,n4) (n1,n2,n3,n4) \
  t1 n1; t2 n2; t3 n3; t4 n4;
#define P5C(t1,n1, t2,n2, t3,n3, t4,n4, t5,n5) (n1,n2,n3,n4,n5) \
  t1 n1; t2 n2; t3 n3; t4 n4; t5 n5;
#define P6C(t1,n1, t2,n2, t3,n3, t4,n4, t5,n5, t6,n6) (n1,n2,n3,n4,n5,n6) \
  t1 n1; t2 n2; t3 n3; t4 n4; t5 n5; t6 n6;

#endif /* not __STDC__ */

#endif /* not KPATHSEA_C_PROTO_H */
