//
// Matrix.h
//
// $Id: Matrix.h,v 1.2 2000/02/18 01:31:49 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1997-1999, Old Dominion University.  All rights reserved.
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

#ifndef SPINDLE_MATRIX_H_
#define SPINDLE_MATRIX_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifndef SPINDLE_MATRIX_BASE_H_
#include "spindle/MatrixBase.h"
#endif

#ifndef SPINDLE_PERSISTANT_H_
#include "spindle/SpindlePersistant.h"
#endif


#ifdef MATLAB_MEX_FILE
extern "C" { 
#include "mex.h"
} // end extern "C"
#endif

SPINDLE_BEGIN_NAMESPACE

class GraphBase;

class Matrix: public MatrixBase, public SpindlePersistant{ 

  SPINDLE_DECLARE_PERSISTANT( Matrix )

public:
  Matrix();
  Matrix( GraphBase* inputGraph );
  Matrix( const GraphBase* inputGraph );
  Matrix( MatrixBase* inputMatrix );
  Matrix( const MatrixBase* inputMatrix );

#ifdef MATLAB_MEX_FILE
  Matrix( const mxArray* mat ); // constructor on matlab's Matrix
#endif 
  virtual ~Matrix();
 // required for setting variables
  virtual void validate();
  virtual void dump( FILE * fp ) const ;
  virtual bool reset();

  const SharedArray<int>& getColPtr() const { return colPtr;  }
  const SharedArray<int>& getRowIdx() const { return rowIdx;  }
  const SharedArray<int>& getDiagPtr() const { return diagPtr;  }
  const SharedArray<double>& getRealValues() const { return realValues; }
  const SharedArray<double>& getImgValues() const { return imgValues;  }
  
  SharedArray<int>& getColPtr() { currentState=UNKNOWN; return colPtr; }
  SharedArray<int>& getRowIdx() { currentState=UNKNOWN; return rowIdx; }
  SharedArray<int>& getDiagPtr() { currentState=UNKNOWN; return diagPtr; }
  SharedArray<double>& getRealValues() { currentState=UNKNOWN; return realValues; }
  SharedArray<double>& getImgValues() { currentState=UNKNOWN; return imgValues; }
};

SPINDLE_END_NAMESPACE

#endif
