//
// DobrianFile.cc -- Florin's format
//
//
//

#include "spindle/DobrianFile.h"

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifndef SPINDLE_MATRIX_UTILS_H_
#include "spindle/MatrixUtils.h"
#endif

#ifdef __FUNC__
#undef __FUNC__
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

#define __FUNC__ "bool DobrianFile::read( FILE * stream )"
bool 
DobrianFile::read( FILE * stream ) {
  FENTER;
  if ( stream == 0 ) {
    ERROR( SPINDLE_ERROR_FILE_MISCERR, "Stream pointer is NULL." );
    FRETURN( false );
  }

  // First, read the matrix size and the number of values in the 
  // lower triangle (higher-adj)
  const int bufferSize=256;
  char buffer[ bufferSize ];
  
  if ( fgets( buffer, bufferSize, stream ) != buffer ) {
    ERROR( SPINDLE_ERROR_FILE_READ, "Unable to read from stream.");
    FRETURN( false );
  }

  //  int nCols;           // Number of colums // inherited from MatrixBase
  //  int nNonZeros;       // Number of nonZeros // inherited from Matrix Base
  int storageScheme;   // 0=lower triangle, 1=upper triangle, 2=square
  int format;          // 0=compressed, 1=uncompressed
  int valueStorage;    // 0=none, 1=real symmetric, 2=complex symmetric, 3=complex hermitian
  
  switch( sscanf( buffer,"%d%d%d%d%d", &nCols, &nNonZeros, &storageScheme, &format, &valueStorage ) ) {
  case 0:
  case 1:
    ERROR( SPINDLE_ERROR_FILE_TOOSHORT, "Could not find header information.");
    FRETURN( false );
    break; // unneccessary, but easier to read
  case 2:
    storageScheme = 0;
    // Programmer Note: no ``break'' I want fall through
  case 3:
    format = 0;
    // Programmer Note:  no ``break'' I want fall through
  case 4:
    valueStorage = 0;
    // Programmer Note: no ``break'' I want fall through
  case 5:
    break;
  default:
    ERROR( SPINDLE_ERROR_FILE_MISCERR,"Unexpected case when attempting to read header of DobrianFile.");
    FRETURN(false);
  }

  // allocate space for the matrix structures
  colPtr.resize( nCols + 1 );
  int * cur = colPtr.begin();
  // now load in the colPtr
  {for( int i=0; i<=nCols; ++i ) {
    if ( fscanf( stream,"%d", cur ) == 0 ) {
      ERROR( SPINDLE_ERROR_FILE_TOOSHORT, "File terminated prematurely."); 
      FRETURN( false );
    }
    (*cur) -= 1; // decrement value by one
    cur++;       // increment pointer
  }}

  int totalNNonZeros;
  if ( storageScheme != 2 ) {
    totalNNonZeros = nCols + nNonZeros;
  } else {
    totalNNonZeros = nNonZeros;
  }

  rowIdx.resize( totalNNonZeros );
  cur = rowIdx.begin();
  // now load rowIdx
  {for( int i=0; i<totalNNonZeros; ++i) { 
    if ( fscanf( stream,"%d", cur ) == 0 ) {
      ERROR( SPINDLE_ERROR_FILE_TOOSHORT, "File terminated prematurely. Got %d out of %d.  ", i, totalNNonZeros );
      FRETURN( false );
    }
    (*cur) -= 1; // decrement value by one
    cur++;       // increment pointer
  }}
  
  if ( valueStorage == 1 ) {
    // more values exist.  Try loading them as well
    realValues.resize( totalNNonZeros );
    double* dcur = realValues.begin();
    for( int i=0; i<totalNNonZeros; ++i ) {
      if ( feof( stream ) || (fscanf(stream,"%lf",dcur++ ) == 0 )) {
	// data ended prematurely
	WARNING("Real Values not loaded: file ended prematurely.");
	realValues.resize(0);
	break;
      }
    }
  } else if ( (valueStorage == 2) || (valueStorage == 3) ) {
    // more values exist.  Try loading them as well
    realValues.resize( totalNNonZeros );
    imgValues.resize(totalNNonZeros);
    double* dReal = realValues.begin();
    double* dImg = imgValues.begin();
    for( int i=0; i<totalNNonZeros; ++i ) {
      if ( feof( stream ) || (fscanf(stream,"%lf%lf",dReal++, dImg++ ) == 0 )) {
	// data ended prematurely
	WARNING("Imaginary Values not loaded: file ended prematurely.");
	imgValues.resize(0);
	break;
      }
    }
  }
  nRows = nCols;
  nNonZeros = totalNNonZeros;
  bool success = ( storageScheme != 2 ) ?  MatrixUtils::makeSymmetric( this ) : true;
  FRETURN( success );
}
#undef __FUNC__


