c Copyright (C) 2008  VZLU Prague, a.s., Czech Republic
c 
c Author: Jaroslav Hajek <highegg@gmail.com>
c 
c This source is free software; you can redistribute it and/or modify
c it under the terms of the GNU General Public License as published by
c the Free Software Foundation; either version 2 of the License, or
c (at your option) any later version.
c 
c This program is distributed in the hope that it will be useful,
c but WITHOUT ANY WARRANTY; without even the implied warranty of
c MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
c GNU General Public License for more details.
c 
c You should have received a copy of the GNU General Public License
c along with this software; see the file COPYING.  If not, see
c <http://www.gnu.org/licenses/>.
c 
      subroutine cch1dn(n,R,u,w,info)
c purpose:      given an upper triangular matrix R that is a Cholesky
c               factor of a hermitian positive definite matrix A, i.e.
c               A = R'*R, this subroutine downdates R -> R1 so that
c               R1'*R1 = A - u*u' 
c               (complex version)
c arguments:
c n (in)        the order of matrix R
c R (io)        on entry, the upper triangular matrix R
c               on exit, the updated matrix R1
c u (io)        the vector determining the rank-1 update
c               on exit, u is destroyed.
c w (w)         a workspace vector of size n
c 
c NOTE: the workspace vector is used to store the rotations
c       so that R does not need to be traversed by rows.
c
      integer n,info
      complex R(n,n),u(n)
      real w(n)
      external ctrsv,clartg,scnrm2
      real rho,scnrm2
      complex crho,rr,ui,t
      integer i,j

c quick return if possible
      if (n <= 0) return
c check for singularity of R
      do i = 1,n
        if (R(i,i) == 0e0) then
          info = 2
          return
        end if
      end do
c form R' \ u
      call ctrsv('U','C','N',n,R,n,u,1)
      rho = scnrm2(n,u,1)
c check positive definiteness      
      rho = 1 - rho**2
      if (rho <= 0e0) then
        info = 1
        return
      end if
      crho = sqrt(rho)
c eliminate R' \ u
      do i = n,1,-1
        ui = u(i)
c generate next rotation        
        call clartg(crho,ui,w(i),u(i),rr)
        crho = rr
      end do
c apply rotations
      do i = n,1,-1
        ui = 0e0
        do j = i,1,-1
          t = w(j)*ui + u(j)*R(j,i)
          R(j,i) = w(j)*R(j,i) - conjg(u(j))*ui
          ui = t
        end do
      end do

      info = 0
      end
