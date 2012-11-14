//
//  SpindleArchive.h
//
// $Id: SpindleArchive.h,v 1.2 2000/02/18 01:31:51 kumfert Exp $
//
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
//  This class is used by all persistant objects to store their data
//

#ifndef SPINDLE_ARCHIVE_H_
#define SPINDLE_ARCHIVE_H_


#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include "pair.h"
#else
#include <utility>
using std::pair;
#endif

// This is a strictly C++ class 
#include <string>
using std::string;

SPINDLE_BEGIN_NAMESPACE

struct ClassMetaData;
class SpindleBaseClass;
class SpindleFile;

/**
 * @memo Used by all persistant objects to store/load their data
 * @type class
 *
 * @author Gary Kumfert
 * @version  #$Id: SpindleArchive.h,v 1.2 2000/02/18 01:31:51 kumfert Exp $#
 * @see SpindleFile
 * @see SpindlePersistant
 */
class SpindleArchive { 
public:
  /// different modes for using an archive object
  enum Mode { store=0x01, 
	      load =0x02,
	      noFlushOnDelete =0x04,
	      noByteSwap = 0x08
            };
  /// constructor
  SpindleArchive( SpindleFile* fp, unsigned int mode, int buffSize=4096, void* buffer=NULL );
  /// destructor
  ~SpindleArchive();

  /**@name simple tests */
  //@{
  /// returns true iff the archive is for loading new instances.
  bool isLoading() const;
  /// returns true iff the archive is storing existing instances.
  bool isStoring() const;
  /// returns true iff the archive is swapping bytes for cross-platform compatability
  bool isByteSwapping() const;
  /// returns true iff the buffer is empty
  bool isBufferEmpty() const;
  //@}

  /// returns the SpindleFile instance that the SpindleArchive is using.
  SpindleFile* getFile();

  void fillBuffer( int atLeast=-1 );
  void flush();
  void close();

  size_t peekLength();
  size_t read( void* buf, size_t itemSz, size_t nItems );
  void write(const void* buf, size_t itemSz, size_t nItems );

  const ClassMetaData* readObjectID();
  SpindleBaseClass* readObject( const ClassMetaData* metaData );
  void writeObject( const SpindleBaseClass* object );

  /**@name insertion */
  //@{
  /**@name friend insertion operators */
  //@{
  /// stores a zero terminated character array
  friend SpindleArchive& operator<<( SpindleArchive& ar, const char* str );
  /// stores a C++ string
  friend SpindleArchive& operator<<( SpindleArchive& ar, const string& str );
  /// stores a class derived from SpindleBaseClass
  friend SpindleArchive& operator<<( SpindleArchive& ar, const SpindleBaseClass* object );
  //@}
  /**@name single value insertion*/
  //@{
  /// stores a char
  SpindleArchive& operator<<( const signed char t);
  /// stores an unsigned char
  SpindleArchive& operator<<( const unsigned char t);
  /// stores a short
  SpindleArchive& operator<<( const signed short t);
  /// stores an unsigned short
  SpindleArchive& operator<<( const unsigned short t);
  /// stores an int
  SpindleArchive& operator<<( const signed int t);
  /// stores an unsigned int
  SpindleArchive& operator<<( const unsigned int t);
  /// stores a long
  SpindleArchive& operator<<( const signed long t);
  /// stores an unsigned long
  SpindleArchive& operator<<( const unsigned long t);
  /// stores a float
  SpindleArchive& operator<<( const float t);
  /// stores a double
  SpindleArchive& operator<<( const double t);
  /// stores a long double
  SpindleArchive& operator<<( const long double t);
  //@}
  /**@name array insertion */
  //@{
  /// stores and array of char's
  SpindleArchive& operator<<( const pair<const signed char*, size_t>& p);
  /// stores and array of unsigned char's
  SpindleArchive& operator<<( const pair<const unsigned char*, size_t>& p);
  /// stores and array of short's
  SpindleArchive& operator<<( const pair<const signed short*, size_t>& p);
  /// stores and array of unsigned short's
  SpindleArchive& operator<<( const pair<const unsigned short*, size_t>& p);
  /// stores and array of int's
  SpindleArchive& operator<<( const pair<const signed int*, size_t>& p);
  /// stores and array of unsigned int's
  SpindleArchive& operator<<( const pair<const unsigned int*, size_t>& p);
  /// stores and array of long's
  SpindleArchive& operator<<( const pair<const signed long*, size_t>& p);
  /// stores and array of unsigned long's
  SpindleArchive& operator<<( const pair<const unsigned long*, size_t>& p);
  /// stores and array of floats
  SpindleArchive& operator<<( const pair<const float*, size_t>& p);
  /// stores and array of doubles
  SpindleArchive& operator<<( const pair<const double*, size_t>& p);
  /// stores and array of long doubles
  SpindleArchive& operator<<( const pair<const long double*, size_t>& p);
  //@}
  //@}
  
