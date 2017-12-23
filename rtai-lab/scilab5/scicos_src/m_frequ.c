/*  Scicos
*
*  Copyright (C) 2015 INRIA - METALAU Project <scicos@inria.fr>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/>.
*
* See the file ./license.txt
*/
/*--------------------------------------------------------------------------*/ 
#include "MALLOC.h"
#include "scicos.h"
#include "scicos_block4.h"
#include "scicos_malloc.h"
#include "scicos_free.h"
#include "dynlib_scicos_blocks.h"
/*--------------------------------------------------------------------------*/ 
SCICOS_BLOCKS_IMPEXP void m_frequ(scicos_block *block,int flag)
{
  double *mat = NULL;
  double *Dt = NULL;
  double *off = NULL;
  long *icount = NULL;
  double t = 0.0;
  long long *counter = NULL;
  int m = 0;
  mat=GetRealOparPtrs(block,1);
  Dt=GetRealOparPtrs(block,2);
  off=GetRealOparPtrs(block,3);
  icount=Getint32OparPtrs(block,4);
  m=GetOparSize(block,1,1);
  
  switch(flag)
  {

   case 4 :  {/* the workspace is used to store discrete counter value */
              if ((*block->work=scicos_malloc(sizeof(long long int)*2))==NULL) {
                set_block_error(-16);
                return;
              }
			  counter=*block->work;
			  *counter=*icount;
			  (*(counter+1))=0;
              break;
             }

   /* event date computation */
   case 3  : {
              counter=*block->work;
              t=get_scicos_time(); 
              *counter+=(int)mat[*(counter+1)]; /*increase counter*/
              block->evout[(int)mat[*(counter+1)+m]-1]=*off+((double)*counter/(*Dt))-t;
              (*(counter+1))++;
	      *(counter+1)=*(counter+1)%m;
              break;
             }

   /* finish */
   case 5  : {
              scicos_free(*block->work); /*free the workspace*/
              break;
             }

   default : break;
  }
}
/*--------------------------------------------------------------------------*/ 
