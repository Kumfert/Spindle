//
// testMatrixUtils_1.cc --tests basic MatrixBase operations
//
//
//
//

#include <iostream.h>
#include <iomanip.h>
#include "spindle/MatrixBase.h"
#include "spindle/MatrixUtils.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

//
// This is a simple derived class to set some of the information
// conveniently.  "MatrixBase" wasn't designed to be used directly.
//
class MatrixTest : public MatrixBase { 
public: 
  void setNRows( int nrows ) { nRows = nrows; }
  void setNCols( int ncols ) { nCols = ncols; }
  void setNNZ( int nnz ) { nNonZeros = nnz; }
};


// this is a standard 3x3 grid using a 5pt. stencil
struct MatrixType { 
  int nRows;
  int nCols;
  int nnz;
  int colPtr[10];
  int rowIdx[33];
};

static struct MatrixType Matrix1 = { 9, 9, 33, { 0, 3, 7, 10, 14, 19, 23, 26, 30, 33 }, 
				  { 0, 1, 3,
				    0, 1, 2, 4, 
				    1, 2, 5,
				    0, 3, 4, 6,
				    1, 3, 4, 5, 7, 
				    2, 4, 5, 8, 
				    3, 6, 7, 
				    4, 6, 7, 8,
				    5, 7, 8 }};


// this is the same grid without diagonal elements
static struct MatrixType Matrix2 = { 9, 9, 24, { 0, 2, 5, 7, 10, 14, 17, 19, 22, 24 },
				     { /*0,*/ 1, 3,
					      0, /*1,*/ 2, 4, 
					      1, /*2,*/ 5,
					      0, /*3,*/ 4, 6,
					      1, 3, /*4,*/ 5, 7, 
					      2, 4, /*5,*/ 8, 
					      3, /*6,*/ 7, 
					      4, 6, /*7,*/ 8,
					      5, 7, /*8,*/
					      0, 1, 2, 3, 4, 5, 6, 7, 8 }}; // junk to fill up the array

int main() {
  MatrixTest mat1, mat2;

  // now set up mat1
  mat1.setNRows( Matrix1.nRows );
  mat1.setNCols( Matrix1.nCols );
  mat1.setNNZ( Matrix1.nnz );
  mat1.getColPtr().borrow( Matrix1.colPtr, Matrix1.nCols + 1 );
  mat1.getRowIdx().borrow( Matrix1.rowIdx, Matrix1.nnz );

  if ( ! MatrixUtils::isSymmetric( &mat1 ) ) {
    cout << "Warning, MatrixUtils incorrectly labeled mat1 as unsymmetric " << endl;
  }

  // now set up mat2
  mat2.setNRows( Matrix2.nRows );
  mat2.setNCols( Matrix2.nCols );
  mat2.setNNZ( Matrix2.nnz );
  mat2.getColPtr().borrow( Matrix2.colPtr, Matrix2.nCols + 1 );
  mat2.getRowIdx().borrow( Matrix2.rowIdx, Matrix2.nnz );

  // finally get mat2 to call setAdjSelf
  if ( ! MatrixUtils::makeDiagonalsExplicit( &mat2 ) ) {
    cout << "MatrixUtils::makeDiagonalsExplicit( MatrixBase* )" << endl;
  }
  
  // since mat2 borrowed from Matrix2.  The arrays should be different
  if ( mat2.getColPtr().lend() == Matrix2.colPtr ) {
    cout << "Warning, mat2 internal data (which was modified) matches unshared external data." << endl;
  }

  // verify that mat1 and mat2 now have the same number of non-zeros
  if ( mat1.queryNNonZeros() != mat2.queryNNonZeros() ) {
    cout << "Warning mat1 and mat2 do not have same number of non-zeros." << endl;
  }

  // finally print out both mat1 and mat2 and compare
  { 
    const int* colPtr1 = mat1.getColPtr().lend();
    const int* colPtr2 = mat2.getColPtr().lend();
    const int nCols = mat1.queryNCols();
    cout << " ColPtr1   ColPtr2 " << endl;
    for( int i=0 ; i<=nCols; ++i ) {
      cout << setw(4) << i << setw(4) << colPtr1[i] << setw(4) << colPtr2[i] << endl;
    }
  }

  {
    const int* rowIdx1 = mat1.getRowIdx().lend();
    const int* rowIdx2 = mat2.getRowIdx().lend();
    const int nnz = mat2.queryNNonZeros();
    cout << " RowIdx1   RowIdx2 " << endl;
    for( int i=0 ; i<nnz; ++i ) {
      cout << setw(4) << i << setw(4) << rowIdx1[i] << setw(4) << rowIdx2[i] << endl;
    }
  }

  // now make mat2 strictly upper triangular
  if ( ! MatrixUtils::makeTriangular( &mat2, MatrixUtils::STRICT_UPPER ) ) {
    cout << " Warning: trouble making triangular " << endl;
  }

  { 
    const int* colPtr = mat2.getColPtr().lend();
    const int* rowIdx = mat2.getRowIdx().lend();
    const int nCols = mat2.queryNCols();
    // create temporary vector 
    SharedArray<int> temphead;
    temphead.import( colPtr, nCols );

    cout << "Spy of the matrix" << endl;
    for( int i=0; i<nCols; ++i ) { 
      cout << setw(4) << i << ": ";
      for( int j=0; j<nCols; ++j ) {
	if ( (temphead[j] < colPtr[j+1]) && (rowIdx[ temphead[j] ] == i )) {
	  cout << setw(4) << j;
	  temphead[j]++;
	} else { 
	  cout << setw(4) << "";
	}
      }
      cout << endl;
    }
  }



  // now make mat2 symmetric 
  if ( ! MatrixUtils::makeSymmetric( &mat2 ) ) {
    cout << " Warning: trouble making mat2 symmetric again" << endl;
  }

  { 
    const int* colPtr = mat2.getColPtr().lend();
    const int* rowIdx = mat2.getRowIdx().lend();
    const int nCols = mat2.queryNCols();
    // create temporary vector 
    SharedArray<int> temphead;
    temphead.import( colPtr, nCols );

    cout << "Spy of the matrix" << endl;
    for( int i=0; i<nCols; ++i ) { 
      cout << setw(4) << i << ": ";
      for( int j=0; j<nCols; ++j ) {
	if ( (temphead[j] < colPtr[j+1]) && (rowIdx[ temphead[j] ] == i )) {
	  cout << setw(4) << j;
	  temphead[j]++;
	} else { 
	  cout << setw(4) << "";
	}
      }
      cout << endl;
    }
  }
}
    


