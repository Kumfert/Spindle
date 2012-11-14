//
// FortranEmulator.cc -- Emulates Fortran read/write routines. 
//
//  $Id: FortranEmulator.cc,v 1.2 2000/02/18 01:31:50 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1998, Old Dominion University.  All rights reserved.
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


#include "spindle/FortranEmulator.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

bool 
FortranEmulator::readArray( FILE * stream, 
			    const char * format,
			    const int nLines,
			    int * array,
			    const int arraySz) {
  char line[82];
  int intsPerLine;
  int colsPerInt;
  int calculatedLines;
  char expanded_line[162];
  int err;
  if ( ( err = sscanf(format," (%d%*c%d)", &intsPerLine, &colsPerInt)) < 2 ) {
    cerr << "Error: FortranEmulator::readArray()" << endl;
    cerr << "       Problem performing sscanf() on integer format \'" << format << "\'" << endl;
    cerr << "       Only scanned " << err << " of the two numbers." << endl;
    cerr << "       intsPerLine=" << intsPerLine << "   colsPerInt=" << colsPerInt << endl;
    return false;
  }
  calculatedLines = arraySz/intsPerLine + ( (arraySz%intsPerLine == 0) ? 0 : 1 );
  if ((nLines != -1) && (calculatedLines != nLines)) { 
    cerr << "Warning: FortranEmulator::readArray()" << endl 
	 << "         calculated " << calculatedLines << " lines, not " << nLines 
	 << "as specified." << endl;
  }
  int curIdx = 0;
  int curLine = 0;
  for( curLine=0; curLine < calculatedLines; curLine++) {
    // First get the line from the stream
    if (fgets(line, 82, stream ) == NULL) { 
      cerr << "Error: cannot read " << nLines << "th part of intarray." << endl;
      return false;
    }
    // Then write the line to the expanded line, inserting spaces when appropriate
    char *src = line;
    char *dest = expanded_line;
    for ( int j=1; ( (j<=80) && (*src!='\0') ); j++) {
      *dest++ = *src++;
      if (j%colsPerInt == 0) { *dest++ = ' '; }
    }
    *dest='\0';
    // Now we have expanded_line with all the required spaces.
    for (char *token = strtok( expanded_line," \t"); (token != NULL) && (curIdx < arraySz) ; token=strtok(0," \t") ) {
      int temp = atoi(token) -1;
      if (temp < 0 ) { break; }
      array[curIdx++] = temp;
    } // end for all tokens on line
  } // end for all lines
  return ( (curLine == calculatedLines) && (arraySz == curIdx) );
}



bool 
FortranEmulator::readArray( FILE * stream, 
			    const char * format, 
			    const int nLines, 
			    double * array, 
			    const int arraySz) {
  char line[82];
  int doublesPerLine;
  int colsPerDouble;
  int Precision;
  char expanded_line[162];
  if ( sscanf(format," (%d%*c%d.%d)", &doublesPerLine, &colsPerDouble, &Precision) < 3 ) {
    if ( sscanf( format," (%*d%*c%d%*c%d.%d)",  
		 &doublesPerLine, &colsPerDouble, &Precision) < 3 ) {
      cerr << "Error in Double format " << format <<  endl;
      return false;
    }
  }
 
  int calculatedLines = arraySz/doublesPerLine + ( (arraySz%doublesPerLine == 0) ? 0 : 1 );
  if ((nLines != -1) && (calculatedLines != nLines)) { 
    cerr << "Warning: calculated " << calculatedLines << " lines, not " << nLines << endl;
  }
  int curIdx = 0;
  int curLine = 0;
  for( curLine=0; curLine < calculatedLines; curLine++) {
    // First get the line from the stream
    if (fgets(line, 82, stream ) == NULL) { 
      cerr << "Error: cannot read " << nLines << "th part of intarray." << endl;
      return false;
    }
    // Then write the line to the expanded line, inserting spaces when appropriate
    char *src = line;
    char *dest = expanded_line;
    for ( int j=1; ( (j<=80) && (*src!='\0') ); j++) {
      *dest++ = *src++;
      if (j%colsPerDouble == 0) { *dest++ = ' '; }
    }
    *dest='\0';
    // Now we have expanded_line with all the required spaces.
    for (char *token = strtok( expanded_line," \t"); (token != NULL) && (curIdx<arraySz); token=strtok(0," \t") ) {
      double temp = atof(token);
      array[curIdx++] = temp;
    } // end for all tokens on line
  } // end for all lines
  return ( (curLine == calculatedLines) && (arraySz == curIdx) );
}


bool 
FortranEmulator::writeArray( FILE * stream, 
			     const char * format, 
			     const int nLines,
			     const int * array,
			     const int arraySz ) {
  int intsPerLine;
  int colsPerInt;
  if ( sscanf(format," (%d%*c%d)", &intsPerLine, &colsPerInt) < 2 ) {
    cerr << "Error in integer format " << format <<  endl;
    return false;
  }
  int calculatedLines = arraySz/intsPerLine + ( (arraySz%intsPerLine == 0) ? 0 : 1 );
  if ((nLines != -1) && (calculatedLines != nLines)) { 
    cerr << "Warning: calculated " << calculatedLines << " lines, not " << nLines << endl;
  }
  int curLine = 0;
  for (int i=0; i<arraySz; i++) {
    fprintf( stream, " %*d", colsPerInt-1, (array[i])+1);
    if ( ((i+1)%intsPerLine) == 0) { 
      fprintf( stream, "\n"); 
      curLine++;
    }
  }
  if ((arraySz)%intsPerLine != 0) { 
    fprintf( stream, "\n"); 
    curLine++; 
  }
  return (calculatedLines == curLine);
}

bool
FortranEmulator::writeArray( FILE * stream, 
			     const char * format, 
			     const int nLines, 
			     const double * array, 
			     const int arraySz) {
  int doublesPerLine = 4;
  int colsPerDouble = 20;
  int Precision = 10;
  if ( sscanf(format," (%d%*c%d.%d)", &doublesPerLine, &colsPerDouble, &Precision) < 3 ) {
    cerr << "Error in Double format " << format <<  endl;
    return false;
  }

  if ( (Precision+6)>colsPerDouble ) { 
    Precision = colsPerDouble-7;
  }
  int calculatedLines = arraySz/doublesPerLine + ( (arraySz%doublesPerLine == 0) ? 0 : 1 );
  if ((nLines != -1) && (calculatedLines != nLines)) { 
    cerr << "Warning: calculated " << calculatedLines << " lines, not " << nLines << endl;
  }
  int curLine =0;
  for (int i=0; i<arraySz; i++) {
    // GNU doesn't like lE in printing doubles ( although it is ANSI C standard )
#ifndef __GNUC__
    fprintf( stream, " %*.*lE", colsPerDouble-1, Precision-1, array[i]);
#else 
    fprintf( stream, " %*.*E", colsPerDouble-1, Precision-1, array[i]);
#endif
    if ( ((i+1)%doublesPerLine) == 0) { 
      fprintf( stream, "\n"); 
      curLine++;
    }
  }
  if ((arraySz)%doublesPerLine != 0) { 
    fprintf( stream, "\n"); 
    curLine++;
  }
  return (calculatedLines == curLine);
}

