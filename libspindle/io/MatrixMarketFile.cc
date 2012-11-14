//
//
//

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include "vector.h"
#include "function.h"
#include <strstream.h>
#include "algo.h" // sort
#else
#include <vector>
#include <functional>
#include <strstream>
#include <algorithm>  // sort
using namespace std;
#endif

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#include "spindle/MatrixMarketFile.h"


#ifndef SPINDLE_MATRIX_UTILS_H_
#include "spindle/MatrixUtils.h"
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

#define BUF_SIZE 2048
static char buffer[BUF_SIZE];

// a little struct to help with converting <I,J,V> triples to compressed column matrices
struct IJVtriples {
  int i;
  int j;
  int intval; // integer value
  double rval; // real value
  double ival; // imaginary value
  // constructors
  IJVtriples(int I, int J) 
    : i(I), j(J), intval(0), rval(0), ival(0) {}
  IJVtriples(int I, int J, int INTVAL) 
    : i(I), j(J), intval(INTVAL), rval(0), ival(0) {}
  IJVtriples(int I, int J, double RVAL) 
    : i(I), j(J), intval(0), rval(RVAL), ival(0) {}
  IJVtriples(int I, int J, double RVAL, double IVAL) 
    : i(I), j(J), intval(0), rval(RVAL), ival(IVAL) {}
};

// a function object that sorts <I,J,V> triples in column major order
struct IJVtriplesColSort : public binary_function< IJVtriples*, IJVtriples*, bool > {
  bool operator()(const IJVtriples& first, const IJVtriples& second ) const {
    if ( first.j < second.j ) { 
      return true;
    } else if ( first.j > second.j ) { 
      return false;
    } else { 
      if ( first.i < second.i ) { 
	return true;
      } else { 
	return false;
      }
    }
  }
};

#define __FUNC__ "bool MatrixMarketFile::read( FILE * stream )"
bool 
MatrixMarketFile::read( FILE * stream ) {
  FENTER;
  if ( stream == 0 ) {
    ERROR( SPINDLE_ERROR_FILE_MISCERR, "Stream pointer is NULL." );
    FRETURN( false );
  }

  // First try to grab the Matrix Market Banner
  { 
    string n_m_and_nnz = read_banner( stream );
    istrstream ist( n_m_and_nnz.c_str() );
    if ( !( ist >> nRows ) ) { 
      ERROR( SPINDLE_ERROR_FILE_MISCERR,"Cannot Determine nRows.");
      FRETURN( false );
    }
    if ( !(ist >> nCols ) ) { 
      ERROR( SPINDLE_ERROR_FILE_MISCERR,"Cannot Determine nCols.");
      FRETURN( false );
    }
    if ( !( ist >> nNonZeros ) ) { 
      format_qualifier = ARRAY;
    } else { 
      format_qualifier = COORDINATE;
    }
  }

  //
  // Now read in the first row of values, checking to
  // see how much we have.
  // 
  vector< IJVtriples > v;
  v.reserve( nNonZeros );
  { 
    // test the next line for the number (and type) of tokens
    istrstream ist( fgets( buffer, BUF_SIZE, stream ) );
    string str_i, str_j, str_real, str_img;
    if ( !( ist >> str_i ) ) { 
      ERROR( SPINDLE_ERROR_FILE_MISCERR,"Cannot read first row entry.");
      FRETURN( false );
    } 
    if ( !( ist >> str_j ) ) { 
      ERROR( SPINDLE_ERROR_FILE_MISCERR,"Cannot read first column index.");
      FRETURN( false );
    } 
    if ( !( ist >> str_real ) ) { 
      if ( field_qualifier == UNKNOWN_FIELD ) { 
	field_qualifier = PATTERN;
      } else if ( field_qualifier != PATTERN ) { 
	WARNING( "cannot read values from non-pattern file:  Assuming field=pattern.");
	field_qualifier = PATTERN;
      }
    } else { // if str_real has a value
      if ( str_real.find('.') == string::npos ) { 
	// no decimal point
	field_qualifier = INTEGER;
      } else { 
	field_qualifier = REAL; // at least
	// now check for complex
	if ( ist >> str_img ) {
	  field_qualifier = COMPLEX;
	}
      }      
    }
    if ( field_qualifier == COMPLEX ) { 
      v.push_back( IJVtriples( atoi(str_i.c_str()) -1,
			       atoi(str_j.c_str()) -1,
			       atof(str_real.c_str()),
			       atof(str_img.c_str()) ) );
    } else if ( field_qualifier == REAL ) { 
      v.push_back( IJVtriples( atoi(str_i.c_str()) -1,
			       atoi(str_j.c_str()) -1,
			       atof(str_real.c_str()) ) );
    } else if ( field_qualifier == INTEGER ) { 
      v.push_back( IJVtriples( atoi(str_i.c_str()) -1,
			       atoi(str_j.c_str()) -1,
			       atoi(str_real.c_str()) ) );
    } else if ( field_qualifier == PATTERN ) { 
      v.push_back( IJVtriples( atoi(str_i.c_str()) -1,
			       atoi(str_j.c_str()) -1) );
    }
  }

  //    
  // now scan in the rest of them.
  //

  int I;
  int J;
  int INTVAL;
  double RVAL;
  double IVAL;
  if (field_qualifier == COMPLEX ) { 
    for(int i=1; i<nNonZeros; ++i ) {
      fscanf( stream,"%d%d%lg%lg\n", &I, &J, &RVAL, &IVAL);
      --I; --J;
      v.push_back( IJVtriples( I, J, RVAL, IVAL ) );
    }
  } else if ( field_qualifier == REAL ) { 
    for(int i=1; i<nNonZeros; ++i ) {
      fscanf( stream,"%d%d%lg\n", &I, &J, &RVAL );
      --I; --J;
      v.push_back( IJVtriples( I, J, RVAL ) );
    }
  } else if ( field_qualifier == INTEGER ) { 
    for(int i=1; i<nNonZeros; ++i ) {
      fscanf( stream,"%d%d%d\n", &I, &J, &INTVAL );
      --I; --J;
      v.push_back( IJVtriples( I, J, INTVAL ) );
    }
  } else if ( field_qualifier == PATTERN ) {
    for(int i=1; i<nNonZeros; ++i ) {
      fscanf( stream,"%d%d\n", &I, &J );
      --I; --J;
      v.push_back( IJVtriples( I, J ) );
    }
  }

  // now sort it
  sort( v.begin(), v.end(), IJVtriplesColSort() );

  colPtr.resize(nCols + 1 );
  rowIdx.resize( nNonZeros );
  realValues.resize( nNonZeros );
  int lastCol = -1;
  IJVtriples* cur = v.begin();
  for ( int i=0; i<nNonZeros; ++i ) { 
    if ( cur->j != lastCol ) {
      lastCol = cur->j;
      colPtr[ lastCol ] = i;
    }
    rowIdx[i] = cur->i;
    realValues[i] = cur->rval;
    ++cur;
  }
  colPtr[ nCols ] = nNonZeros; 
  
  if ( symmetry_qualifier == SYMMETRIC ||
       symmetry_qualifier == HERMITIAN ) {
    MatrixUtils::makeSymmetric( this );
  }
  return true;
}
#undef __FUNC__



