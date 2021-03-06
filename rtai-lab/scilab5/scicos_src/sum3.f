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

      subroutine sum3(flag,nevprt,t,xd,x,nx,z,nz,tvec,ntvec,
     &     rpar,nrpar,ipar,nipar,u1,nu1,u2,nu2,u3,nu3,y,ny)
c     Copyright INRIA

c     Scicos block simulator
c     adds the inputs weighed by rpar
c
      double precision t,xd(*),x(*),z(*),tvec(*),rpar(*)
      double precision u1(*),u2(*),u3(*),y(*)
      integer flag,nevprt,nx,nz,ntvec,nrpar,ipar(*)
      integer nipar,nu1,ny

c
      do 1 i=1,nu1
         y(i)=u1(i)*rpar(1)+u2(i)*rpar(2)+u3(i)*rpar(3)
 1    continue
      return
      end
