//
// MatrixUtils.cc -- a collection of static methods for manipulating matrices
//
//  $Id: MatrixUtils.cc,v 1.2 2000/02/18 01:31:50 kumfert Exp $
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

#include "spindle/MatrixUtils.h"

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include "math.h"
#else
#include <cmath>
using namespace std;
#endif

#ifndef SPINDLE_MATRIX_BASE_H_
#include "spindle/MatrixBase.h"
#endif

#ifndef SPINDLE_GRAPH_BASE_H_
#include "spindle/GraphBase.h"
#endif  

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

#ifdef __FUNC__
#undef __FUNC__
#endif

#define __FUNC__ "bool MatrixUtils::isSymmetric( const MatrixBase* matrix, double tol )"
bool 
MatrixUtils::isSymmetric( const MatrixBase* matrix, double tol ) {
  FENTER;
  const int n = matrix->queryNRows();
  const int m = matrix->queryNCols();
  if ( m != n ) { return false; }  // can't be symmetric if not square 

  // First copy values from ColPtr to Atrans
  SharedArray<int> Atrans;
  Atrans.import( matrix->getColPtr());
  
  // now get pointers out of arrays
  const int * colPtr = matrix->getColPtr().lend();
  const int * rowIdx = matrix->getRowIdx().lend();
  int * adjTrans  = Atrans.begin();  // non-const since this might change

  if ( tol < 0.0 ) { // test for structural symmetry only 
    if ( matrix->getDiagPtr().size() == n ) { // if diagPtr is set
      const int * diagPtr = matrix->getDiagPtr().lend();
      for( int i=0; i<n; ++i ) {
	for( int j=diagPtr[i]; j<colPtr[i+1]; ++j ) {
	  int temp = adjTrans[rowIdx[j]]++;
	  if ( rowIdx[temp] != i ) {
	    // not structurally symmetric
	    FRETURN( false );
	  }
	}
      }
    } else {  // if diagPtr is not set
      for( int i=0; i<n; ++i ) {
	for( int j=colPtr[i]; j<colPtr[i+1]; ++j ) {
	  int temp = adjTrans[rowIdx[j]]++;
	  if ( rowIdx[temp] != i ) {
	    // not structurally symmetric
	    FRETURN( false );
	  }
	} // end j loop
      } // end i loop
    } // end if diagPtr is not set
  } else { // else if tolerance positive, test for numerical symmetry
    if ( matrix->getDiagPtr().size() == n ) { // if diagPtr is set
      const int *diagPtr = matrix->getDiagPtr().lend();
      if ( matrix->getRealValues().size() == n ) { // check real values if they exist
	const double* val = matrix->getRealValues().lend();
	for( int i=0; i<n; ++i ) {
	  for( int j=diagPtr[i]; j<colPtr[i+1]; ++j ) {
	    int temp = adjTrans[rowIdx[j]]++;
	    if ( fabs( val[temp] - val[j] ) > tol ) {
	      // not numerically  symmetric
	      FRETURN( false );
	    }
	  }
	}
      } // end check real values
      if ( matrix->getImgValues().size() == n ) { // check imaginary values if they exist
	const double* val = matrix->getImgValues().lend();
	for( int i=0; i<n; ++i ) {
	  for( int j=diagPtr[i]; j<colPtr[i+1]; ++j ) {
	    int temp = adjTrans[rowIdx[j]]++;
	    if ( fabs( val[temp] + val[j] ) > tol ) { // imaginary checks for conjugate
	      // not numerically  hermitian
	      FRETURN( false );
	    }
	  }
	}
      } // end check imaginary values
    } else {  // if diagPtr is not set
      if ( matrix->getRealValues().size() == n ) { // check real values if they exist
	const double* val = matrix->getRealValues().lend();
	for( int i=0; i<n; ++i ) {
	  for( int j=colPtr[i]; j<colPtr[i+1]; ++j ) {
	    int temp = adjTrans[rowIdx[j]]++;
	    if ( fabs( val[temp] - val[j] ) > tol ) {
	      // not numerically  symmetric
	      FRETURN( false );
	    }
	  } // end j loop
	} // end i loop
      } // end if real values are set
      if ( matrix->getImgValues().size() == n ) { // check imaginary values
	const double* val = matrix->getImgValues().lend();
	for( int i=0; i<n; ++i ) {
	  for( int j=colPtr[i]; j<colPtr[i+1]; ++j ) {
	    int temp = adjTrans[rowIdx[j]]++;
	    if ( fabs( val[temp] + val[j] ) > tol ) { // + since imaginaries are concerned with the conjugate
	      // not numerically hermitian
	      FRETURN( false );
	    }
	  } // end j loop
	} // end i loop
      } // end if imaginary values are set
    } // end if diagPtr is not set
  } // end if check for numeric symmetry
  FRETURN( true );
}	     
#undef __FUNC__


