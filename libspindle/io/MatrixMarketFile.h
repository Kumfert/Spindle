//
// MatrixMarketFile.h -- I/O routines for creating a matrix from a MatrixMarket File
//
// $Id: MatrixMarketFile.h,v 1.2 2000/02/18 01:31:51 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1999, Old Dominion University.  All rights reserved.
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
// see http://www.nist.gov/MatrixMarket for details
//

#ifndef SPINDLE_MATRIX_MARKET_FILE_H_
#define SPINDLE_MATRIX_MARKET_FILE_H_

#ifndef SPINDLE_H_
#include SPINDLE_H_
#endif

#ifndef SPINDLE_TEXT_DATA_FILE_H_
#include "spindle/TextDataFile.h"
#endif

#ifndef SPINDLE_MATRIX_BASE_H_
#include "spindle/MatrixBase.h"
#endif

SPINDLE_BEGIN_NAMESPACE

class MatrixMarketFile : public MatrixBase, public TextDataFile {
public:
  bool standard_header;
  string comments;
  enum { UNKNOWN_FORMAT, COORDINATE, ARRAY } format_qualifier;
  enum { UNKNOWN_FIELD, PATTERN, INTEGER, REAL, COMPLEX } field_qualifier;
  enum { UNKNOWN_SYMMETRY, GENERAL, SYMMETRIC, SKEW_SYMMETRIC, HERMITIAN } symmetry_qualifier;

protected:
  string read_banner(FILE * stream); 
  // reads the specifications, if any
  // returns first line not beginning with "%"

public:
  MatrixMarketFile() : standard_header(0), comments(""), 
    format_qualifier(UNKNOWN_FORMAT), 
    field_qualifier( UNKNOWN_FIELD), 
    symmetry_qualifier(UNKNOWN_SYMMETRY){}
  virtual ~MatrixMarketFile() {}
  virtual bool read( FILE * stream );
  virtual bool write( FILE * stream );

  bool read() { return read( fp ); }
  bool write() { return write( fp ); }
};

SPINDLE_END_NAMESPACE

#endif // not definded( SPINDLE_MATRIX_MARKET_FILE_H_ )
