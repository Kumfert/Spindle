//
// HarwellBoeingFile.cc  -- I/O routines for creating a graph from HarwellBoeing file
//
//  $Id: HarwellBoeingFile.cc,v 1.2 2000/02/18 01:31:51 kumfert Exp $
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
//
//
//

#ifndef SPINDLE_HARWELL_BOEING_FILE_H_
#include "spindle/HarwellBoeingFile.h"
#endif

#ifndef SPINDLE_FORTRAN_EMULATOR_H_
#include "spindle/FortranEmulator.h"
#endif

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifndef SPINDLE_MATRIX_UTILS_H_
#include "spindle/MatrixUtils.h"
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

bool 
HarwellBoeingFile::setTitle( const char* new_title ) {
  strncpy( Title, new_title, 72 );
  Title[72]='\0';
  return true;
}

bool 
HarwellBoeingFile::setKey( const char* new_key ) {
  strncpy( Key, new_key, 8 );
  Key[8]='\0';
  return true;
}

bool 
HarwellBoeingFile::setMATtype( const char* type ) { 
  char c0 = type[0];
  char c1 = type[1];
  char c2 = type[2];
  if (! ( (c0=='R') || (c0=='r') || (c0=='C') || (c0=='c') || (c0=='P') || (c0=='p'))) { return false; }
  if (! ( (c1=='S') || (c1=='s') || (c1=='U') || (c1=='u') || (c1=='H') || (c1=='h') || 
	  (c1=='Z') || (c1=='z') || (c1=='R') || (c1=='r'))) { return false; }
  if (! ( (c2=='A') || (c2=='a') || (c2=='E') || (c2=='a') ) ) { return false; }
  MATtype[0] = c0;
  MATtype[1] = c1;
  MATtype[2] = c2;
  return true;
}

bool 
HarwellBoeingFile::setRHStype( const char* type) { 
  char c0 = type[0];
  char c1 = type[1];
  char c2 = type[2];
  if (! ( (c0=='F') || (c0=='f') || (c0=='M') || (c0=='m'))) { return false; }
  if (! ( (c1=='G') || (c1=='f') || (c1==' '))) { return false; }
  if (! ( (c2=='X') || (c2=='x') || (c2==' '))) { return false; }
  RHStype[0] = c0;
  RHStype[1] = c1;
  RHStype[2] = c2;
  return true;
}

#ifdef __FUNC__
#undef __FUNC__
#endif