#define __FUNC__ "bool MatrixUtils::isTriangular(const MatrixBase* matrix, int triangularOption )"
bool
MatrixUtils::isTriangular( const MatrixBase* matrix, int triangularOption ) {
  FENTER;
  const int* colPtr = matrix->getColPtr().lend();
  const int* rowIdx = matrix->getRowIdx().lend();
  const int n = matrix->queryNCols();
  
  switch ( triangularOption ) {
  case (int) STRICT_LOWER:
    {for( int j=0; j<n; ++j ) {
      for( int i=colPtr[j]; i<colPtr[j+1]; ++i ) {
	if ( j>=rowIdx[i] ) {
	  FRETURN( false );
	}
      }
    }}
    break;
  case (int) LOWER:
    {for( int j=0; j<n; ++j ) {
      for( int i=colPtr[j]; i<colPtr[j+1]; ++i ) {
	if ( j>rowIdx[i] ) {
	  FRETURN( false );
	}
      }
    }}
    break;
  case (int) DIAGONAL:
    {for( int j=0; j<n; ++j ) {
      for( int i=colPtr[j]; i<colPtr[j+1]; ++i ) {
	if ( j!=rowIdx[i] ) {
	  FRETURN( false );
	}
      }
    }}
    break;
  case (int) UPPER:
    {for( int j=0; j<n; ++j ) {
      for( int i=colPtr[j]; i<colPtr[j+1]; ++i ) {
	if ( j<rowIdx[i] ) {
	  FRETURN( false );
	}
      }
    }}
    break;  
  case (int) STRICT_UPPER:
    {for( int j=0; j<n; ++j ) {
      for( int i=colPtr[j]; i<colPtr[j+1]; ++i ) {
	if ( j<rowIdx[i] ) {
	  FRETURN( false );
	}
      }
    }}
    break;
  default:
    ERROR( SPINDLE_ERROR_UNEXP_CASE, "Unexpected case%d\n", triangularOption );
  }
  FRETURN( true );
}
#undef __FUNC__


