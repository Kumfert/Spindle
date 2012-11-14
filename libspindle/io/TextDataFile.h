//
// TextDataFile.h
//
//  $Id: TextDataFile.h,v 1.2 2000/02/18 01:31:52 kumfert Exp $
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
// =========================================================================
//
//

#ifndef SPINDLE_TEXT_DATA_FILE_H_
#define SPINDLE_TEXT_DATA_FILE_H_

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include <stdio.h>
#include <string.h>
#else
#include <cstdio>
#include <cstring>
#endif

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

SPINDLE_BEGIN_NAMESPACE

/**
 * @memo Handles compressed or uncompressed files.
 * @type class
 *
 * This class hides details about where the #FILE# pointer
 * came from.  It could be that the user just specified
 * a file name, or the user could have specified a
 * command with a pipe #|# symbol.  Or the user could 
 * have specified a compressed (gzipped, etc) file that
 * this class uncompresses and opens a pipe to.
 * 
 * @author Gary Kumfert
 * @version #$Id: TextDataFile.h,v 1.2 2000/02/18 01:31:52 kumfert Exp $#
 */
class TextDataFile { 
private:
  enum {pipe, file} filetype;

  bool openRead( const char* string, const char* mode );
  bool openWrite( const char* string, const char* mode );

  void checkERRNO();

protected:
  FILE * fp;
  bool m_isBinary;
  bool m_isFormatted;
  /// protected default constructor
  TextDataFile() : fp(0), m_isBinary(false), m_isFormatted(false) {}

public:
  /// destructor
  ~TextDataFile();

  /// returns the FILE* 
  FILE* getFilePointer();
  /// opens a file, pipe, or compressed file depending on string
  bool open( const char* string, const char* mode );
  /// closes the file.
  bool close();
};

inline FILE* 
TextDataFile::getFilePointer() {
  return fp;
}

SPINDLE_END_NAMESPACE

#endif