#define __FUNC__ "bool DobrianFile::write( FILE * stream )"
bool
DobrianFile::write( FILE * stream ) {
  FENTER;

  // first print the number of vertices and number of 
  MatrixUtils::makeDiagonalsExplicit( this );

  if ( MatrixUtils::isSymmetric( this ) ) { 
    MatrixUtils::makeTriangular( this, MatrixUtils::LOWER );
  }
  int valueStorage;
  if ( realValues.size() == 0 ) {
    valueStorage = 0;
  } else if ( realValues.size() == nNonZeros ) {
    if ( imgValues.size() == 0 ) {
      valueStorage = 1;
    } else if ( imgValues.size() == nNonZeros ) {
      valueStorage = 2;
    } else {
      ERROR( SPINDLE_ERROR_FILE_MISCERR,
	     "Internal data consistancy error in DobrianFile: nNonZeros != nImgValues ");
      FRETURN( false );
    }
  } else { 
    ERROR( SPINDLE_ERROR_FILE_MISCERR,
	   "Internal data consistancy error in DobrianFile: nNonZeros != nRealValues");
    FRETURN( false );
  }
  //  if ( (imgValues.size() == realValues.size()) ) 
  fprintf( stream, "%9d %9d 0 0 %d\n", nCols, nNonZeros-nCols, valueStorage );
  
  const int* cur = colPtr.begin();
  {for(int i=0;i<=nCols; ) {
    for(int j=0; (j<8)&&(i<=nCols); ++j,++i ) {
      fprintf(stream," %9d",(*cur)+1);
      ++cur;
    }
    fprintf(stream,"\n");
  }}
  
  cur = rowIdx.begin();
  {for(int i=0;i<nNonZeros; ) {
    for(int j=0; (j<8)&&(i<nNonZeros); ++j,++i ) {
      fprintf(stream," %9d",(*cur)+1);
      ++cur;
    }
    fprintf(stream,"\n");
  }}

  if ( imgValues.size() == nNonZeros ) {
    double* curReal = realValues.begin();
    double* curImg = imgValues.begin();
    for(int i=0;i<nNonZeros; ) {
      for(int j=0; (j<2)&&(i<nNonZeros); ++j,++i ) {
#ifndef __GNUC__
	fprintf(stream," %19.12lE %19.12lE",*curReal++,*curImg++);
#else 
	fprintf(stream," %19.12E %19.12E",*curReal++,*curImg++);
#endif
      }
      fprintf(stream,"\n");
    }
  } else if ( realValues.size() == nNonZeros ) {
    double* curReal = realValues.begin();
    for(int i=0;i<nNonZeros; ) {
      for(int j=0; (j<4)&&(i<nNonZeros); ++j,++i ) {
#ifndef __GNUC__
	fprintf(stream," %19.12lE",*curReal++);
#else
	fprintf(stream," %19.12E",*curReal++);
#endif
      }
      fprintf(stream,"\n");
    }
  }

  return true;
}