#define __FUNC__ "bool HarwellBoeingFile::readHeader( FILE * stream )"
bool 
HarwellBoeingFile::readHeader( FILE * stream ) {
  FENTER;
  // Line 1
  {
    if (fgets(line, 82, stream ) == NULL) { 
      ERROR( SPINDLE_ERROR_FILE_READ, "Cannot read 1st line of Harwell Boeing header.  " );
      FRETURN( false );
    }
    if (sscanf( line, "%72c%8c\n", Title, Key) < 1) { 
      WARNING("First line has no Title field");
    }
    Key[8] = '\0';
    Title[72] = '\0';
  }

  // Line 2
  {
    if (fgets(line, 82, stream ) == NULL) { 
      ERROR( SPINDLE_ERROR_FILE_READ ,"Cannot read 2nd line of Harwell Boeing header.  ");
      FRETURN( false );
    }  
    // scratch space
    char cTOT[15]; 
    char cPTR[15]; 
    char cIND[15]; 
    char cVAL[15]; 
    char cRHS[15];  
    int nScanned = sscanf(line,"%14c%14c%14c%14c%14c\n",cTOT,cPTR,cIND,cVAL,cRHS);
    if (nScanned < 3) { 
      ERROR( SPINDLE_ERROR_FILE_MISCERR, "Unexpected number (%d) of structures scanned in line 2.  ", nScanned);
      FRETURN( false );
    }
    // set nils at end
    cTOT[14] = '\0';  
    cPTR[14] = '\0';  
    cIND[14] = '\0';  
    cVAL[14] = '\0';  
    cRHS[14] = '\0';
    TOTcrd = atoi(cTOT);
    PTRcrd = atoi(cPTR);
    INDcrd = atoi(cIND);
    VALcrd = (nScanned > 3) ? atoi(cVAL) : 0;
    RHScrd = (nScanned > 4) ? atoi(cRHS) : 0;
  }

  // Line 3
  {
    if (fgets(line, 82, stream ) == NULL) { 
      ERROR( SPINDLE_ERROR_FILE_READ, "Cannot read 3rd line of Harwell Boeing header.  ");
      FRETURN( false );
    }  
    // character scratch spaces
    char junk[12]; 
    char cnRows[15];
    char cnCols[15];
    char cNNZero[15];
    char cNEltVal[15];
    int nScanned = sscanf(line,"%3c%11c%14c%14c%14c%s\n",MATtype,junk,cnRows,cnCols,cNNZero,cNEltVal);
    if (nScanned < 5) { 
      ERROR( SPINDLE_ERROR_FILE_MISCERR, "Unexpected number (%d) of structures scanned in line 3.  ", nScanned);
      FRETURN( false );
    }
    // set nils at end
    MATtype[3] = '\0';
    cnRows[14] = '\0'; 
    cnCols[14] = '\0'; 
    cNNZero[14] = '\0';
    cNEltVal[14] = '\0';
    nRows = atoi(cnRows);
    nCols = atoi(cnCols);
    nNonZeros = atoi(cNNZero);
    NEltVal = (nScanned > 5) ? atoi(cNEltVal) : 0;
  }

  // Line 4
  {
    if (fgets(line, 82, stream ) == NULL) { 
      ERROR( SPINDLE_ERROR_FILE_READ, "Cannot read 4th line of Harwell Boeing header.  ");
      FRETURN( false );
    }  
    int nScanned = sscanf(line,"%16c%16c%20c%20c\n",PTRfmt, INDfmt, VALfmt, RHSfmt);
    if ((nScanned < 3) || ( (nScanned < 4) && (RHScrd > 0 ))) { 
      ERROR( SPINDLE_ERROR_FILE_MISCERR, "Unexpected number (%d) of structures scanned in line 4.  ", nScanned);
      FRETURN( false );
    }
    PTRfmt[16] = '\0'; 
    INDfmt[16] = '\0'; 
    VALfmt[20] = '\0'; 
    RHSfmt[20] = '\0';
  }

  // Line 5
  if (RHScrd > 0) {
    if (fgets(line, 82, stream ) == NULL) {  
      ERROR( SPINDLE_ERROR_FILE_READ, "Cannot read 4th line of Harwell Boeing header.  ");
      FRETURN( false );
    }
    // scratch space
    char junk[12];
    char cnRHS[15];
    char cnRHSind[15];
    int nScanned = sscanf(line,"%3c%11c%14c%14c\n",RHStype,junk,cnRHS,cnRHSind);
    if ((nScanned !=0 ) && ( ( RHStype[0] == 'F' ) || (RHStype[0] == 'f' ))) { 
      nRHSind = 0;
      RHStype[3] = '\0'; 
      if (nScanned > 1 ) { 
	cnRHS[14] = '\0';
      } else {
	cnRHS[0]= '\0';
      }
      cnRHSind[0] = '\0';
      nRHS = atoi(cnRHS);
    } else if ((nScanned < 3) || ((nScanned < 4)&&( (RHStype[0]=='M')|| 
						    (RHStype[0]=='m') )) ) { 
      ERROR( SPINDLE_ERROR_FILE_MISCERR, 
	     "Unexpected number (%d) of structures scanned in line 5.  ", 
	     nScanned);
      FRETURN( false );
    } else {
      RHStype[3] = '\0'; 
      cnRHS[14] = '\0'; 
      cnRHSind[14] = '\0';
      nRHS = atoi(cnRHS);
      nRHSind = ( nScanned > 3) ? atoi(cnRHSind) : 0;
    }
  } else {  // RHScrd ==0
    RHStype[0] = '\0';
    nRHS = 0;
    nRHSind =0;
  }
  FRETURN( true );
}
#undef __FUNC__

#define __FUNC__ "bool HarwellBoeingFile::writeHeader( FILE * stream )"
bool 
HarwellBoeingFile::writeHeader( FILE * stream ) {
  FENTER;
  fprintf( stream, "%-72s%-8s\n", Title, Key);
  fprintf( stream, "%14d%14d%14d%14d%14d\n", TOTcrd, PTRcrd, INDcrd, VALcrd, RHScrd );
  fprintf( stream, "%3s%11s%14d%14d%14d\n", MATtype, "", nRows, nCols, nNonZeros);
  if ( (MATtype[0] == 'P') || (MATtype[0] == 'p') ) { // print pattern only
    fprintf ( stream, "%16s%16s\n", PTRfmt, INDfmt );
  } else if ( RHScrd==0 ) {  // print values, but not RHS info
    fprintf ( stream, "%16s%16s%20s\n", PTRfmt, INDfmt, VALfmt);
  } else { // nRHS != 0
    fprintf ( stream, "%16s%16s%20s%20s\n", PTRfmt, INDfmt, VALfmt, RHSfmt);
    fprintf ( stream, "%3s%11s%14d\n", RHStype, "", nRHS);
  }
  FRETURN( true );
}
#undef __FUNC__