#define __FUNC__ "bool MatrixUtils::makeTriangular( MatrixBase* matrix, int triangularOption )"
bool
MatrixUtils::makeTriangular( MatrixBase* matrix, int triangularOption ) {
  FENTER;
  const int * colPtr = matrix->getColPtr().lend();
  const int * rowIdx = matrix->getRowIdx().lend();
  const int n = matrix->queryNCols();
  const int oldNNZ = matrix->queryNNonZeros();
  SharedArray<int> newHead( n+1 );
  SharedArray<int> newList( oldNNZ );
  SharedArray<int> triMap( oldNNZ );
  triMap.init(-1);
  int curIdx = 0;
  
  switch ( triangularOption ) {
  case (int) STRICT_LOWER:
    {for( int j=0; j<n; ++j ) {
      newHead[j] = curIdx;
      for( int i=colPtr[j]; i<colPtr[j+1]; ++i ) {
	if ( rowIdx[i] > j ) {
	  newList[curIdx] = rowIdx[i];
	  triMap[i] = curIdx;
	  curIdx++;
	}
      }    
    }}
    newHead[n] = curIdx;
    break;
  case (int) LOWER:
    {for( int j=0; j<n; ++j ) {
      newHead[j] = curIdx;
      for( int i=colPtr[j]; i<colPtr[j+1]; ++i ) {
	if ( rowIdx[i] >= j ) {
	  newList[curIdx] = rowIdx[i];
	  triMap[i] = curIdx;
	  curIdx++;
	}
      }
    }}
    newHead[n] = curIdx;
    break;
  case (int) DIAGONAL:
    {for( int j=0; j<n; ++j ) {
      newHead[j] = curIdx;
      for( int i=colPtr[j]; i<colPtr[j+1]; ++i ) {
	if ( rowIdx[i] == j ) {
	  newList[curIdx] = rowIdx[i];
	  triMap[i] = curIdx;
	  curIdx++;
	}
      }
    }}
    newHead[n] = curIdx;
    break;
  case (int) UPPER:
    {for( int j=0; j<n; ++j ) {
      newHead[j] = curIdx;
      for( int i=colPtr[j]; i<colPtr[j+1]; ++i ) {
	if ( rowIdx[i] <= j ) {
	  newList[curIdx] = rowIdx[i];
	  triMap[i] = curIdx;
	  curIdx++;
	}
      }
    }}
    newHead[n] = curIdx;
    break;  
  case (int) STRICT_UPPER:
    {for( int j=0; j<n; ++j ) {
      newHead[j] = curIdx;
      for( int i=colPtr[j]; i<colPtr[j+1]; ++i ) {
	if ( rowIdx[i] < j ) {
	  newList[curIdx] = rowIdx[i];
	  triMap[i] = curIdx;
	  curIdx++;
	}
      }
    }}
    newHead[n] = curIdx;
    break;
  default:
    ERROR( SPINDLE_ERROR_UNEXP_CASE, "Unexpected case%d\n", triangularOption );
  }
  matrix->getColPtr().swap( newHead );
  matrix->getRowIdx().swap( newList );
  matrix->nNonZeros = curIdx;
  matrix->getRowIdx().resize( curIdx );  // set current size correctly

  const int triNNZ = curIdx;
  if ( matrix->getRealValues().size() == oldNNZ ) {
    // create new real values
    SharedArray<double> triRealValues( triNNZ );
    const double* realValues = matrix->getRealValues().lend();
    for( int i=0; i<oldNNZ; ++i ) {
      if ( triMap[i] >= 0 ) {
	triRealValues[triMap[i]] = realValues[i];
      }
    }
    matrix->getRealValues().swap( triRealValues );
    matrix->getRealValues().resize( triNNZ );
  }
  if ( matrix->getImgValues().size() == oldNNZ ) {
    // create new real values
    SharedArray<double> triImgValues( triNNZ );
    const double* imgValues = matrix->getImgValues().lend();
    for( int i=0; i<oldNNZ; ++i ) {
      if ( triMap[i] >= 0 ) {
	triImgValues[triMap[i]] = imgValues[i];
      }
    }
    matrix->getImgValues().swap( triImgValues );
    matrix->getImgValues().resize( triNNZ );
  }
  FRETURN( true );
}
#undef __FUNC__

#define __FUNC__ "bool MatrixUtils::makeDiagonalsExplicit( MatrixBase* matrix )"
bool
MatrixUtils::makeDiagonalsExplicit( MatrixBase* matrix ) {
  FENTER;
  const int n = matrix->queryNCols();
  int insertNSelfEdges = n;

  const int * colPtr = matrix->getColPtr().lend();
  const int * rowIdx = matrix->getRowIdx().lend();
  {for( int i=0; i<n; ++i ) {
    for( int j=colPtr[i]; j<colPtr[i+1]; ++j ) {
      if ( rowIdx[j]==i ) {
	--insertNSelfEdges;
	break;
      }
    }
  }}
  if ( insertNSelfEdges > 0 ) { // we have self-edges to insert
    // create some temporary arrays that will become the new ones.
    const int oldNNZ = matrix->queryNNonZeros();
    const int newNNZ = oldNNZ + insertNSelfEdges;
    SharedArray<int> newColPtr( n + 1 );
    SharedArray<int> newRowIdx( newNNZ );
    SharedArray<int> scatter;
    bool do_scatter = false;
    int *p = newRowIdx.begin(); // only owner can give non-const pointer
    SharedArray<double> newRealValues;
    if ( matrix->getRealValues().size() == oldNNZ ) {
      newRealValues.resize( newNNZ );
      do_scatter = true;
    }
    SharedArray<double> newImgValues;
    if ( matrix->getImgValues().size() == oldNNZ ) {
      newImgValues.resize( newNNZ );
      do_scatter = true;
    }
    int offset = 0;
    if ( do_scatter ) {
      scatter.resize( oldNNZ );
    }
    {for( int i=0; i<n; ++i ) {
      newColPtr[i] = colPtr[i] + offset;
      int j;
      for( j=colPtr[i]; (j<colPtr[i+1]) && (rowIdx[j]<i); ++j, ++p ) {
	*p = rowIdx[j];
	if ( do_scatter ) { 
	  scatter[j] = j + offset;
	}
      }
      if ( ( j == colPtr[i+1] ) || ( rowIdx[j] != i ) ) {
	*p = i;
	++p;
	++offset;
      }
      for( ; j<colPtr[i+1]; ++j, ++p ) {
	*p = rowIdx[j];
	if ( do_scatter ) { 
	  scatter[j] = j + offset;
	}
      }
    }}
    newColPtr[n] = colPtr[n] + offset;
    matrix->nNonZeros = newNNZ;
    newColPtr.swap( matrix->getColPtr() );
    newRowIdx.swap( matrix->getRowIdx() );
    if ( do_scatter ) {
      if ( matrix->getRealValues().size() == oldNNZ ) {
	newRealValues.resize(newNNZ);
	newRealValues.init(0.0);
	const double* realValue = matrix->getRealValues().lend();
	double* newRealValue = newRealValues.begin();
	for( int i=0; i<oldNNZ; ++i ) {
	  newRealValue[ scatter[i] ] = realValue[i];
	}
	newRealValues.swap( matrix->getRealValues() );
      }
      if ( matrix->getImgValues().size() == oldNNZ ) {
	newImgValues.resize(newNNZ);
	newImgValues.init(0.0);
	const double* imgValue = matrix->getImgValues().lend();
	double* newImgValue = newImgValues.begin();
	for( int i=0; i<oldNNZ; ++i ) {
	  newImgValue[ scatter[i] ] = imgValue[i];
	}
	newImgValues.swap( matrix->getImgValues() );
      }
    }
    if ( matrix->getDiagPtr().size() == n ) {
      // must recompute this as well
      if (  matrix->setDiagPtr() == false ) {
	// error computing self edge, 
	FRETURN( false );
      }
    }
  } // end if insertNSelfEdges > 0 
  FRETURN( true );
}
#undef __FUNC__

