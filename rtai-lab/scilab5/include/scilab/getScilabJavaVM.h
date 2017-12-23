
/*
 * Scilab ( http://www.scilab.org/ ) - This file is part of Scilab
 * Copyright (C) 2015 INRIA
 * Copyright (C) 2015 Allan CORNET
 *
 * This file must be used under the terms of the CeCILL.
 * This source file is licensed as described in the file COPYING, which
 * you should have received as part of this distribution.  The terms
 * are also available at
 * http://www.cecill.info/licences/Licence_CeCILL_V2.1-en.txt
 *
 */

/*--------------------------------------------------------------------------*/
#ifndef __GETSCILABJAVAVM_H__
#define __GETSCILABJAVAVM_H__

#include <jni.h> /* JavaVM */

/**
* returns Scilab JavaVM
* @return JavaVM
*/
JavaVM *getScilabJavaVM(void);

#endif /* __GETSCILABJAVAVM_H__ */
/*--------------------------------------------------------------------------*/
