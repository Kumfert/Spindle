//
// Matrix.cc
//
// $Id: Matrix.cc,v 1.2 2000/02/18 01:31:49 kumfert Exp $
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

#include "spindle/Matrix.h"

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include "algo.h"
#else
#include <algorithm>
using namespace std;
#endif

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifndef SPINDLE_ARCHIVE_H_
#include "spindle/SpindleArchive.h"
#endif

#ifndef SPINDLE_GRAPH_BASE_H_
#include "spindle/GraphBase.h"
#endif

#ifndef SPINDLE_MATRIX_UTILS_H_
#include "spindle/MatrixUtils.h"
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

#ifndef __FUNC__
#undef __FUNC__
#endif

#define __FUNC__ "Matrix::Matrix()"
Matrix::Matrix() : MatrixBase(), SpindlePersistant() { 
  FENTER;
  incrementInstanceCount( Matrix::MetaData ); 
  FCALL reset();
  FEXIT;
}
#undef __FUNC__


#define __FUNC__ "Matrix::Matrix( GraphBase* inputGraph )"
Matrix::Matrix( GraphBase* inputGraph ){
  incrementInstanceCount( Matrix::MetaData );
  FCALL reset();
  if ( inputGraph == 0 ) {
    ERROR(SPINDLE_ERROR_ARG_BADPTR,"Invalid constructor arguments.");
    currentState = INVALID;
    FEXIT;
  } 
  nCols = inputGraph->queryNVtxs();
  nRows = nCols;
  nNonZeros = inputGraph->queryNNonZeros();
  colPtr.import( inputGraph->getAdjHead() );
  rowIdx.import( inputGraph->getAdjList() );
  currentState = UNKNOWN;
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "Matrix::Matrix( const GraphBase* inputGraph )"
Matrix::Matrix( const GraphBase* inputGraph ) {
  incrementInstanceCount( Matrix::MetaData );
  FCALL reset();
  if ( inputGraph == 0 ) {
    ERROR(SPINDLE_ERROR_ARG_BADPTR,"Invalid constructor arguments.");
    currentState = INVALID;
    FEXIT;
  } 
  nCols = inputGraph->queryNVtxs();
  nRows = nCols;
  nNonZeros = inputGraph->queryNNonZeros();
  colPtr.import( inputGraph->getAdjHead() );
  rowIdx.import( inputGraph->getAdjList() );
  currentState = UNKNOWN;
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "Matrix::Matrix( MatrixBase* inputMatrix )"
Matrix::Matrix( MatrixBase* inputMatrix ) {
  incrementInstanceCount( Matrix::MetaData );
  FCALL reset();
  if ( inputMatrix == 0 ) { 
    ERROR(SPINDLE_ERROR_ARG_BADPTR,"Invalid constructor arguments.");
    currentState = INVALID;
    FEXIT;
  }
  nCols = inputMatrix->queryNCols();
  nRows = inputMatrix->queryNRows();
  nNonZeros = inputMatrix->queryNNonZeros();
  colPtr.import( inputMatrix->getColPtr() );
  rowIdx.import( inputMatrix->getRowIdx() );
  if ( inputMatrix->getDiagPtr().size() > 0 ) { 
    diagPtr.import( inputMatrix->getDiagPtr() );
  }
  if ( inputMatrix->getRealValues().size() > 0 ) { 
    realValues.import( inputMatrix->getRealValues() );
  }
  if ( inputMatrix->getImgValues().size() > 0 ) { 
    imgValues.import( inputMatrix->getImgValues() );
  }
  currentState = UNKNOWN;
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "Matrix::Matrix( const MatrixBase* inputMatrix )"
Matrix::Matrix( const MatrixBase* inputMatrix ) {
  incrementInstanceCount( Matrix::MetaData );
  FCALL reset();
  if ( inputMatrix == 0 ) { 
    ERROR(SPINDLE_ERROR_ARG_BADPTR,"Invalid constructor arguments.");
    currentState = INVALID;
    FEXIT;
  }
  nCols = inputMatrix->queryNCols();
  nRows = inputMatrix->queryNRows();
  nNonZeros = inputMatrix->queryNNonZeros();
  colPtr.import( inputMatrix->getColPtr() );
  rowIdx.import( inputMatrix->getRowIdx() );
  if ( inputMatrix->getDiagPtr().size() > 0 ) { 
    diagPtr.import( inputMatrix->getDiagPtr() );
  }
  if ( inputMatrix->getRealValues().size() > 0 ) { 
    realValues.import( inputMatrix->getRealValues() );
  }
  if ( inputMatrix->getImgValues().size() > 0 ) { 
    imgValues.import( inputMatrix->getImgValues() );
  }
  currentState = UNKNOWN;
  FEXIT;
}
#undef __FUNC__

#ifdef MATLAB_MEX_FILE
#define __FUNC__  "Graph( const mxArray* mat )"
Matrix::Matrix( const mxArray* mat ) { // constructor on matlab's Matrix
  FENTER;
  incrementInstanceCount( Matrix::MetaData );
  FCALL reset();
 
  if ( mat == 0 ) {
    ERROR(SPINDLE_ERROR_ARG_BADPTR,"Invalid constructor arguments.");
    currentState = INVALID;
    FEXIT;
  }
  if ( ! mxIsSparse( mat ) ) { 
    ERROR(SPINDLE_ERROR_ARG_INVALID,"Input Matlab Matrix is not sparse.");
    currentState = INVALID;
    FEXIT;
  }
  nRows = mxGetM( mat );
  nCols = mxGetN( mat );
  colPtr.import( mxGetJc( mat ), nCols + 1 );
  nNonZeros = colPtr[ nCols ];
  rowIdx.import( mxGetIr( mat ), nNonZeros );
  if ( mxGetPr( mat ) != 0 ) { 
    realValues.import( mxGetPr( mat ), nNonZeros );
  } 
  if ( mxGetPi( mat ) != 0 ) { 
    imgValues.import( mxGetPi( mat ), nNonZeros );
  }
  currentState = UNKNOWN;
  FEXIT;
}
#undef __FUNC__
#endif 
Matrix::~Matrix() { 
  decrementInstanceCount( Matrix::MetaData );
}

#define __FUNC__ "Matrix::validate()"
void 
Matrix::validate(){
  FENTER;
  if ( currentState == VALID ) { FEXIT; } // if known valid, don't bother
  if ( currentState == INVALID ) { 
    WARNING("Cannot validate a known invalid object, must \"reset()\" it first.\n");
    FEXIT;
  }

  // 1. Make sure that colPtr and rowIdx are not null.
  if ( (colPtr.size()==0) || (rowIdx.size()==0) ) { 
    ERROR(SPINDLE_ERROR_ARG_BADPTR,
	  "Cannot validate Matrix with empty \'colPtr\' or \'rowIdx\'\n");
    currentState=INVALID;
    FEXIT;
  }
  
  // 2. now deterimine nRows, nCols;
  nCols = colPtr.size() - 1;
  nRows = 0;
  {for( const int * cur = rowIdx.begin(), *stop = rowIdx.end(); cur != stop; ++cur ){ 
    nRows = ( *cur > nRows ) ? *cur : nRows; 
  }}
  ++nRows;
  nNonZeros = rowIdx.size();
  currentState = VALID;
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "void dump( FILE * fp ) const"
void 
Matrix::dump( FILE * fp ) const {
  FENTER;
  fprintf( fp,"\nMatrix::dump() {\n");
  fprintf(fp,"   className    = %s\n", MetaData->className );
  fprintf(fp,"   classSize    = %d\n", MetaData->classSize );
  fprintf(fp,"   idNumber     = %d\n", queryIDNumber() );
  switch( currentState ) {
  case INVALID:
    fprintf(fp,"   currentState = INVALID\n");
    break;
  case EMPTY:
    fprintf(fp,"   currentState = EMPTY\n");
    break;
  case UNKNOWN:
    fprintf(fp,"   currentState = UNKNOWN\n");
    break;
  case VALID:
    fprintf(fp,"   currentState = VALID\n");
    break;
  default:
    WARNING("Unrecognized state detected... seting to INVALID");
    Matrix* cast_away_const = ( Matrix* ) this;
    cast_away_const->currentState = INVALID;
    fprintf(fp,"   currentState = Unrecognized... set to INVALID\n");
    break;
  }
  if ( currentState == VALID ) {
    fprintf( fp,"   nCols        = %d\n", nCols );
    fprintf( fp,"   nRows        = %d\n", nRows );
    fprintf( fp,"   nNonZeros    = %d\n", nNonZeros );
    fprintf( fp,"   realValues?  = %s\n", (realValues.size()==0)? "No" : "Yes" );
    fprintf( fp,"   imgValues?   = %s\n", (imgValues.size()==0)? "No" : "Yes" );
  }
  fprintf(fp,"}\n");
  FEXIT;
}
#undef __FUNC__
bool
Matrix::reset() { 
  FENTER;
  currentState = EMPTY;
  nCols = 0;
  nRows = 0;
  nNonZeros = 0;

  colPtr.reset();
  rowIdx.reset();
  diagPtr.reset();
  realValues.reset();
  imgValues.reset();
  FRETURN( true );
}
#undef __FUNC__


#define __FUNC__ "void Matrix::loadObject( SpindleArchive& ar )"
void 
Matrix::loadObject( SpindleArchive& ar ) {
  FENTER;
  if ( &ar ) ; // keeps compiler from complaining about ar not being used
  ERROR( SPINDLE_ERROR_UNIMP_FUNC,"Unimplemented function" );
  FEXIT;
}
#undef __FUNC__


#define __FUNC__ "void Matrix::storeObject( SpindleArchive& ar ) const"
void 
Matrix::storeObject( SpindleArchive& ar ) const {
  FENTER;
  if ( &ar ) ; // keeps compiler from complaining about ar not being used
  ERROR( SPINDLE_ERROR_UNIMP_FUNC,"Unimplemented function" );
  FEXIT;
}
#undef __FUNC__


SPINDLE_IMPLEMENT_DYNAMIC( Matrix, SpindlePersistant )