#define __FUNC__ "bool MatrixUtils::makeSymmetric( MatrixBase* matrix )"
bool
MatrixUtils::makeSymmetric( MatrixBase* matrix ) {
  FENTER;
  const int n = matrix->queryNCols();
  if ( n != matrix->queryNRows() ) { 
    FRETURN( false );
  }
  //  if ( ! makeDiagonalsExplicit( matrix ) ) { 
  //    FRETURN( false );
  //  }
  if ( isSymmetric( matrix ) ) { 
    FRETURN( true );
  }

  const int nnz = matrix->queryNNonZeros();
  
  MatrixBase transpose;
  SharedArray<int> Tmap(nnz); // map from indices of transposed matrix to indicies of original
  MatrixUtils::deepCopy( matrix, &transpose );
  makeTranspose( &transpose , Tmap );

  // now have to merge transpose and matrix together.
  SharedArray<int> symHead(n+1);
  SharedArray<int> symList(2*nnz);  // adjacency list for symmetric matrix
  SharedArray<int> symMap(2*nnz);   // map from symmetric matrix to nonzeros of original

  const int* colPtr = matrix->getColPtr().lend();
  const int* rowIdx = matrix->getRowIdx().lend();
  const int* TcolPtr = transpose.getColPtr().lend();
  const int* TrowIdx = transpose.getRowIdx().lend();

  int curIdx = 0;
  {for( int i=0; i<n; ++i ) {
    symHead[i] = curIdx;
    const int* cur = rowIdx + colPtr[i];
    const int* stop = rowIdx + colPtr[i+1];
    const int* Tcur = TrowIdx + TcolPtr[i];
    const int* Tstop = TrowIdx + TcolPtr[i+1];
    while(  (cur<stop) ||  (Tcur<Tstop) ) {
      if ( (cur<stop) && (Tcur<Tstop) ) {
	if ( *cur == *Tcur ) {
	  symList[curIdx] = *cur;
	  symMap[curIdx] = cur - rowIdx;
	  ++cur;
	  ++Tcur;
	  ++curIdx;
	} else if ( *cur < *Tcur ) {
	  symList[curIdx] = *cur;
	  symMap[curIdx] = cur - rowIdx;
	  ++cur;
	  ++curIdx;
	} else {
	  symList[curIdx] = *Tcur;
	  symMap[curIdx] = Tmap[ Tcur - TrowIdx ];
	  ++Tcur;
	  ++curIdx;
	}
      } else if ( cur<stop ) {
	while( cur<stop ) {
	  symList[curIdx] = *cur;
	  symMap[curIdx] = cur - rowIdx;
	  ++cur;
	  ++curIdx;
	}
      } else {
	while( Tcur<Tstop ) {
	  symList[curIdx] = *Tcur;
	  symMap[curIdx] = Tmap[ Tcur - TrowIdx ];
	  ++Tcur;
	  ++curIdx;
	}
      }
    }
  }}
  const int symNNZ = curIdx;
  symHead[n] = symNNZ;

  // swap with real matrix.
  matrix->getColPtr().swap( symHead );
  matrix->getRowIdx().swap( symList );
  matrix->getRowIdx().resize( symNNZ ); // doesn't delete data if newsize < maxSize

  // now symMatrix is computed.  Check if other things need taking care of
  if ( matrix->getRealValues().size() == nnz ) {
    // create new real values
    SharedArray<double> symRealValues( symNNZ );
    const double* realValues = matrix->getRealValues().lend();
    for( int i=0; i<symNNZ; ++i ) {
      symRealValues[i] = - realValues[symMap[i]];
    }
    matrix->getRealValues().swap( symRealValues );
  }
  if ( matrix->getImgValues().size() == nnz ) {
    // create new real values
    SharedArray<double> symImgValues( symNNZ );
    const double* imgValues = matrix->getImgValues().lend();
    for( int i=0; i<symNNZ; ++i ) {
      symImgValues[i] = imgValues[symMap[i]];
    }
    matrix->getImgValues().swap( symImgValues );
  }
  //
  // finally take advantage of friendship
  //
  matrix->nNonZeros = symNNZ;
  //

  FRETURN( true );
}
#undef __FUNC__

