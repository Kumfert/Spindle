//
// ChristensenFile.cc -- Peter Christensen's Format
//
//
//

#include "spindle/ChristensenFile.h"

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifndef SPINDLE_MATRIX_UTILS_H_
#include "spindle/MatrixUtils.h"
#endif

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include "algo.h"
#else
#include <algorithm>
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

#ifdef __FUNC__
#undef __FUNC__
#endif

#define __FUNC__ "bool ChristensenFile::read( FILE * stream )"
bool 
ChristensenFile::read( FILE * stream ) {
  FENTER;
  if ( stream == 0 ) {
    ERROR( SPINDLE_ERROR_FILE_MISCERR, "Stream pointer is NULL." );
    FRETURN( false );
  }

  // First, read the matrix size and the number of values in the 
  // lower triangle (higher-adj)
  const int bufferSize = 256;
  char buffer[ bufferSize ];
  //  int nCols;           Number of colums  inherited from MatrixBase
  //  int nNonZeros;       Number of nonZeros  inherited from Matrix Base
   
  // First, get the dimension of the matrix
  if ( fgets( buffer, bufferSize, stream ) != buffer ) {
    ERROR( SPINDLE_ERROR_FILE_READ, "Unable to read from stream.");
    FRETURN( false );
  }
  if ( sscanf( buffer,"%d", &nCols) != 1) {
    ERROR( SPINDLE_ERROR_FILE_MISCERR,
	   "Unexpected case when attempting to read header of ChristensenFile.");
    FRETURN(false);
  }
  nRows = nCols;

  // Next get the number of nonzero elements
  if ( fgets( buffer, bufferSize, stream ) != buffer ) {
    ERROR( SPINDLE_ERROR_FILE_READ, "Unable to read from stream.");
    FRETURN( false );
  }
  if( sscanf( buffer,"%d", &nNonZeros) != 1 ) { 
    ERROR( SPINDLE_ERROR_FILE_MISCERR, 
	   "Unexpected case when attempting to read header of ChristensenFile.");
    FRETURN(false);
  }
  
  
  // Now get the number of columns in block 22.
  int nColsInBlock22;
  if ( fgets( buffer, bufferSize, stream ) != buffer ) {
    ERROR( SPINDLE_ERROR_FILE_READ, "Unable to read from stream.");
    FRETURN( false );
  }
  if ( sscanf( buffer,"%d", &nColsInBlock22) != 1 ) {
    ERROR( SPINDLE_ERROR_FILE_MISCERR,
	   "Unexpected case when attempting to read header of ChristensenFile.");
    FRETURN(false);
  }
  colsInBlock22.reserve( nColsInBlock22 );

  // Now load in all the edges
  typedef vector< pair< int, int> > EdgeList_t;
  EdgeList_t edgeList;
  edgeList.reserve( nNonZeros );
  {for( int i=0; i<nNonZeros; ++i ) { 
    int i_row, j_col;
    if ( fscanf( stream, "%d%d", &j_col, &i_row) != 2 ) {
      ERROR( SPINDLE_ERROR_FILE_TOOSHORT, "File terminated prematurely."); 
      FRETURN( false );
    }
    edgeList.push_back( pair<int,int>( j_col, i_row ) );
  }}

  // Now sort them, column first then row
  sort( edgeList.begin(), edgeList.end(), pair_first_then_second_less<int,int>() );
  
  // Create a mapping from gapped sequence, one with no gaps
  typedef map<int,int,less<int> > map_t;
  map_t mapping;
  {for( EdgeList_t::const_iterator it = edgeList.begin(), stop_it = edgeList.end(); it != stop_it; ++it ) { 
    mapping[ (*it).first ] = 0;
    mapping[ (*it).second] = 0;
  }}

  // Now fill in the details
  int curIdx = 0;
  {for( map_t::iterator it = mapping.begin(), stop_it = mapping.end(); it != stop_it; ++it ) { 
    (*it).second = curIdx++;
  }}

  // now, the mapping should map the sequence 
  {for( EdgeList_t::iterator it = edgeList.begin(), stop_it = edgeList.end(); it != stop_it; ++it ) { 
    (*it).first = mapping[ (*it).first ];
    (*it).second = mapping[ (*it).second];
  }}
  
  // allocate space for the matrix structures
  colPtr.resize( nCols + 1 );
  rowIdx.resize( nNonZeros );
  int * col_ptr = colPtr.begin();
  int * row_idx = rowIdx.begin();
  int cur_idx = 0;
  int cur_col = -1;
  col_ptr[0] = 0;
  {for( int i=0; i<nNonZeros; ++i ) {
    int i_row, j_col;
    j_col = edgeList[i].first;
    i_row = edgeList[i].second;
    if ( j_col > cur_col ) { // we have a new column
      col_ptr[ j_col ] = cur_idx; // colPtr points to current index.
      ++cur_col;
    } else if ( j_col < cur_col ) { 
      ERROR( SPINDLE_ERROR_FILE_MISCERR, "Column indices not sorted." );
    }
    row_idx[ cur_idx++ ] = i_row;
  }}
  col_ptr[ nCols ] = cur_idx;

  {for( int i=0; i<nColsInBlock22; ++i ) {
    int col_id;
    if ( fscanf( stream, "%d", &col_id) != 1 ) {
      ERROR( SPINDLE_ERROR_FILE_TOOSHORT, "File terminated prematurely."); 
      FRETURN( false );
    }
    colsInBlock22.push_back( mapping[col_id] );
  }}

  bool success = MatrixUtils::makeSymmetric( this ) ; 
  FRETURN( success );
}
#undef __FUNC__


#define __FUNC__ "bool ChristensenFile::write( FILE * stream )"
bool
ChristensenFile::write( FILE * stream ) {
  FENTER;

  // first print the number of vertices and number of 
  MatrixUtils::makeDiagonalsExplicit( this );
  MatrixUtils::makeTriangular( this, MatrixUtils::LOWER );

  fprintf( stream, "%d\n%d\n%d\n", nCols, nNonZeros, colsInBlock22.size() );

  const int* col_ptr = colPtr.begin();
  const int* row_idx = rowIdx.begin();
  {for(int i=0;i<nCols; ++i) {
    for(int j=col_ptr[i]; j<col_ptr[i+1]; ++j ) { 
      fprintf(stream,"%d %d\n", i+1, row_idx[j]+1 );
    }
  }}
  
  {for( vector<int>::const_iterator iter = colsInBlock22.begin(), 
	 stop = colsInBlock22.end(); iter != stop ; ++iter ) { 
    fprintf( stream, "%d\n", *iter + 1 );
  }}

  return true;
}