  /**@name extraction */
  //@{
  /**@name friend extraction operators */
  //@{
  /// loads a zero terminated character string
  friend SpindleArchive& operator>>( SpindleArchive& ar, char* &str );
  /// loads a C++ string
  friend SpindleArchive& operator>>( SpindleArchive& ar, string& str );
  /// loads an instance of a class derived from SpindleBaseClass
  friend SpindleArchive& operator>>( SpindleArchive& ar, SpindleBaseClass* &object );
  //@}
  /**@name single value extraction */
  //@{
  /// loads a char
  SpindleArchive& operator>>( signed char& );
  /// loads an unsigned char
  SpindleArchive& operator>>( unsigned char& );
  /// loads a short
  SpindleArchive& operator>>( signed short& );
  /// loads an unsigned short
  SpindleArchive& operator>>( unsigned short& );
  /// loads an int
  SpindleArchive& operator>>( signed int& );
  /// loads an unsigned int
  SpindleArchive& operator>>( unsigned int& );
  /// loads a long
  SpindleArchive& operator>>( signed long& );
  /// loads an unsigned long
  SpindleArchive& operator>>( unsigned long& );
  /// loads a float
  SpindleArchive& operator>>( float& );
  /// loads a double
  SpindleArchive& operator>>( double& );
  /// loads a long double
  SpindleArchive& operator>>( long double& );
  //@}
  /**@name  array extraction */
  //@{
  /// loads an array of char's
  SpindleArchive& operator>>( pair< signed char*, size_t>& );
  /// loads an array of unsigned char's
  SpindleArchive& operator>>( pair< unsigned char*, size_t>& );
  /// loads an array of short's
  SpindleArchive& operator>>( pair< signed short*, size_t>& );
  /// loads an array of unsigned short's
  SpindleArchive& operator>>( pair< unsigned short*, size_t>& );
  /// loads an array of int's
  SpindleArchive& operator>>( pair< signed int*, size_t>& );
  /// loads an array of unsigned int's
  SpindleArchive& operator>>( pair< unsigned int*, size_t>& );
  /// loads an array of long's
  SpindleArchive& operator>>( pair< signed long*, size_t>& );
  /// loads an array of unsigned long's
  SpindleArchive& operator>>( pair< unsigned long*, size_t>& );
  /// loads an array of float's
  SpindleArchive& operator>>( pair< float*, size_t>& );
  /// loads an array of double's
  SpindleArchive& operator>>( pair< double*, size_t>& );
  /// loads an array of long double's
  SpindleArchive& operator>>( pair< long double*, size_t>& );
  //@}
  //@}
protected:
  /// copy constructor
  SpindleArchive( const SpindleArchive& src );
  /// assignment
  void operator=( const SpindleArchive& src );

  unsigned int mode_;
  size_t bufferSize_;
  SpindleFile* file_;
  
  char* bufCur_;
  char* bufMax_;
  char* bufStart_;
  bool ownBuffer_;

};

SPINDLE_END_NAMESPACE

#endif