#define __FUNC__ "bool MatrixMarketFile::read_banner( FILE * stream )"
string
MatrixMarketFile::read_banner( FILE * stream ) {
  FENTER;
  string first_line = fgets( buffer, BUF_SIZE, stream );
  istrstream ist( first_line.c_str() );
  string token;
  if ( !(ist >> token )) {
    FRETURN("");
  }
  if ( strcasecmp( token.c_str(), "%%MatrixMarket" ) ) {
    // Not a MatrixMarket format... though it could just be 
    // a coordinate format
    format_qualifier = COORDINATE;
    field_qualifier = UNKNOWN_FIELD;
    symmetry_qualifier = GENERAL;
    FRETURN( first_line );
  }
  // assert: we've got a file with the "MatrixMarket header
  ist >> token;
  if ( strcasecmp( token.c_str(), "matrix" ) ) {
    FRETURN(""); 
  }
  // assert: second token is "matrix"
  ist >> token;
  if ( !strcasecmp( token.c_str(), "coordinate" ) ) {
    format_qualifier = COORDINATE;
  } else if ( !strcasecmp( token.c_str(), "array" ) ) {
    format_qualifier = ARRAY;
  } 
  // assert: format_qualifier is specified as best we can
  ist >> token;
  if ( !strcasecmp( token.c_str(), "pattern" ) ) {
    field_qualifier = PATTERN;
  } else if ( !strcasecmp( token.c_str(), "integer" ) ) {
    field_qualifier = INTEGER;
  } else if ( !strcasecmp( token.c_str(), "real" ) ) {
    field_qualifier = REAL;
  } else if ( !strcasecmp( token.c_str(), "complex" ) ) {
    field_qualifier = COMPLEX;
  } 
  // assert: field_qualifier is now specified as best we can
  ist >> token;
  if ( !strcasecmp( token.c_str(), "general" ) ) {
    symmetry_qualifier = GENERAL;
  } else if ( !strcasecmp( token.c_str(), "symmetric" ) ) {
    symmetry_qualifier = SYMMETRIC;
  } else if ( !strcasecmp( token.c_str(), "skew-symmetric" ) ) {
    symmetry_qualifier = SKEW_SYMMETRIC;
  } else if ( !strcasecmp( token.c_str(), "hermitian" ) ) {
    symmetry_qualifier = HERMITIAN;
  } 
  // assert: symmetry_qualifier is now specified as best we can.
  
  // now just skip through the rest of the lines 'til
  // we get to a data line
  comments = "";
  string next_line = fgets( buffer, BUF_SIZE, stream );
  while ( next_line[0] == '%' ) {
    comments += next_line.substr(1,string::npos);
    // comments += '\n';fgets leaves the trailing \n on.
    next_line = fgets( buffer, BUF_SIZE, stream );
  }
  
  FRETURN( next_line );
}
#undef __FUNC__

