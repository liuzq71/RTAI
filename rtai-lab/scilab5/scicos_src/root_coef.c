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
extern int C2F(dprxc)();
/*--------------------------------------------------------------------------*/ 
SCICOS_BLOCKS_IMPEXP void root_coef(scicos_block *block,int flag)
{
	int mu = GetInPortRows(block,1);
	double *u = GetRealInPortPtrs(block,1);
	double *y = GetRealOutPortPtrs(block,1);

	if (flag==1||flag==6) C2F(dprxc)(&mu,u,y);
}
/*--------------------------------------------------------------------------*/ 
