*** This subroutine includes all of the ZDOTU function instead of simply
*** wrapping it in a subroutine to avoid possible differences in the way
*** complex values are returned by various Fortran compilers.  For
*** example, if we simply wrap the function and compile this file with
*** gfortran and the library that provides ZDOTU is compiled with a
*** compiler that uses the g77 (f2c-compatible) calling convention for
*** complex-valued functions, all hell will break loose.

      subroutine xzdotu(n,zx,incx,zy,incy,ztemp)

***   double complex function zdotu(n,zx,incx,zy,incy)
c
c     forms the dot product of two vectors.
c     jack dongarra, 3/11/78.
c     modified 12/3/93, array(1) declarations changed to array(*)
c
      double complex zx(*),zy(*),ztemp
      integer i,incx,incy,ix,iy,n
      ztemp = (0.0d0,0.0d0)
***   zdotu = (0.0d0,0.0d0)
      if(n.le.0)return
      if(incx.eq.1.and.incy.eq.1)go to 20
c
c        code for unequal increments or equal increments
c          not equal to 1
c
      ix = 1
      iy = 1
      if(incx.lt.0)ix = (-n+1)*incx + 1
      if(incy.lt.0)iy = (-n+1)*incy + 1
      do 10 i = 1,n
        ztemp = ztemp + zx(ix)*zy(iy)
        ix = ix + incx
        iy = iy + incy
   10 continue
***   zdotu = ztemp
      return
c
c        code for both increments equal to 1
c
   20 do 30 i = 1,n
        ztemp = ztemp + zx(i)*zy(i)
   30 continue
***   zdotu = ztemp
      return
      end
