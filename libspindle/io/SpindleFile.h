//
// SpindleFile.h -- base class for all Spindle File types
//
// $Id: SpindleFile.h,v 1.2 2000/02/18 01:31:52 kumfert Exp $
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

#ifndef SPINDLE_FILE_H_
#define SPINDLE_FILE_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifndef SPINDLE_BASE_CLASS_H_
#include "spindle/SpindleBaseClass.h"
#endif

SPINDLE_BEGIN_NAMESPACE

/**
 * @memo Base class for all file types
 * @type class
 *
 * This class encapsulates a FILE pointer.  There can be
 * other classes to encapsulate sockets, MPI connections, 
 * possibly memory mapped files, etc.   
 *
 * @author Gary Kumfert
 * @version #$Id: SpindleFile.h,v 1.2 2000/02/18 01:31:52 kumfert Exp $#
 * @see SpindleArchive
 */
class SpindleFile : public SpindleBaseClass { 
public:
  /// defines how different ways of seeking though a file.
  typedef enum { FILE_START, FILE_CUR, FILE_END } SeekOption;


  /**@name constructors/destructors */
  //@{
  /// default
  SpindleFile();
  /// create from a standard FILE pointer.
  SpindleFile( FILE * fp );
  /// create from a UNIX file descriptor
  SpindleFile( int filedes, const char * openFlags );
  /// open the file based on its names.
  SpindleFile( const char * filename, const char * openFlags );
  /// destructor
  virtual ~SpindleFile();
  //@}

  /**@name Basic File operations */
  //@{
  /// return true iff at the end of the file
  virtual bool isEOF();
  /// return true iff there is an error detected
  virtual bool isError();
  /// open a file based on the UNIX file descriptor
  virtual bool open( int filedes, const char * openFlags );
  /// open a file based on the file name
  virtual bool open( const char * filename, const char * openFlags );
  /// flush the file buffer
  virtual void flush();
  /// close the file.
  virtual void close();
  /** seek in the file
   * @param offset in bytes
   * @param from must be one of #FILE_START, FILE_CUR,# or #FILE_END#
   */
  virtual bool seek( long offset, SpindleFile::SeekOption from );
  /// equivalent to #seek( 0, SpindleFile::FILE_START )#
  virtual void rewind();
  /// read a certain number of bytes into the buffer.
  virtual size_t read( void* buffer, size_t nBytes, size_t nObjs );
  /// write a certain number of bytes from the buffer.
  virtual size_t write( const void* buffer, size_t nBytes, size_t nObjs );
  //@}

  /**@name  Static Manipulators */
  //@{
  /// Rename the file.
  static void rename( const char* oldFilename, const char* newFilename );
  /// Remove the file.
  static void remove( const char* filename );
  //@}


private:
  const char * filename_;
  FILE * fp_;
  enum { dont, file, pipe } closeOnDelete_;

  SPINDLE_DECLARE_DYNAMIC( SpindleFile )

};

SPINDLE_END_NAMESPACE

#endif