#define __FUNC__ "bool HarwellBoeingFile::readData( FILE * stream )"
bool 
HarwellBoeingFile::readData( FILE * stream ) {
  FENTER;

  colPtr.resize( nCols + 1 );
  if ( ! FortranEmulator::readArray( stream, PTRfmt, PTRcrd, colPtr.begin(), nCols+1 )) {
    ERROR( SPINDLE_ERROR, "in FortranEmulator::readArray( ) for int[].");
    FRETURN( false );
  }

  rowIdx.resize( nNonZeros );
  if ( ! FortranEmulator::readArray( stream, INDfmt, INDcrd, rowIdx.begin(), nNonZeros )) {
    ERROR( SPINDLE_ERROR, "in FortranEmulator::readArray( ) for int[].");
    FRETURN( false );
  }

  if (VALcrd > 0) {
    // read matrix values
    if ( (MATtype[2] == 'A') || (MATtype[2] == 'a') ) { // if Matrix is assembled
      if ( (MATtype[0] == 'C') || (MATtype[0] == 'c') ) { // if Matix is complex assembled
	realValues.resize( nNonZeros );
	if ( ! FortranEmulator::readArray( stream, VALfmt, VALcrd/2, realValues.begin(), nNonZeros )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::readArray( ) for double[].");
	  FRETURN( false );
	}
	imgValues.resize( nNonZeros );
	if ( ! FortranEmulator::readArray( stream, VALfmt, VALcrd/2, imgValues.begin(), nNonZeros )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::readArray( ) for double[].");
	  FRETURN( false );
	}
      } else { // matrix is Real assembled
	realValues.resize( nNonZeros );
	if ( ! FortranEmulator::readArray( stream, VALfmt, VALcrd, realValues.begin(), nNonZeros )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::readArray( ) for double[].");
	  FRETURN( false );
	}
      }
    } else { // Matrix is finite element
      if ( (MATtype[0]=='C') || (MATtype[0]=='c') ) { // if Matix is complex
	realValues.resize( NEltVal );
	if ( ! FortranEmulator::readArray( stream, VALfmt, VALcrd/2, realValues.begin(), NEltVal )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::readArray( ) for double[].");
	  FRETURN( false );
	}
	imgValues.resize( NEltVal );
	if ( ! FortranEmulator::readArray( stream, VALfmt, VALcrd/2, imgValues.begin(), NEltVal )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::readArray( ) for double[].");
	  FRETURN( false );
	}
      } else { // matrix is Real
	realValues.resize( NEltVal );
	if ( ! FortranEmulator::readArray( stream, VALfmt, VALcrd, realValues.begin(), NEltVal )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::readArray( ) for double[].");
	  FRETURN( false );
	}
      } // end if matrix is Real, Finite element
    } // end if matrix is finite element

    // Now how about them right hand sides
    if ( nRHS > 0 ) { 
      if ( ( RHStype[0] == 'F' ) || ( RHStype[0] == 'f' ) ) { // if RHS is a dense vector
	int nRHSVals = nRows * nRHS;
	RHS_realValues.resize( nRHSVals );
	if ( ! FortranEmulator::readArray( stream, RHSfmt, -1, RHS_realValues.begin(), nRHSVals )) {
	  cerr << "Error: in FortranEmulator::readArray() for double[]" << endl;
	  FRETURN( false );
	}
      } else if ( ( MATtype[2] == 'A' )|| ( MATtype[2] == 'a' ) ) { // RHS is sparse array
	RHS_colPtr.resize( nRHS + 1 );
	if ( ! FortranEmulator::readArray( stream, PTRfmt, -1, RHS_colPtr.begin(), nRHS + 1) ) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::readArray( ) for int[].");
	  FRETURN( false );
	}
	RHS_rowIdx.resize( nRHSind );
	if ( ! FortranEmulator::readArray( stream, INDfmt, -1, RHS_rowIdx.begin(), nRHSind) ) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::readArray( ) for int[].");
	  FRETURN( false );
	}
	RHS_realValues.resize( nRHSind );
	if ( ! FortranEmulator::readArray( stream, RHSfmt, -1, RHS_realValues.begin(), nRHSind )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::readArray( ) for double[].");
	  FRETURN( false );
	}
      } else { // RHS is elemental
	int nRHSVal = nNonZeros * nRHS;
	RHS_realValues.resize( nRHSVal );
	if ( ! FortranEmulator::readArray( stream, RHSfmt, -1, RHS_realValues.begin(), nRHSVal )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::readArray( ) for double[].");
	  FRETURN( false );
	}
      } // end if RHS is dense, sparse, or elemental	
      if ( ( RHStype[1] == 'G' ) || ( RHStype[1] == 'g' ) ) { // if there are starting guesses
	int nINIT_Guess = nRows * nRHS;
	initialGuess.resize( nINIT_Guess );
	if ( ! FortranEmulator::readArray( stream, RHSfmt, -1, initialGuess.begin(), nINIT_Guess )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::readArray( ) for double[].");
	  FRETURN( false );
	}
      }
      if ( ( RHStype[2] == 'X' ) || ( RHStype[2] == 'x' ) ) { // if there are starting guesses
	int nEXACT_SOLN = nRows * nRHS;
	exactSolution.resize( nEXACT_SOLN );
	if ( ! FortranEmulator::readArray( stream, RHSfmt, -1, exactSolution.begin(), nEXACT_SOLN )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::readArray( ) for double[].");
	  FRETURN( false );
	}
      }
    }
  } // end if RHS exist
  FRETURN( true );
}
#undef __FUNC__
	
      
#define __FUNC__ "bool HarwellBoeingFile::read( FILE * stream )"
bool 
HarwellBoeingFile::read( FILE * stream ) {
  FENTER;
  // Quit if file pointer isn't valid 
  if (stream == NULL) {
    ERROR( SPINDLE_ERROR_ARG_BADPTR, "Null file pointer.");
    FRETURN( false );
  }
  if ( feof(stream) || ferror(stream) ) {
    ERROR( SPINDLE_ERROR_FILE_READ, "File cannot be read.  ");
    FRETURN( false );
  }

  if ( ! readHeader( stream ) ) { 
    ERROR( SPINDLE_ERROR, "HarwellBoeingFile::readHeader() failed.  ");
    FRETURN( false ); 
  }

  if (! readData( stream ) ) { 
    ERROR( SPINDLE_ERROR, "HarwellBoeingFile::readData() failed.  ");
    FRETURN( false ); 
  }

  if ( MATtype[1] == 'S' || MATtype[1] == 's' ) {
    MatrixUtils::makeSymmetric( this );
  }

  FRETURN( true );
}
#undef __FUNC__
    
