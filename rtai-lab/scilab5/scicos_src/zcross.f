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

      subroutine zcross(flag,nevprt,t,xd,x,nx,z,nz,tvec,ntvec,
     &     rpar,nrpar,ipar,nipar,u,nu,g,ng)
c     Copyright INRIA

c     Scicos block simulator
c     zero crossing block
c
      double precision t,xd(*),x(*),z(*),tvec(*),rpar(*),u(*),g(*)
      integer flag,nevprt,nx,nz,ntvec,nrpar,ipar(*)
      integer nipar,nu,ng
c   

      if ((flag.eq.3).and.(nevprt.lt.0)) then
         kev=0
         do 44 j = 1,ng
            kev=2*kev+abs(g(ng+1-j))
 44      continue
         do 45 j = 1,ng 
            kev=2*kev
            if (g(ng+1-j).eq.-1) kev=kev+1
 45      continue

         l=kev*ntvec
         do 10 i=1,ntvec
            tvec(i)=rpar(l+i)+t  
 10      continue
      elseif(flag.eq.9) then
         do 20 i=1,ng
            g(i)=u(i)  
 20      continue
      endif
      end
