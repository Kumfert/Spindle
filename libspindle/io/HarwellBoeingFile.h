//
// HarwellBoeingFile.h  -- I/O routines for creating a graph from HarwellBoeing file
//
//  $Id: HarwellBoeingFile.h,v 1.2 2000/02/18 01:31:51 kumfert Exp $
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
#define SPINDLE_HARWELL_BOEING_FILE_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifndef SPINDLE_MATRIX_BASE_H_
#include "spindle/MatrixBase.h"
#endif

#ifndef SPINDLE_TEXT_DATA_FILE_H_
#include "spindle/TextDataFile.h"
#endif

SPINDLE_BEGIN_NAMESPACE

class HarwellBoeingFile: public MatrixBase, public TextDataFile {
private:
  // Fields in the header
  // Line 1
  char Title[73]; // Title
  char Key[9];    // Key
  // Line 2
  int TOTcrd;      // Total number of lines excluding header
  int PTRcrd;      // Number of lines for pointers
  int INDcrd;      // Number of lines for indices
  int VALcrd;      // Number of lines for numerical values
  int RHScrd;      // Number of lines for right hand sides 
                   // (incl. initial guesses and exact solns if present)
  // Line 3
  char MATtype[4]; // Matrix type {R,C,P}{S,U,H,Z,R}{A,E}
  int NEltVal;     // number of elemental matrix entries (zero for assembled)

  // Line 4
  char PTRfmt[17];  // Format for pointers
  char INDfmt[17];  // Format for rwo (or variable) indices
  char VALfmt[21];  // Format for numerical values of coefficient matrix
  char RHSfmt[21];  // Format for numerical values of right hand sides

  // Line 5  (only present if there are right-hand sides present
  char RHStype[4]; // RHS type {F,M}{' ',G}{' ',X}
  int nRHS;        // number of right-hand sides
  int nRHSind;     // number of right-hand row indices 
                   // (ingnored in unassembled matrices)

  char line[82];

  // Some additional fields not in MatrixFile
  SharedArray<int> RHS_colPtr;        // PTR for sparse right hand sides
  SharedArray<int> RHS_rowIdx;        // IND for sparse right hand sides
  SharedArray<double> RHS_realValues;  // RHS Values, dense, sparse, or elemental.
  SharedArray<double> initialGuess;    // initial guess (always dense)
  SharedArray<double> exactSolution;   // exact solution (always dense)
  
  bool readHeader( FILE * stream );
  bool readData( FILE * stream );


  bool writeHeader( FILE * stream );
  bool writeData( FILE * stream );
  
public:  

  HarwellBoeingFile() {}
  ~HarwellBoeingFile() {}

  bool read( FILE * stream );
  bool read() { return read( fp ); }

  bool write( FILE * stream );
  bool write() { return write( fp ); }

  const char * getMATtype() const { return (const char *) MATtype; }
  const char * getRHStype() const { return (const char *) RHStype; }
  bool setMATtype( const char * type) ;
  bool setRHStype( const char * type) ;
  const char * getTitle() const { return Title; }
  const char * getKey() const { return Key; }
  bool setTitle( const char * new_title);
  bool setKey( const char * new_key );

  const SharedArray<int>& getRHS_colPtr() const { return RHS_colPtr; }
  const SharedArray<int>& getRHS_rowIdx() const { return RHS_rowIdx; } 
  const SharedArray<double>& getRHS_realValues() const { return RHS_realValues; }
  const SharedArray<double>& getInitialGuess() const { return initialGuess; }
  const SharedArray<double>& getExactSolution() const { return exactSolution; }
  
  SharedArray<int>& getRHS_colPtr() { return RHS_colPtr; }
  SharedArray<int>& getRHS_rowIdx() { return RHS_rowIdx; } 
  SharedArray<double>& getRHS_realValues() { return RHS_realValues; }
  SharedArray<double>& getInitialGuess() { return initialGuess; }
  SharedArray<double>& getExactSolution() { return exactSolution; }  
};

SPINDLE_END_NAMESPACE

#endif