#define __FUNC__ "bool HarwellBoeingFile::writeData( FILE * stream )"
bool 
HarwellBoeingFile::writeData( FILE * stream ) {
  FENTER;
  if ( ! FortranEmulator::writeArray( stream, PTRfmt, PTRcrd, colPtr.begin(), nCols+1 )) {
    ERROR( SPINDLE_ERROR, "in FortranEmulator::writeArray( ) for int[].");
    FRETURN( false );
  }
  if ( ! FortranEmulator::writeArray( stream, INDfmt, -1, rowIdx.begin(), nNonZeros )) {
    ERROR( SPINDLE_ERROR, "in FortranEmulator::writeArray( ) for int[].");
    FRETURN( false );
  }

  if (VALcrd > 0) {
    // read matrix values
    if ( (MATtype[2] == 'A') || (MATtype[2] == 'a') ) { // if Matrix is assembled
      if ( (MATtype[0] == 'C') || (MATtype[0]== 'c') ) { // if Matix is complex
	if ( ! FortranEmulator::writeArray( stream, VALfmt, -1/*VALcrd/2*/, realValues.begin(), nNonZeros )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::writeArray( ) for double[].");
	  FRETURN( false );
	}
	if ( ! FortranEmulator::writeArray( stream, VALfmt, -1/*VALcrd/2*/, imgValues.begin(), nNonZeros )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::writeArray( ) for double[].");
	  FRETURN( false );
	}
      } else { // matrix is Real
	if ( ! FortranEmulator::writeArray( stream, VALfmt, -1 /*VALcrd*/ , realValues.begin(), nNonZeros )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::writeArray( ) for double[].");
	  FRETURN( false );
	}
      }
    } else { // Matrix is finite element
      if ( (MATtype[0]=='C') || (MATtype[0]=='c') )  { // if Matix is complex
	if ( ! FortranEmulator::writeArray( stream, VALfmt, -1/*VALcrd/2*/, realValues.begin(), NEltVal )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::writeArray( ) for double[].");
	  FRETURN( false );
	}
	if ( ! FortranEmulator::writeArray( stream, VALfmt, -1/*VALcrd/2*/, imgValues.begin(), NEltVal )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::writeArray( ) for double[].");
	  FRETURN( false );
	}
      } else { // matrix is Real
	if ( ! FortranEmulator::writeArray( stream, VALfmt, -1/*VALcrd*/, realValues.begin(), NEltVal )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::writeArray( ) for double[].");
	  FRETURN( false );
	}
      } // end if matrix is Real, Finite element
    } // end if matrix is finite element

    // Now how about them right hand sides
    if ( nRHS > 0 ) { 
      if ( ( RHStype[0] == 'F' ) || ( RHStype[0] == 'f' ) ) { // if RHS is a dense vector
	int nRHSVals = nRows * nRHS;
	if ( ! FortranEmulator::writeArray( stream, RHSfmt, -1, RHS_realValues.begin(), nRHSVals )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::writeArray( ) for double[].");
	  FRETURN( false );
	}
      } else if ( ( MATtype[2] == 'A' ) || ( MATtype[2] == 'a' ) ) { // RHS is sparse array
	if ( ! FortranEmulator::writeArray( stream, PTRfmt, -1, RHS_colPtr.begin(), nRHS + 1) ) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::writeArray( ) for int[].");
	  FRETURN( false );
	}
	if ( ! FortranEmulator::writeArray( stream, INDfmt, -1, RHS_rowIdx.begin(), nRHSind) ) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::writeArray( ) for int[].");
	  FRETURN( false );
	}
	if ( ! FortranEmulator::writeArray( stream, RHSfmt, -1, RHS_realValues.begin(), nRHSind )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::writeArray( ) for double[].");
	  FRETURN( false );
	}
      } else { // RHS is elemental
	int nRHSVal = nNonZeros * nRHS;
	if ( ! FortranEmulator::writeArray( stream, RHSfmt, -1, RHS_realValues.begin(), nRHSVal )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::writeArray( ) for double[].");
	  FRETURN( false );
	}
      } // end if RHS is dense, sparse, or elemental	
      if ( ( RHStype[1] == 'G' ) || ( RHStype[1] == 'g' ) ) { // if there are starting guesses
	int nINIT_SOLN = nRows * nRHS;
	if ( ! FortranEmulator::writeArray( stream, RHSfmt, -1, initialGuess.begin(), nINIT_SOLN )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::writeArray( ) for double[].");
	  FRETURN( false );
	}
      }
      if ( ( RHStype[2] == 'X' ) || ( RHStype[2] == 'x' ) ) { // if there are starting guesses
	int nEXACT_SOLN = nRows * nRHS;
	if ( ! FortranEmulator::writeArray( stream, RHSfmt, -1, exactSolution.begin(), nEXACT_SOLN )) {
	  ERROR( SPINDLE_ERROR, "in FortranEmulator::writeArray( ) for double[].");
	  FRETURN( false );
	}
      }
    }
  } // end if RHS exist
  FRETURN( true );
}
#undef __FUNC__

#define __FUNC__ "bool HarwellBoeingFile::write( FILE * stream )"

bool 
HarwellBoeingFile::write( FILE * stream ) {
  FENTER;
  if ( MatrixUtils::isSymmetric( this ) ) {
    MATtype[1] = 'S';
    MatrixUtils::makeTriangular( this, MatrixUtils::LOWER );
  }
  
  if ( ! writeHeader( stream ) ) { 
    ERROR( SPINDLE_ERROR, "HarwellBoeingFile::writeHeader() failed.  ");
    FRETURN( false ); 
  }

  if (! writeData( stream ) ) { 
    ERROR( SPINDLE_ERROR, "HarwellBoeingFile::writeData() failed.  ");
    FRETURN( false ); 
  }

  FRETURN( true );
}
