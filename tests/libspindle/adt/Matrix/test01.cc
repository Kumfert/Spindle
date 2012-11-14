//
// testMatrixBase_1.cc --tests basic MatrixBase operations
//
//
//
//

#include <iostream.h>
#include <iomanip.h>
#include "spindle/MatrixBase.h"
#include "samples/small_mesh.h"

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

int main() {
  MatrixTest mat1;

  // now set up mat1
  mat1.setNRows( nvtxs );
  mat1.setNCols( nvtxs );
  mat1.setNNZ( Aptr[nvtxs] );
  mat1.getColPtr().borrow( Aptr, nvtxs+1 );
  mat1.getRowIdx().borrow( Aind, Aptr[nvtxs] );

  // finally get mat1 to call setDiagPtr
  if ( !mat1.setDiagPtr() ) {
    cout << "mat1.setDiagPtr() failed " << endl;
  }

  // const int * colPtr = mat1.getColPtr().lend(); //unused
  const int * rowIdx = mat1.getRowIdx().lend();
  const int * diagPtr = mat1.getDiagPtr().lend();

  for( int i=0; i<nvtxs; ++i ) {
    cout << " DiagPtr[ " << i << "] =  " << rowIdx[ diagPtr[ i ] ] << endl;
  }
}
    
