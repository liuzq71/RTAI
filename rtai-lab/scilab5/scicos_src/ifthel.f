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

      subroutine ifthel(flag,nevprt,ntvec,rpar,nrpar,ipar,nipar,u,nu)
c     Scicos block simulator
c     if-then-else block
c     if event input exits from then or else clock ouputs based
c     on the sign of the unique input (if input>0 then  else )
c
c     Copyright INRIA
      double precision rpar(*),u(*)
      integer flag,nevprt,ntvec,nrpar,ipar(*)
      integer nipar,nu

c
      common /dbcos/ idb
c
      if(idb.eq.1) then
         write(6,'(''ifthel     t='',e10.3,'' flag='',i1)') t,flag
      endif
c
      if(flag.eq.3) then
         if(u(1).le.0.d0) then
            ntvec=2
         else
            ntvec=1
         endif
      endif
      end
