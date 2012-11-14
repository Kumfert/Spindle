//
// MatrixBase.h -- a base class for all matrix/graph classes
//
//  $Id: MatrixBase.h,v 1.2 2000/02/18 01:31:50 kumfert Exp $
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

#ifndef SPINDLE_MATRIX_BASE_H_
#define SPINDLE_MATRIX_BASE_H_

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifndef SPINDLE_AUTO_ARRAY_H_
#include "spindle/SharedArray.h"
#endif

SPINDLE_BEGIN_NAMESPACE 

class MatrixUtils;

class MatrixBase { 
friend class MatrixUtils;

protected:
  int nRows;
  int nCols;
  int nNonZeros;

  SharedArray<int> colPtr;
  SharedArray<int> rowIdx;
  SharedArray<int> diagPtr;
  SharedArray<double> realValues;
  SharedArray<double> imgValues;

public:

  bool resize( const int newNCols, const int newNRows, const int newNNonZeros );
  int queryNCols() const { return nCols; }
  int queryNRows() const { return nRows; }
  int queryNNonZeros() const { return nNonZeros; }

  const SharedArray<int>& getColPtr() const { return colPtr;  }
  const SharedArray<int>& getRowIdx() const { return rowIdx;  }
  const SharedArray<int>& getDiagPtr() const { return diagPtr;  }
  const SharedArray<double>& getRealValues() const { return realValues; }
  const SharedArray<double>& getImgValues() const { return imgValues;  }
  
  SharedArray<int>& getColPtr() { return colPtr; }
  SharedArray<int>& getRowIdx() { return rowIdx; }
  SharedArray<int>& getDiagPtr() { return diagPtr; }
  SharedArray<double>& getRealValues() { return realValues; }
  SharedArray<double>& getImgValues() { return imgValues;  }

  bool setDiagPtr();
};

SPINDLE_END_NAMESPACE

#endif
