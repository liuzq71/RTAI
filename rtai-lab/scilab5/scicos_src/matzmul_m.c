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
#include "machine.h"
#include "scicos_block4.h"
#include "dynlib_scicos_blocks.h"
/*--------------------------------------------------------------------------*/ 
extern int C2F(wmmul)();
/*--------------------------------------------------------------------------*/ 
SCICOS_BLOCKS_IMPEXP void matzmul_m(scicos_block *block,int flag)
{
	int mu1=GetInPortRows(block,1);
	int mu2=GetInPortRows(block,2);
	int nu=GetInPortCols(block,2);

	double *u1r=GetRealInPortPtrs(block,1);
	double *u1i=GetImagInPortPtrs(block,1);
	double *u2r=GetRealInPortPtrs(block,2);
	double *u2i=GetImagInPortPtrs(block,2);
	double *yr=GetRealOutPortPtrs(block,1);
	double *yi=GetImagOutPortPtrs(block,1);

	C2F(wmmul)(u1r,u1i,&mu1,u2r,u2i,&mu2,yr,yi,&mu1,&mu1,&mu2,&nu);
}
/*--------------------------------------------------------------------------*/ 
