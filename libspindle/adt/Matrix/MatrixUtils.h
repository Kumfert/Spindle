//
// MatrixUtils.h -- a collection of static methods for manipulating matrices
//
//  $Id: MatrixUtils.h,v 1.2 2000/02/18 01:31:50 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1997, Old Dominion University.  All rights reserved.
// 
//  Permission to use, copy, modify, distribute and sell this software and
//  its documentation for any purpose is hereby granted without fee, 
//  provided that the above copyright notice appear in all copies and
//  that both that copyright notice and this permission notice appear
//  in supporting documentation.  Old Dominion University makes no
//  representations about the suitability of this software for any 
//  purpose.  It is provided "as is" without express or implied warranty.
//
///////////////////////////////////////////////////////////////////////
//
//

#ifndef SPINDLE_MATRIX_UTILS_H_
#define SPINDLE_MATRIX_UTILS_H_

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifndef SPINDLE_AUTO_ARRAY_H_
#include "spindle/SharedArray.h"
#endif

SPINDLE_BEGIN_NAMESPACE

class MatrixBase;
class GraphBase;

class MatrixUtils { 
public:
  enum { STRICT_LOWER = -2, LOWER = -1, 
	 DIAGONAL = 0, UPPER = 1, 
	 STRICT_UPPER = 2 } triangularOption;

  static bool isSymmetric( const MatrixBase* matrix, double tol = -1.0 );
  static bool isHermitian( const MatrixBase* matrix, double tol = -1.0 )
  { return isSymmetric( matrix, tol ); }
  static bool isTriangular( const MatrixBase* matrix, int triangularOption );

  static bool makeDiagonalsExplicit( MatrixBase* matrix );
  static bool makeSymmetric( MatrixBase* matrix );
  static bool makeHermitian( MatrixBase* matrix ) 
  { return makeSymmetric( matrix ); }
  static bool makeTriangular( MatrixBase* matrix, int triangularOption );
  static bool makeTranspose( MatrixBase* matrix );
  static bool makeTranspose( MatrixBase* matrix, SharedArray<int>& Tmap ); // save transpose map
  static bool deepCopy( const MatrixBase* src, MatrixBase* dest );

  static bool metrics( MatrixBase* matrix, double* answers, int nAnswers );

};

SPINDLE_END_NAMESPACE

#endif
