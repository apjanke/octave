      subroutine xzdotu(n,zx,incx,zy,incy,retval)
      double complex zdotu, zx(*), zy(*), retval
      integer n, incx, incy
      external zdotu
      retval = zdotu (n, dx, incx, dy, incy)
      return
      end