#define __FUNC__ "bool MatrixUtils::makeTranspose( MatrixBase* matrix )"
bool 
MatrixUtils::makeTranspose( MatrixBase* matrix ) {
  FENTER;
  const int* colPtr = matrix->getColPtr().lend();
  const int* rowIdx = matrix->getRowIdx().lend();
  const int m = matrix->queryNRows();
  const int n = matrix->queryNCols();
  const int nnz = matrix->queryNNonZeros();

  SharedArray<int> TcolPtr(m+1);
  TcolPtr.init( 0 );
  
  SharedArray<int> TrowIdx(nnz);
  
  // count rows
  {for( int i=0; i<nnz; ++i ) {
    // PRINTF("rowIdx[%d] = %d\n",i,rowIdx[i] );
    TcolPtr[ rowIdx[i] + 1]++; // note shifting by one, tHead = length m+1
  }}

  // now sum the rows 
  {for( int i=1; i<=n; ++i ) {
    TcolPtr[i] += TcolPtr[i-1];
  }}

  // make an extra copy
  SharedArray<int> temp;
  temp.import( TcolPtr );

  if ( ( matrix->getRealValues().size() == 0 ) &&
       ( matrix->getImgValues().size() == 0 ) ) {
    // don't need Tmap, just generate TrowIdx
    for( int i=0; i<n; ++i ) {
      for( int j=colPtr[i]; j<colPtr[i+1]; ++j ) {
	int idx = temp[ rowIdx[j] ] ++;
	TrowIdx[ idx ] = i;
      }
    }
  } else {
    // generate Tmap for the values
    SharedArray<int> Tmap(nnz);
    {for (int i=0; i<n; i++) {
      for (int j=colPtr[i]; j<colPtr[i+1]; j++) {
	int idx = temp[rowIdx[j]]++;
	TrowIdx[idx] = i;
	Tmap[j] = idx;
      }
    }}
    if ( matrix->getRealValues().size() != 0 ) {
      const double* realValues = matrix->getRealValues().lend();
      SharedArray<double> newRealValues( nnz );
      for( int i=0; i<nnz; ++i) {
	newRealValues[ Tmap[i] ] = realValues[ i ];
      }
      newRealValues.swap( matrix->getRealValues() );
    }
    if ( matrix->getImgValues().size() != 0 ) {
      const double* imgValues = matrix->getImgValues().lend();
      SharedArray<double> newImgValues( nnz );
      for( int i=0; i<nnz; ++i) {
	newImgValues[ Tmap[i] ] = imgValues[ i ];
      }
      newImgValues.swap( matrix->getImgValues() );
    }
  } // if Tmap

  ////
  //// now exploit friendship and adjust some things
  int temp_int = matrix->nRows;
  matrix->nRows = matrix->nCols;
  matrix->nCols = temp_int;
  ////
  ////

  TcolPtr.swap( matrix->getColPtr() );
  TrowIdx.swap( matrix->getRowIdx() );
  if ( matrix->getDiagPtr().size() != 0 ) {
    // must recompute this as well
    if (  matrix->setDiagPtr() == false ) {
      // error computing self edge, 
      FRETURN( false );
    }
  }
  FRETURN( true );
}  
#undef __FUNC__


