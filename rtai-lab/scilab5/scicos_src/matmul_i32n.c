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
#include <math.h>
#include "scicos_block4.h"
#include "dynlib_scicos_blocks.h"
/*--------------------------------------------------------------------------*/ 
SCICOS_BLOCKS_IMPEXP void matmul_i32n(scicos_block *block,int flag)
{
	if ((flag==1)|(flag==6)) 
	{
		int mu1 = GetInPortRows(block,1);
		int nu1 = GetInPortCols(block,1);
		int nu2 = GetInPortCols(block,2);
		long *u1 = Getint32InPortPtrs(block,1);
		long *u2 = Getint32InPortPtrs(block,2);
		long *y = Getint32OutPortPtrs(block,1);

		double k = pow(2,32);
		int l = 0;
        for (l=0;l<nu2;l++)
	    {
			int j = 0;
			for(j=0;j<mu1;j++)
	        {
				double D = 0.;
				double t = 0.;
				int jl = j + l*mu1;
				int i = 0;
				for(i=0;i<nu1;i++)
				{
					int ji = j + i*mu1;
		    	    int il = i + l*nu1;
					double C = (double)(u1[ji])*(double)(u2[il]);
					D = D + C;
				}
				t = D - (double)((int)(D/(k)))*((k));
				if ((t>=k/2)|(-(t)>=k/2))
				{
					if (t>=0) (t)=(-((k/2))+fabs(t-(double)((int)((t)/((k/2))))*((k/2))));
					else (t)=-(-((k/2))+fabs(t-(double)((int)((t)/((k/2))))*((k/2))));
				}
				y[jl]=(long)t;
			}
		}
	}
}
/*--------------------------------------------------------------------------*/ 