#define __FUNC__ "bool MatrixMarketFile::write( FILE * stream )"
bool 
MatrixMarketFile::write( FILE * stream ) {
  FENTER;
  if ( stream == 0 ) {
    ERROR( SPINDLE_ERROR_FILE_MISCERR, "Stream pointer is NULL." );
    FRETURN( false );
  }

  if ( standard_header ) { // write the standard header out
    fprintf( stream, "%s", "%%MatrixMarket matrix" );
    if ( format_qualifier == COORDINATE ) { 
      fprintf( stream, "%s", " coordinate" );
    } else if ( format_qualifier == ARRAY ) { 
      fprintf( stream, "%s", " array" );
    } else {
      ERROR( SPINDLE_ERROR_FILE_MISCERR, "format_qualifier unknown.");
      FRETURN( false );
    }
    if ( field_qualifier == PATTERN ) { 
      fprintf( stream, "%s", " pattern" );
    } else if ( field_qualifier == INTEGER ) { 
      fprintf( stream, "%s", " integer" );
    } else if ( field_qualifier == REAL ) { 
      fprintf( stream, "%s", " real" );   
    } else if ( field_qualifier == COMPLEX ) { 
      fprintf ( stream, "%s", " complex" );
    } else { 
      ERROR( SPINDLE_ERROR_FILE_MISCERR, "field_qualifier unknown.");
      FRETURN( false );
    }
    if ( symmetry_qualifier == GENERAL ) { 
      fprintf( stream, "%s\n", " general" );
    } else if ( symmetry_qualifier == SYMMETRIC ) { 
      fprintf( stream, "%s\n", " symmetric" );
      MatrixUtils::makeTriangular( this, MatrixUtils::LOWER );
    } else if ( symmetry_qualifier == SKEW_SYMMETRIC ) { 
      fprintf( stream, "%s\n", " skew-symmetric" );
      MatrixUtils::makeTriangular( this, MatrixUtils::LOWER );
    } else if ( symmetry_qualifier == HERMITIAN ) { 
      fprintf( stream, "%s\n", " hermitian" );
      MatrixUtils::makeTriangular( this, MatrixUtils::LOWER );
    } else { 
      ERROR( SPINDLE_ERROR_FILE_MISCERR, "symmetry_qualifier unknown.");
      FRETURN( false );
    }

    // Now print the comments, if any
    if (comments != "" ) {  
      istrstream ist(comments.c_str() );
      string line;
      while( getline( ist, line ) ) {
	fprintf( stream, "%s%s\n", "%", line.c_str() );
      }
    }

  } // end if standard_header
  
  // next print nRows, nCols, nNonZeros
  fprintf( stream, "%d %d %d\n", nRows, nCols, nNonZeros );

  // finally print the I.J.val stuff.
  const int * col = colPtr.lend();
  const int * row = rowIdx.lend();
  const double * rv = realValues.lend();
  const double * iv = imgValues.lend();
  for ( int i =0; i< nCols; ++i ) { 
    for ( int j = col[i]; j<col[i+1]; ++j ) { 
      if (field_qualifier == COMPLEX ) { 
	fprintf( stream,"%d%d%g%g\n", i+1, row[j]+1, rv[j], iv[j] );
      } else if ( field_qualifier == REAL ) { 
	fprintf( stream,"%d%d%g\n", i+1, row[j]+1, rv[j] );
      } else if ( field_qualifier == INTEGER ) { 
	fprintf( stream,"%d%d%d\n", i+1, row[j]+1, (int) rv[j] );
      } else if ( field_qualifier == PATTERN ) {
	fprintf( stream,"%d%d\n", i+1, row[j]+1 );
      }
    }
  }
  FRETURN ( true );
}