#define __FUNC__ "bool MatrixUtils::makeTranspose( MatrixBase* matrix, SharedArray<int>& Tmap )"
bool 
MatrixUtils::makeTranspose( MatrixBase* matrix, SharedArray<int>& Tmap ) {
  FENTER;
  const int* colPtr = matrix->getColPtr().lend();
  const int* rowIdx = matrix->getRowIdx().lend();
  const int m = matrix->queryNRows();
  const int n = matrix->queryNCols();
  const int nnz = matrix->queryNNonZeros();

  SharedArray<int> TcolPtr(m+1);
  TcolPtr.init( 0 );
  
  SharedArray<int> TrowIdx(nnz);
  
  // count rows
  {for( int i=0; i<nnz; ++i ) {
    TcolPtr[ rowIdx[i] + 1]++; // note shifting by one, tHead = length m+1
  }}

  // now sum the rows 
  {for( int i=1; i<=n; ++i ) {
    TcolPtr[i] += TcolPtr[i-1];
  }}

  // make an extra copy
  SharedArray<int> temp;
  temp.import( TcolPtr );

  // generate Tmap for the values
  Tmap.resize(nnz);
  {for (int i=0; i<n; i++) {
    for (int j=colPtr[i]; j<colPtr[i+1]; j++) {
      int idx = temp[rowIdx[j]]++;
      TrowIdx[idx] = i;
      Tmap[j] = idx;
    }
  }}

  // now we have a map for the other values
  if ( matrix->getRealValues().size() != 0 ) {
    const double* realValues = matrix->getRealValues().lend();
    SharedArray<double> newRealValues( nnz );
    for( int i=0; i<nnz; ++i) {
      newRealValues[ Tmap[i] ] = realValues[ i ];
    }
    newRealValues.swap( matrix->getRealValues() );
  }
  if ( matrix->getImgValues().size() != 0 ) {
    const double*  imgValues = matrix->getImgValues().lend();
    SharedArray<double> newImgValues( nnz );
    for( int i=0; i<nnz; ++i) {
      newImgValues[ Tmap[i] ] = imgValues[ i ];
    }
    newImgValues.swap( matrix->getImgValues() );
  }
  
  ////
  //// now exploit friendship and adjust some things
  int temp_int = matrix->nRows;
  matrix->nRows = matrix->nCols;
  matrix->nCols = temp_int;
  ////
  ////
  
  TcolPtr.swap( matrix->getColPtr() );
  TrowIdx.swap( matrix->getRowIdx() );  
  if ( matrix->getDiagPtr().size() != 0 ) {
    // must recompute this as well
    if (  matrix->setDiagPtr() == false ) {
      // error computing self edge, 
      FRETURN( false );
    }
  }
  FRETURN( true );
}  
#undef __FUNC__


#define __FUNC__ "bool MatrixUtils::deepCopy( const MatrixBase* src , MatrixBase* dest)"
bool 
MatrixUtils::deepCopy( const MatrixBase* src , MatrixBase* dest) {
  FENTER;
  if ( src==0 ) { return 0; }
  if ( dest==0 ) { return 0; }
  dest->nRows = src->nRows;
  dest->nCols = src->nCols;
  dest->nNonZeros = src->nNonZeros;
  if ( src->getColPtr().notNull() ) {
    dest->colPtr.import( src->getColPtr() );
  } else { 
    dest->colPtr.resize(0);
  }
  if ( src->getRowIdx().notNull() ) {
    dest->rowIdx.import( src->getRowIdx() );
  } else { 
    dest->rowIdx.resize(0);
  }

  if ( src->getDiagPtr().notNull() ) {
    dest->diagPtr.import( src->getDiagPtr() );
  } else { 
    dest->diagPtr.resize(0);
  }

  if ( src->getRealValues().notNull() ) {
    dest->realValues.import( src->getRealValues() );
  } else { 
    dest->realValues.resize(0);
  }

  if ( src->getImgValues().notNull() ) {
    dest->imgValues.import( src->getImgValues() );
  } else { 
    dest->imgValues.resize(0);
  }

  FRETURN( true );
}
#undef __FUNC__

