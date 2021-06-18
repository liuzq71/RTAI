c     Scicos 
c 
c     Copyright (C) 2015 INRIA - METALAU Project <scicos@inria.fr> 
c 
c     This program is free software; you can redistribute it and/or modify 
c     it under the terms of the GNU General Public License as published by 
c     the Free Software Foundation; either version 2 of the License, or 
c     (at your option) any later version. 
c 
c     This program is distributed in the hope that it will be useful, 
c     but WITHOUT ANY WARRANTY; without even the implied warranty of 
c     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
c     GNU General Public License for more details. 
c 
c     You should have received a copy of the GNU General Public License 
c     along with this program; if not, see <http://www.gnu.org/licenses/>. 
c 
c     See the file ./license.txt 
c 

      subroutine selblk(flag,nevprt,t,xd,x,nx,z,nz,tvec,ntvec,
     &     rpar,nrpar,ipar,nipar,u,nu,y,ny)
c     Copyright INRIA

c     Scicos block simulator
c     Selector block
c
      double precision t,xd(*),x(*),z(*),tvec(*),rpar(*),u(*),y(*)
      integer flag,nevprt,nx,nz,ntvec,nrpar,ipar(*)
      integer nipar,nu,ny
c
c
      if(flag.eq.2.and.nevprt.gt.0) then
         ic=0
         nev=nevprt
 10      continue
         if(nev.ge.1) then
            ic=ic+1
            nev=nev/2
            goto 10
         endif
         z(1)=dble(ic)
      elseif(flag.eq.1.or.flag.eq.6) then
      y(1)=u(int(z(1)))
      endif
      end

