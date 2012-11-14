//
// SpindleArchive.cc 
//
// $Id: SpindleArchive.cc,v 1.2 2000/02/18 01:31:51 kumfert Exp $
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

#include <string>
using std::string;

#include "spindle/SpindleArchive.h"

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifndef SPINDLE_PERSISTANT_H_
#include "spindle/SpindlePersistant.h"
#endif

#ifndef SPINDLE_FILE_H_
#include "spindle/SpindleFile.h"
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
using std::string;
#endif

template < class T >
void byteSwap( T t, int nItems, void* buff ) ;


#ifdef __FUNC__
#undef __FUNC__
#endif

#define __FUNC__ "SpindleArchive::SpindleArchive( SpindleFile* file, unsigned int mode, int buffSize, void* buffer )"
SpindleArchive::SpindleArchive( SpindleFile* file, unsigned int mode, int buffSize, void* buffer ) {
  FENTER;
  file_ = file;
  mode_ = mode;
  bufferSize_ = buffSize;
  
  if ( buffer == 0 ) { 
    bufStart_ = new char[bufferSize_];
    ownBuffer_ = true;
  } else {
    bufStart_ = (char *)buffer;
    ownBuffer_ = false;
  }
  bufMax_ = bufStart_ + bufferSize_ ;
  if ( mode & SpindleArchive::load ) {
    bufCur_ = bufMax_;
  } else if ( mode & SpindleArchive::store ) {
    bufCur_ = bufStart_;
  } else { 
    WARNING("Invalid mode for SpindleArchive Constructor: must be load xor store");
    bufCur_ = 0;
  }
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive::~SpindleArchive()"
SpindleArchive::~SpindleArchive() {
  FENTER;
  if ( file_ != 0 ) {
    if ( isStoring() ) {
      FCALL flush();
    }
    FCALL file_ ->close();
  }
  if (ownBuffer_) {
    delete[] bufStart_;
  }
  FEXIT;
}
#undef __FUNC__

bool
SpindleArchive::isLoading() const { 
  return (( mode_ & 0x01 ) != 0 ); 
}

bool
SpindleArchive::isStoring() const {
  return (( mode_ & 0x01 ) == 0 );
}

bool
SpindleArchive::isByteSwapping() const {
  return (( mode_ & 0x04 ) != 0 );
}

bool
SpindleArchive::isBufferEmpty() const {
  return ( bufCur_ == bufStart_ );
}

SpindleFile*
SpindleArchive::getFile() {
  return file_;
}

#define __FUNC__ "void SpindleArchive::FCALL fillBuffer( int atLeast )"
void
SpindleArchive::fillBuffer( int atLeast ) {
  FENTER;
  if ( bufCur_ == bufStart_ ) { return; }
  if (( atLeast > 0 ) && ( (int)bufferSize_ < atLeast ) ) { 
    ASSERT( true, "Buffer is too small! need to write buffer growing code.");
  }
  if ( bufCur_ != bufMax_ ) { 
    // we have some info to copy to the front.
    int nUnusedBytes = bufMax_ - bufCur_;
    memmove( bufStart_, bufCur_, nUnusedBytes );
    bufCur_ = bufStart_ + nUnusedBytes;
  } else { 
    bufCur_ = bufStart_;
  }
  // now read in the rest
  bufMax_ = bufStart_ + bufferSize_;
  size_t nRead;
  FCALL nRead = file_->read( bufCur_ , 1, (bufMax_ - bufCur_ ) );
  if ( (int)nRead != ( bufMax_ - bufCur_ ) ) {
    // reached the end.  Adjust bufMax_ accordingly.
    bufMax_ = bufCur_ + nRead;
  }
  bufCur_ = bufStart_;
  FEXIT;
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive::flush()"
void 
SpindleArchive::flush() {
  FENTER;
  if (bufCur_ == bufStart_) { FEXIT; }
  size_t bytesWritten = file_->write( bufStart_, sizeof(char), (bufCur_ - bufStart_) );
  if ( (int) bytesWritten != ( bufCur_ - bufStart_) ) {
  //    WARNING("Possible error flushing archive: only wrote %d out of %d bytes",
  //	    bytesWritten, ( bufCur_ - bufStart_));
    ;
  }
  bufCur_ = bufStart_;
  FEXIT;
}
#undef __FUNC__


#define __FUNC__ "void SpindleArchive::close()"
void
SpindleArchive::close() {
  FENTER;
  if ( file_ != 0 ) {
    FCALL flush();   // flush buffer here
    FCALL file_ ->close(); // close the file
    file_ = 0;
  }
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "size_t SpindleArchive::peekLength()"
size_t 
SpindleArchive::peekLength() {
  FENTER;
  size_t temp = 0;
  FCALL this->operator>>( temp );
  if ( temp != 0 ) {
    bufCur_ -= sizeof( size_t );
    FRETURN( temp );
  } else {
    FRETURN( 0 );
  }
}
#undef __FUNC__


#define __FUNC__ "size_t SpindleArchive::read( void* buf, size_t itemSz, size_t nItems )"
size_t
SpindleArchive::read( void* buf, size_t itemSz, size_t nItems ) {
  FENTER;
  char* buffer = (char*) buf;
  int nBytesToRead = itemSz * nItems;
  int nBytesInBuffer = bufMax_ - bufCur_;
  if ( nBytesToRead > nBytesInBuffer ) { 
    memcpy( buffer, bufCur_, nBytesInBuffer );
    buffer += nBytesInBuffer;
    bufCur_ += nBytesInBuffer;
    nBytesToRead -= nBytesInBuffer;
    while( nBytesToRead > (int) bufferSize_ ) {
      FCALL fillBuffer();
      memcpy( buffer, bufCur_, bufferSize_ );
      buffer += bufferSize_;
      bufCur_ += bufferSize_;
      nBytesToRead -= bufferSize_;
    }
    FCALL fillBuffer();
    nBytesInBuffer = bufferSize_;
  }
  if ( nBytesToRead > 0 ) {
    memcpy( buffer, bufCur_, nBytesToRead );
    bufCur_ += nBytesToRead;
  }
  FRETURN ( itemSz * nItems );
}
#undef __FUNC__

#define __FUNC__ "void SpindleArchive::write( const void* buf, size_t itemSz, size_t nItems )"
void
SpindleArchive::write( const void* buf, size_t itemSz, size_t nItems ) {
  FENTER;
  char* buffer = (char*) buf;
  int nBytesToWrite = itemSz * nItems;
  int nBytesInBuffer = bufMax_ - bufCur_;
  if ( nBytesToWrite > nBytesInBuffer ) {
    memcpy( bufCur_, buffer, nBytesInBuffer );
    buffer += nBytesInBuffer;
    bufCur_ += nBytesInBuffer;
    nBytesToWrite -= nBytesInBuffer;
    while( nBytesToWrite > (int)bufferSize_) { 
      FCALL flush();
      memcpy( bufCur_, buffer, bufferSize_ );
      buffer += bufferSize_;
      bufCur_ += bufferSize_;
      nBytesToWrite -= bufferSize_;
    }
    FCALL flush();
    nBytesInBuffer = bufferSize_;
  }
  if ( nBytesToWrite > 0 ) {
    memcpy( bufCur_, buffer, nBytesToWrite );
    bufCur_ += nBytesToWrite;
  }
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "const ClassMetaData* SpindleArchive::readObjectID()"
const ClassMetaData*
SpindleArchive::readObjectID() {
  FENTER;
  string className;
  *this >> className ;
  const ClassMetaData* metaData;
  metaData = SpindleSystem::registry().findClass( className.begin() );
  WARN_IF( metaData == 0, "Cannot find class \"%s\" in registry.", className.begin() );
  FRETURN( metaData );
}
#undef __FUNC__

#define __FUNC__ "SpindleBaseClass* SpindleArchive::readObject( const ClassMetaData* metaData )"
SpindleBaseClass* 
SpindleArchive::readObject( const ClassMetaData* metaData ) {
  FENTER;
  ASSERT( metaData != 0, "Cannot create object from NULL metaData pointer.");
  ASSERT( metaData->pfnCreateObject != 0, "Cannot create class \"%s\" with no constructor in its MetaData.", 
	  metaData->className);
  SpindleBaseClass* object = 0; 
  object = (metaData->pfnCreateObject)();
  ASSERT( object != 0 , "Cannot load object through NULL pointer.");
  object->loadObject( *this );
  FRETURN( object );
}
#undef __FUNC__

#define __FUNC__ "void SpindleArchive::writeObject( const SpindleBaseClass* object )"
void
SpindleArchive::writeObject( const SpindleBaseClass* object ) {
  FENTER;
  const ClassMetaData* metaData;
  metaData = object->getMetaData(); // casting for this->operator<<(className), below
  ASSERT( metaData != 0, "Cannot store and object with a NULL MetaData pointer.");
  ASSERT( metaData->className != 0, "Cannot store and object with a no class name in MetaData.");
  ASSERT( metaData->pfnCreateObject != 0,  "Cannot store and object with a no constructor in MetaData.");
  *this << ( metaData->className );
  object->storeObject( *this );
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& operator<<( SpindleArchive& ar, const SpindleBaseClass* object )"
SpindleArchive& 
#ifdef HAVE_NAMESPACES
  SPINDLE_NAMESPACE::
#endif 
operator<<( SpindleArchive& ar, const SpindleBaseClass* object) {
  FENTER;
  FCALL ar.writeObject( object );
  FRETURN( ar );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& operator>>( SpindleArchive& ar, SpindleBaseClass* &object )"
SpindleArchive& 
#ifdef HAVE_NAMESPACES
  SPINDLE_NAMESPACE::
#endif 
operator>>( SpindleArchive& ar, SpindleBaseClass* &object) {
  FENTER;
  const ClassMetaData * metaData; 
  FCALL metaData = ar.readObjectID();
  
  if ( metaData != 0 ) {
    FCALL object = ar.readObject( metaData );
  } 
  FRETURN( ar );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& operator<<(SpindleArchive& ar, const char* str)"
SpindleArchive& 
#ifdef HAVE_NAMESPACES
  SPINDLE_NAMESPACE::
#endif 
operator<<(SpindleArchive& ar, const char* str) {
  FENTER;
  int len = strlen( str ) + 1;
  FCALL ar << len;
  FCALL ar.write(str, sizeof(char), len );
  FRETURN( ar );
}
#undef __FUNC__

//#ifndef HAVE_NAMESPACES
#define __FUNC__ "SpindleArchive& operator<<(SpindleArchive& ar, const string& str)"
SpindleArchive&
#ifdef HAVE_NAMESPACES
  SPINDLE_NAMESPACE::
#endif 
operator<<(SpindleArchive& ar, const string& str) {
  FENTER;
  FCALL ar << str.c_str();
  FRETURN( ar );
}
#undef __FUNC__
//#endif

#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<(const signed char t)"
SpindleArchive& 
SpindleArchive::operator<<(const signed char t) {
  FENTER;
  if ( ( bufCur_ + sizeof(signed char) ) > bufMax_ ) { 
    FCALL flush(); 
  }
  *(signed char*) bufCur_ = t;
  if ( ! ( mode_ & noByteSwap ) ) { 
    FCALL byteSwap( t, 1, bufCur_ ); 
  } 
  bufCur_ += sizeof(signed char);
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<(const unsigned char t)"
SpindleArchive& 
SpindleArchive::operator<<(const unsigned char t) {
  FENTER;
  if ( ( bufCur_ + sizeof(unsigned char) ) > bufMax_ ) { 
    FCALL flush(); 
  }
  *(unsigned char*) bufCur_ = t;  
  if ( ! ( mode_ & noByteSwap ) ) { 
    FCALL byteSwap( t, 1, bufCur_ ); 
  } 
  bufCur_ += sizeof(unsigned char);
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<(const signed short t)"
SpindleArchive& 
SpindleArchive::operator<<(const signed short t) {
  FENTER;
  if ( ( bufCur_ + sizeof(signed short) ) > bufMax_ ) {
    FCALL flush();
  }
  *(signed short*) bufCur_ = t;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, bufCur_ );
  } 
  bufCur_ += sizeof(signed short);
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<(const unsigned short t)"
SpindleArchive& 
SpindleArchive::operator<<(const unsigned short t) {
  FENTER;
  if ( ( bufCur_ + sizeof(unsigned short) ) > bufMax_ ) {
    FCALL flush();
  }
  *(unsigned short*) bufCur_ = t;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, bufCur_ );
  } 
  bufCur_ += sizeof(unsigned short);
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<(const signed int t)"
SpindleArchive& 
SpindleArchive::operator<<(const signed int t) {
  FENTER;
  if ( ( bufCur_ + sizeof(signed int) ) > bufMax_ ) {
    FCALL flush();
  }
  *(signed int*) bufCur_ = t;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, bufCur_ );
  } 
  bufCur_ += sizeof(signed int);
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<(const unsigned int t)"
SpindleArchive& 
SpindleArchive::operator<<(const unsigned int t) {
  FENTER;
  if ( ( bufCur_ + sizeof(unsigned int) ) > bufMax_ ) {
    FCALL flush();
  }
  *(unsigned int*) bufCur_ = t;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, bufCur_ );
  } 
  bufCur_ += sizeof(unsigned int);
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<(const signed long t)"
SpindleArchive& 
SpindleArchive::operator<<(const signed long t) {
  FENTER;
  if ( ( bufCur_ + sizeof(signed long) ) > bufMax_ ) {
    FCALL flush();
  }
  *(signed long*) bufCur_ = t;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, bufCur_ );
  } 
  bufCur_ += sizeof(signed long);
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<(const unsigned long t)"
SpindleArchive& 
SpindleArchive::operator<<(const unsigned long t) {
  FENTER;
  if ( ( bufCur_ + sizeof(unsigned long) ) > bufMax_ ) {
    FCALL flush();
  }
  *(unsigned long*) bufCur_ = t;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, bufCur_ );
  } 
  bufCur_ += sizeof(unsigned long);
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<(const float t)"
SpindleArchive& 
SpindleArchive::operator<<(const float t) {
  FENTER;
  if ( ( bufCur_ + sizeof(float) ) > bufMax_ ) {
    FCALL flush();
  }
  *(float*) bufCur_ = t;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, bufCur_ );
  } 
  bufCur_ += sizeof(float);
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<(const double t)"
SpindleArchive& 
SpindleArchive::operator<<(const double t) {
  FENTER;
  if ( ( bufCur_ + sizeof(double) ) > bufMax_ ) {
    FCALL flush();
  }
  *(double*) bufCur_ = t;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, bufCur_ );
  } 
  bufCur_ += sizeof(double);
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<(const long double t)"
SpindleArchive& 
SpindleArchive::operator<<(const long double t) {
  FENTER;
  if ( ( bufCur_ + sizeof(long double) ) > bufMax_ ) {
    FCALL flush();
  }
  *(long double*) bufCur_ = t;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, bufCur_ );
  } 
  bufCur_ += sizeof(long double);
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<( const pair<const signed char*, size_t>& p )"
SpindleArchive&
SpindleArchive::operator<<( const pair<const signed char*, size_t>& p ) {
  FENTER;
  // const signed char* start = p.first; // unused 
  size_t nItems = p.second;
  FCALL this->operator<<( nItems );
  const signed char* cur = p.first;
  const signed char* end = p.first + nItems;
  if ( ( bufCur_ + (sizeof(signed char) * nItems) ) > bufMax_ ) {
    // buffer is too small, write buffer sized chunks at a time
    FCALL flush();
    int nItemsPerBuffer = bufferSize_ / sizeof(signed char);
    int nBufferPacks = nItems / nItemsPerBuffer; 
    for( int i=0; i<nBufferPacks; i++ ) {  // for each time we fill the buffer
      char* bufTemp = bufCur_;
      for( int j=0; j<nItemsPerBuffer; j++) { //for each item we insert
	*(signed char*) bufTemp = *cur;
	bufTemp += sizeof(signed char);
	cur++;
      }
      // packed a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, bufCur_ );
      } 
      FCALL flush();
    }
  }
  // write the remainder to the buffer.  we know it fits
  int nRemaining = 0;
  while( cur != end ) {
    *(signed char*) bufCur_ = *cur;
    bufCur_ += sizeof(signed char);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, bufCur_ );
  } 
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<( const pair<const unsigned char*, size_t>& p )"
SpindleArchive&
SpindleArchive::operator<<( const pair<const unsigned char*, size_t>& p ) {
  FENTER;
  //  const unsigned char* start = p.first; // unused 
  size_t nItems = p.second;
  FCALL this->operator<<( nItems );
  const unsigned char* cur = p.first;
  const unsigned char* end = p.first + nItems;
  if ( ( bufCur_ + (sizeof(unsigned char) * nItems) ) > bufMax_ ) {
    // buffer is too small, write buffer sized chunks at a time
    FCALL flush();
    int nItemsPerBuffer = bufferSize_ / sizeof(unsigned char);
    int nBufferPacks = nItems / nItemsPerBuffer; 
    for( int i=0; i<nBufferPacks; i++ ) {  // for each time we fill the buffer
      char* bufTemp = bufCur_;
      for( int j=0; j<nItemsPerBuffer; j++) { //for each item we insert
	*(unsigned char*) bufTemp = *cur;
	bufTemp += sizeof(unsigned char);
	cur++;
      }
      // packed a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, bufCur_ );
      } 
      FCALL flush();
    }
  }
  // write the remainder to the buffer.  we know it fits
  int nRemaining = 0;
  while( cur != end ) {
    *(unsigned char*) bufCur_ = *cur;
    bufCur_ += sizeof(unsigned char);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, bufCur_ );
  } 
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<( const pair<const signed short*, size_t>& p )"
SpindleArchive&
SpindleArchive::operator<<( const pair<const signed short*, size_t>& p ) {
  FENTER;
  // const signed short* start = p.first; // unused 
  size_t nItems = p.second;
  FCALL this->operator<<( nItems );
  const signed short* cur = p.first;
  const signed short* end = p.first + nItems;
  if ( ( bufCur_ + (sizeof(signed short) * nItems) ) > bufMax_ ) {
    // buffer is too small, write buffer sized chunks at a time
    FCALL flush();
    int nItemsPerBuffer = bufferSize_ / sizeof(signed short);
    int nBufferPacks = nItems / nItemsPerBuffer; 
    for( int i=0; i<nBufferPacks; i++ ) {  // for each time we fill the buffer
      char* bufTemp = bufCur_;
      for( int j=0; j<nItemsPerBuffer; j++) { //for each item we insert
	*(signed short*) bufTemp = *cur;
	bufTemp += sizeof(signed short);
	cur++;
      }
      // packed a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, bufCur_ );
      } 
      FCALL flush();
    }
  }
  // write the remainder to the buffer.  we know it fits
  int nRemaining = 0;
  while( cur != end ) {
    *(signed short*) bufCur_ = *cur;
    bufCur_ += sizeof(signed short);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, bufCur_ );
  } 
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<( const pair<const unsigned short*, size_t>& p )"
SpindleArchive&
SpindleArchive::operator<<( const pair<const unsigned short*, size_t>& p ) {
  FENTER;
  // const unsigned short* start = p.first; // unused 
  size_t nItems = p.second;
  FCALL this->operator<<( nItems );
  const unsigned short* cur = p.first;
  const unsigned short* end = p.first + nItems;
  if ( ( bufCur_ + (sizeof(unsigned short) * nItems) ) > bufMax_ ) {
    // buffer is too small, write buffer sized chunks at a time
    FCALL flush();
    int nItemsPerBuffer = bufferSize_ / sizeof(unsigned short);
    int nBufferPacks = nItems / nItemsPerBuffer; 
    for( int i=0; i<nBufferPacks; i++ ) {  // for each time we fill the buffer
      char* bufTemp = bufCur_;
      for( int j=0; j<nItemsPerBuffer; j++) { //for each item we insert
	*(unsigned short*) bufTemp = *cur;
	bufTemp += sizeof(unsigned short);
	cur++;
      }
      // packed a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, bufCur_ );
      } 
      FCALL flush();
    }
  }
  // write the remainder to the buffer.  we know it fits
  int nRemaining = 0;
  while( cur != end ) {
    *(unsigned short*) bufCur_ = *cur;
    bufCur_ += sizeof(unsigned short);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, bufCur_ );
  } 
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<( const pair<const signed int*, size_t>& p )"
SpindleArchive&
SpindleArchive::operator<<( const pair<const signed int*, size_t>& p ) {
  FENTER;
  // const signed int* start = p.first; // unused 
  size_t nItems = p.second;
  FCALL this->operator<<( nItems );
  const signed int* cur = p.first;
  const signed int* end = p.first + nItems;
  if ( ( bufCur_ + (sizeof(signed int) * nItems) ) > bufMax_ ) {
    // buffer is too small, write buffer sized chunks at a time
    FCALL flush();
    int nItemsPerBuffer = bufferSize_ / sizeof(signed int);
    int nBufferPacks = nItems / nItemsPerBuffer; 
    for( int i=0; i<nBufferPacks; i++ ) {  // for each time we fill the buffer
      char* bufTemp = bufCur_;
      for( int j=0; j<nItemsPerBuffer; j++) { //for each item we insert
	*(signed int*) bufTemp = *cur;
	bufTemp += sizeof(signed int);
	cur++;
      }
      // packed a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, bufCur_ );
      } 
      FCALL flush();
    }
  }
  // write the remainder to the buffer.  we know it fits
  int nRemaining = 0;
  while( cur != end ) {
    *(signed int*) bufCur_ = *cur;
    bufCur_ += sizeof(signed int);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, bufCur_ );
  } 
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<( const pair<const unsigned int*, size_t>& p )"
SpindleArchive&
SpindleArchive::operator<<( const pair<const unsigned int*, size_t>& p ) {
  FENTER;
  //  const unsigned int* start = p.first; // unused 
  size_t nItems = p.second;
  FCALL this->operator<<( nItems );
  const unsigned int* cur = p.first;
  const unsigned int* end = p.first + nItems;
  if ( ( bufCur_ + (sizeof(unsigned int) * nItems) ) > bufMax_ ) {
    // buffer is too small, write buffer sized chunks at a time
    FCALL flush();
    int nItemsPerBuffer = bufferSize_ / sizeof(unsigned int);
    int nBufferPacks = nItems / nItemsPerBuffer; 
    for( int i=0; i<nBufferPacks; i++ ) {  // for each time we fill the buffer
      char* bufTemp = bufCur_;
      for( int j=0; j<nItemsPerBuffer; j++) { //for each item we insert
	*(unsigned int*) bufTemp = *cur;
	bufTemp += sizeof(unsigned int);
	cur++;
      }
      // packed a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, bufCur_ );
      } 
      FCALL flush();
    }
  }
  // write the remainder to the buffer.  we know it fits
  int nRemaining = 0;
  while( cur != end ) {
    *(unsigned int*) bufCur_ = *cur;
    bufCur_ += sizeof(unsigned int);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, bufCur_ );
  } 
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<( const pair<const signed long*, size_t>& p )"
SpindleArchive&
SpindleArchive::operator<<( const pair<const signed long*, size_t>& p ) {
  FENTER;
  // const signed long* start = p.first; // unused
  size_t nItems = p.second;
  FCALL this->operator<<( nItems );
  const signed long* cur = p.first;
  const signed long* end = p.first + nItems;
  if ( ( bufCur_ + (sizeof(signed long) * nItems) ) > bufMax_ ) {
    // buffer is too small, write buffer sized chunks at a time
    FCALL flush();
    int nItemsPerBuffer = bufferSize_ / sizeof(signed long);
    int nBufferPacks = nItems / nItemsPerBuffer; 
    for( int i=0; i<nBufferPacks; i++ ) {  // for each time we fill the buffer
      char* bufTemp = bufCur_;
      for( int j=0; j<nItemsPerBuffer; j++) { //for each item we insert
	*(signed long*) bufTemp = *cur;
	bufTemp += sizeof(signed long);
	cur++;
      }
      // packed a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, bufCur_ );
      } 
      FCALL flush();
    }
  }
  // write the remainder to the buffer.  we know it fits
  int nRemaining = 0;
  while( cur != end ) {
    *(signed long*) bufCur_ = *cur;
    bufCur_ += sizeof(signed long);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, bufCur_ );
  } 
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<( const pair<const unsigned long*, size_t>& p )"
SpindleArchive&
SpindleArchive::operator<<( const pair<const unsigned long*, size_t>& p ) {
  FENTER;
  // const unsigned long* start = p.first; // unused 
  size_t nItems = p.second;
  FCALL this->operator<<( nItems );
  const unsigned long* cur = p.first;
  const unsigned long* end = p.first + nItems;
  if ( ( bufCur_ + (sizeof(unsigned long) * nItems) ) > bufMax_ ) {
    // buffer is too small, write buffer sized chunks at a time
    FCALL flush();
    int nItemsPerBuffer = bufferSize_ / sizeof(unsigned long);
    int nBufferPacks = nItems / nItemsPerBuffer; 
    for( int i=0; i<nBufferPacks; i++ ) {  // for each time we fill the buffer
      char* bufTemp = bufCur_;
      for( int j=0; j<nItemsPerBuffer; j++) { //for each item we insert
	*(unsigned long*) bufTemp = *cur;
	bufTemp += sizeof(unsigned long);
	cur++;
      }
      // packed a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, bufCur_ );
      } 
      FCALL flush();
    }
  }
  // write the remainder to the buffer.  we know it fits
  int nRemaining = 0;
  while( cur != end ) {
    *(unsigned long*) bufCur_ = *cur;
    bufCur_ += sizeof(unsigned long);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, bufCur_ );
  } 
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<( const pair<const float*, size_t>& p )"
SpindleArchive&
SpindleArchive::operator<<( const pair<const float*, size_t>& p ) {
  FENTER;
  // const float* start = p.first; // unused 
  size_t nItems = p.second;
  FCALL this->operator<<( nItems );
  const float* cur = p.first;
  const float* end = p.first + nItems;
  if ( ( bufCur_ + (sizeof(float) * nItems) ) > bufMax_ ) {
    // buffer is too small, write buffer sized chunks at a time
    FCALL flush();
    int nItemsPerBuffer = bufferSize_ / sizeof(float);
    int nBufferPacks = nItems / nItemsPerBuffer; 
    for( int i=0; i<nBufferPacks; i++ ) {  // for each time we fill the buffer
      char* bufTemp = bufCur_;
      for( int j=0; j<nItemsPerBuffer; j++) { //for each item we insert
	*(float*) bufTemp = *cur;
	bufTemp += sizeof(float);
	cur++;
      }
      // packed a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, bufCur_ );
      } 
      FCALL flush();
    }
  }
  // write the remainder to the buffer.  we know it fits
  int nRemaining = 0;
  while( cur != end ) {
    *(float*) bufCur_ = *cur;
    bufCur_ += sizeof(float);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, bufCur_ );
  } 
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<( const pair<const double*, size_t>& p )"
SpindleArchive&
SpindleArchive::operator<<( const pair<const double*, size_t>& p ) {
  FENTER;
  // const double* start = p.first; // unused 
  size_t nItems = p.second;
  FCALL this->operator<<( nItems );
  const double* cur = p.first;
  const double* end = p.first + nItems;
  if ( ( bufCur_ + (sizeof(double) * nItems) ) > bufMax_ ) {
    // buffer is too small, write buffer sized chunks at a time
    FCALL flush();
    int nItemsPerBuffer = bufferSize_ / sizeof(double);
    int nBufferPacks = nItems / nItemsPerBuffer; 
    for( int i=0; i<nBufferPacks; i++ ) {  // for each time we fill the buffer
      char* bufTemp = bufCur_;
      for( int j=0; j<nItemsPerBuffer; j++) { //for each item we insert
	*(double*) bufTemp = *cur;
	bufTemp += sizeof(double);
	cur++;
      }
      // packed a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, bufCur_ );
      } 
      FCALL flush();
    }
  }
  // write the remainder to the buffer.  we know it fits
  int nRemaining = 0;
  while( cur != end ) {
    *(double*) bufCur_ = *cur;
    bufCur_ += sizeof(double);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, bufCur_ );
  } 
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator<<( const pair<const long double*, size_t>& p )"
SpindleArchive&
SpindleArchive::operator<<( const pair<const long double*, size_t>& p ) {
  FENTER;
  // const long double* start = p.first; // unused 
  size_t nItems = p.second;
  FCALL this->operator<<(nItems);
  const long double* cur = p.first;
  const long double* end = p.first + nItems;
  if ( ( bufCur_ + (sizeof(long double) * nItems) ) > bufMax_ ) {
    // buffer is too small, write buffer sized chunks at a time
    FCALL flush();
    int nItemsPerBuffer = bufferSize_ / sizeof(long double);
    int nBufferPacks = nItems / nItemsPerBuffer; 
    for( int i=0; i<nBufferPacks; i++ ) {  // for each time we fill the buffer
      char* bufTemp = bufCur_;
      for( int j=0; j<nItemsPerBuffer; j++) { //for each item we insert
	*(long double*) bufTemp = *cur;
	bufTemp += sizeof(long double);
	cur++;
      }
      // packed a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, bufCur_ );
      } 
      FCALL flush();
    }
  }
  // write the remainder to the buffer.  we know it fits
  int nRemaining = 0;
  while( cur != end ) {
    *(long double*) bufCur_ = *cur;
    bufCur_ += sizeof(long double);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, bufCur_ );
  } 
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& operator>>(SpindleArchive& ar, char* &str)"
SpindleArchive& 
#ifdef HAVE_NAMESPACES
  SPINDLE_NAMESPACE::
#endif 
operator>>(SpindleArchive& ar, char* &str) {
  FENTER;
  size_t len;
  FCALL ar >> len;
  if ( str == 0 ) {
    str = new char[len+1];
  }
  FCALL ar.read(str, sizeof(char), len );
  str[len] = '\0';
  FRETURN( ar );
}
#undef __FUNC__
#define __FUNC__ "SpindleArchive& operator>>(SpindleArchive& ar, string& str )"
SpindleArchive&
#ifdef HAVE_NAMESPACES
  SPINDLE_NAMESPACE::
#endif 
operator>>(SpindleArchive& ar, string& str) {
  FENTER;
  size_t len;
  ar >> len;
  if ( len > 0 ) {
    vector<char> v(len);
    FCALL ar.read((void *)v.begin(), sizeof(char), len );
    str.assign(v.begin(),len);
  } else { 
    str.erase();
  }
  FRETURN( ar );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>(signed char& t)"
SpindleArchive& 
SpindleArchive::operator>>(signed char& t) {
  FENTER;
  if ( ( bufCur_ + sizeof(signed char) ) > bufMax_ ) {
    FCALL fillBuffer( sizeof(signed char) - (size_t) (bufMax_ - bufCur_ ) );
  }
  t = *(signed char*) bufCur_;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, (void*)&t );
  }
  bufCur_ += sizeof( signed char );
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>(unsigned char& t)"
SpindleArchive& 
SpindleArchive::operator>>(unsigned char& t) {
  FENTER;
  if ( ( bufCur_ + sizeof(unsigned char) ) > bufMax_ ) {
    FCALL fillBuffer( sizeof(unsigned char) - (size_t) (bufMax_ - bufCur_ ) );
  }
  t = *(unsigned char*) bufCur_;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, (void*)&t );
  }
  bufCur_ += sizeof( unsigned char );
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>(signed short& t)"
SpindleArchive& 
SpindleArchive::operator>>(signed short& t) {
  FENTER;
  if ( ( bufCur_ + sizeof(signed short) ) > bufMax_ ) {
    FCALL fillBuffer( sizeof(signed short) - (size_t) (bufMax_ - bufCur_ ) );
  }
  t = *(signed short*) bufCur_;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, (void*)&t );
  }
  bufCur_ += sizeof( signed short );
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>(unsigned short& t)"
SpindleArchive& 
SpindleArchive::operator>>(unsigned short& t) {
  FENTER;
  if ( ( bufCur_ + sizeof(unsigned short) ) > bufMax_ ) {
    FCALL fillBuffer( sizeof(unsigned short) - (size_t) (bufMax_ - bufCur_ ) );
  }
  t = *(unsigned short*) bufCur_;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, (void*)&t );
  }
  bufCur_ += sizeof( unsigned short );
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>(signed int& t)"
SpindleArchive& 
SpindleArchive::operator>>(signed int& t) {
  FENTER;
  if ( ( bufCur_ + sizeof(signed int) ) > bufMax_ ) {
    FCALL fillBuffer( sizeof(signed int) - (size_t) (bufMax_ - bufCur_ ) );
  }
  t = *(signed int*) bufCur_;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, (void*)&t );
  }
  bufCur_ += sizeof( signed int );
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>(unsigned int& t)"
SpindleArchive& 
SpindleArchive::operator>>(unsigned int& t) {
  FENTER;
  if ( ( bufCur_ + sizeof(unsigned int) ) > bufMax_ ) {
    FCALL fillBuffer( sizeof(unsigned int) - (size_t) (bufMax_ - bufCur_ ) );
  }
  t = *(unsigned int*) bufCur_;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, (void*)&t );
  }
  bufCur_ += sizeof( unsigned int );
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>(signed long& t)"
SpindleArchive& 
SpindleArchive::operator>>(signed long& t) {
  FENTER;
  if ( ( bufCur_ + sizeof(signed long) ) > bufMax_ ) {
    FCALL fillBuffer( sizeof(signed long) - (size_t) (bufMax_ - bufCur_ ) );
  }
  t = *(signed long*) bufCur_;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, (void*)&t );
  }
  bufCur_ += sizeof( signed long );
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>(unsigned long& t)"
SpindleArchive& 
SpindleArchive::operator>>(unsigned long& t) {
  FENTER;
  if ( ( bufCur_ + sizeof(unsigned long) ) > bufMax_ ) {
    FCALL fillBuffer( sizeof(unsigned long) - (size_t) (bufMax_ - bufCur_ ) );
  }
  t = *(unsigned long*) bufCur_;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, (void*)&t );
  }
  bufCur_ += sizeof( unsigned long );
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>(float& t)"
SpindleArchive& 
SpindleArchive::operator>>(float& t) {
  FENTER;
  if ( ( bufCur_ + sizeof(float) ) > bufMax_ ) {
    FCALL fillBuffer( sizeof(float) - (size_t) (bufMax_ - bufCur_ ) );
  }
  t = *(float*) bufCur_;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, (void*)&t );
  }
  bufCur_ += sizeof( float );
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>(double& t)"
SpindleArchive& 
SpindleArchive::operator>>(double& t) {
  FENTER;
  if ( ( bufCur_ + sizeof(double) ) > bufMax_ ) {
    FCALL fillBuffer( sizeof(double) - (size_t) (bufMax_ - bufCur_ ) );
  }
  t = *(double*) bufCur_;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, (void*)&t );
  }
  bufCur_ += sizeof( double );
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>(long double& t)"
SpindleArchive& 
SpindleArchive::operator>>(long double& t) {
  FENTER;
  if ( ( bufCur_ + sizeof(long double) ) > bufMax_ ) {
    FCALL fillBuffer( sizeof(long double) - (size_t) (bufMax_ - bufCur_ ) );
  }
  t = *(long double*) bufCur_;
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( t, 1, (void*)&t );
  }
  bufCur_ += sizeof( long double );
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>( pair<signed char*, size_t>& p )"
SpindleArchive& 
SpindleArchive::operator>>( pair<signed char*, size_t>& p ) {
  FENTER;
  size_t nItems;
  FCALL this->operator>>( nItems );
  p.second = nItems;
  if ( p.first == 0 ) {
    p.first = new signed char[nItems];
  }
  signed char* cur = p.first;
  signed char* end = p.first + nItems;
  signed char* chunkBegin = cur;
  if ( ( bufCur_ + ( sizeof(signed char)*nItems ) ) > bufMax_ ) { 
    // the buffer does not contain enough for the whole array
    FCALL fillBuffer( );
    int nItemsPerBuffer = bufferSize_ / sizeof(signed char); 
    int nBufferPacks = nItems / nItemsPerBuffer;
    for( int i=0; i<nBufferPacks; i++ ) { // for each time we fill the buffer
      chunkBegin = cur;
      for( int j=0; j<nItemsPerBuffer; j++) { // for each item we extract
	*cur = *(signed char*) bufCur_;
	bufCur_ += sizeof(signed char);
	cur++;
      }
      // extracted a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, (void*)chunkBegin );
      }
      FCALL fillBuffer( );
    }
  } // end if
  // read the remainder from the buffer.  we know it fits
  int nRemaining = 0;
  chunkBegin = cur;
  while( cur < end ) {
    *cur = *(signed char*) bufCur_;
    bufCur_ += sizeof(signed char);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, (void*) chunkBegin );
  } 
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>( pair<unsigned char*, size_t>& p )"
SpindleArchive& 
SpindleArchive::operator>>( pair<unsigned char*, size_t>& p ) {
  FENTER;
  size_t nItems;
  FCALL this->operator>>( nItems );
  p.second = nItems;
  if ( p.first == 0 ) {
    p.first = new unsigned char[nItems];
  }
  unsigned char* cur = p.first;
  unsigned char* end = p.first + nItems;
  unsigned char* chunkBegin = cur;
  if ( ( bufCur_ + ( sizeof(unsigned char)*nItems ) ) > bufMax_ ) { 
    // the buffer does not contain enough for the whole array
    FCALL fillBuffer( );
    int nItemsPerBuffer = bufferSize_ / sizeof(unsigned char); 
    int nBufferPacks = nItems / nItemsPerBuffer;
    for( int i=0; i<nBufferPacks; i++ ) { // for each time we fill the buffer
      chunkBegin = cur;
      for( int j=0; j<nItemsPerBuffer; j++) { // for each item we extract
	*cur = *(unsigned char*) bufCur_;
	bufCur_ += sizeof(unsigned char);
	cur++;
      }
      // extracted a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, (void*)chunkBegin );
      }
      FCALL fillBuffer( );
    }
  } // end if
  // read the remainder from the buffer.  we know it fits
  int nRemaining = 0;
  chunkBegin = cur;
  while( cur < end ) {
    *cur = *(unsigned char*) bufCur_;
    bufCur_ += sizeof(unsigned char);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, (void*) chunkBegin );
  } 
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>( pair<signed short*, size_t>& p )"
SpindleArchive& 
SpindleArchive::operator>>( pair<signed short*, size_t>& p ) {
  FENTER;
  size_t nItems;
  FCALL this->operator>>( nItems );
  p.second = nItems;
  if ( p.first == 0 ) {
    p.first = new signed short[nItems];
  }
  signed short* cur = p.first;
  signed short* end = p.first + nItems;
  signed short* chunkBegin = cur;
  if ( ( bufCur_ + ( sizeof(signed short)*nItems ) ) > bufMax_ ) { 
    // the buffer does not contain enough for the whole array
    FCALL fillBuffer( );
    int nItemsPerBuffer = bufferSize_ / sizeof(signed short); 
    int nBufferPacks = nItems / nItemsPerBuffer;
    for( int i=0; i<nBufferPacks; i++ ) { // for each time we fill the buffer
      chunkBegin = cur;
      for( int j=0; j<nItemsPerBuffer; j++) { // for each item we extract
	*cur = *(signed short*) bufCur_;
	bufCur_ += sizeof(signed short);
	cur++;
      }
      // extracted a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, (void*)chunkBegin );
      }
      FCALL fillBuffer( );
    }
  } // end if
  // read the remainder from the buffer.  we know it fits
  int nRemaining = 0;
  chunkBegin = cur;
  while( cur < end ) {
    *cur = *(signed short*) bufCur_;
    bufCur_ += sizeof(signed short);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, (void*) chunkBegin );
  } 
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>( pair<unsigned short*, size_t>& p )"
SpindleArchive& 
SpindleArchive::operator>>( pair<unsigned short*, size_t>& p ) {
  FENTER;
  size_t nItems;
  FCALL this->operator>>( nItems );
  p.second = nItems;
  if ( p.first == 0 ) {
    p.first = new unsigned short[nItems];
  }
  unsigned short* cur = p.first;
  unsigned short* end = p.first + nItems;
  unsigned short* chunkBegin = cur;
  if ( ( bufCur_ + ( sizeof(unsigned short)*nItems ) ) > bufMax_ ) { 
    // the buffer does not contain enough for the whole array
    FCALL fillBuffer( );
    int nItemsPerBuffer = bufferSize_ / sizeof(unsigned short); 
    int nBufferPacks = nItems / nItemsPerBuffer;
    for( int i=0; i<nBufferPacks; i++ ) { // for each time we fill the buffer
      chunkBegin = cur;
      for( int j=0; j<nItemsPerBuffer; j++) { // for each item we extract
	*cur = *(unsigned short*) bufCur_;
	bufCur_ += sizeof(unsigned short);
	cur++;
      }
      // extracted a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, (void*)chunkBegin );
      }
      FCALL fillBuffer( );
    }
  } // end if
  // read the remainder from the buffer.  we know it fits
  int nRemaining = 0;
  chunkBegin = cur;
  while( cur < end ) {
    *cur = *(unsigned short*) bufCur_;
    bufCur_ += sizeof(unsigned short);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, (void*) chunkBegin );
  } 
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>( pair<signed int*, size_t>& p )"
SpindleArchive& 
SpindleArchive::operator>>( pair<signed int*, size_t>& p ) {
  FENTER;
  size_t nItems;
  FCALL this->operator>>( nItems );
  p.second = nItems;
  if ( p.first == 0 ) {
    p.first = new signed int[nItems];
  }
  signed int* cur = p.first;
  signed int* end = p.first + nItems;
  signed int* chunkBegin = cur;
  if ( ( bufCur_ + ( sizeof(signed int)*nItems ) ) > bufMax_ ) { 
    // the buffer does not contain enough for the whole array
    FCALL fillBuffer( );
    int nItemsPerBuffer = bufferSize_ / sizeof(signed int); 
    int nBufferPacks = nItems / nItemsPerBuffer;
    for( int i=0; i<nBufferPacks; i++ ) { // for each time we fill the buffer
      chunkBegin = cur;
      for( int j=0; j<nItemsPerBuffer; j++) { // for each item we extract
	*cur = *(signed int*) bufCur_;
	bufCur_ += sizeof(signed int);
	cur++;
      }
      // extracted a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, (void*)chunkBegin );
      }
      FCALL fillBuffer( );
    }
  } // end if
  // read the remainder from the buffer.  we know it fits
  int nRemaining = 0;
  chunkBegin = cur;
  while( cur < end ) {
    *cur = *(signed int*) bufCur_;
    bufCur_ += sizeof(signed int);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, (void*) chunkBegin );
  } 
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>( pair<unsigned int*, size_t>& p )"
SpindleArchive& 
SpindleArchive::operator>>( pair<unsigned int*, size_t>& p ) {
  FENTER;
  size_t nItems;
  FCALL this->operator>>( nItems );
  p.second = nItems;
  if ( p.first == 0 ) {
    p.first = new unsigned int[nItems];
  }
  unsigned int* cur = p.first;
  unsigned int* end = p.first + nItems;
  unsigned int* chunkBegin = cur;
  if ( ( bufCur_ + ( sizeof(unsigned int)*nItems ) ) > bufMax_ ) { 
    // the buffer does not contain enough for the whole array
    FCALL fillBuffer( );
    int nItemsPerBuffer = bufferSize_ / sizeof(unsigned int); 
    int nBufferPacks = nItems / nItemsPerBuffer;
    for( int i=0; i<nBufferPacks; i++ ) { // for each time we fill the buffer
      chunkBegin = cur;
      for( int j=0; j<nItemsPerBuffer; j++) { // for each item we extract
	*cur = *(unsigned int*) bufCur_;
	bufCur_ += sizeof(unsigned int);
	cur++;
      }
      // extracted a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, (void*)chunkBegin );
      }
      FCALL fillBuffer( );
    }
  } // end if
  // read the remainder from the buffer.  we know it fits
  int nRemaining = 0;
  chunkBegin = cur;
  while( cur < end ) {
    *cur = *(unsigned int*) bufCur_;
    bufCur_ += sizeof(unsigned int);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, (void*) chunkBegin );
  } 
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>( pair<signed long*, size_t>& p )"
SpindleArchive& 
SpindleArchive::operator>>( pair<signed long*, size_t>& p ) {
  FENTER;
  size_t nItems;
  FCALL this->operator>>( nItems );
  p.second = nItems;
  if ( p.first == 0 ) {
    p.first = new signed long[nItems];
  }
  signed long* cur = p.first;
  signed long* end = p.first + nItems;
  signed long* chunkBegin = cur;
  if ( ( bufCur_ + ( sizeof(signed long)*nItems ) ) > bufMax_ ) { 
    // the buffer does not contain enough for the whole array
    FCALL fillBuffer( );
    int nItemsPerBuffer = bufferSize_ / sizeof(signed long); 
    int nBufferPacks = nItems / nItemsPerBuffer;
    for( int i=0; i<nBufferPacks; i++ ) { // for each time we fill the buffer
      chunkBegin = cur;
      for( int j=0; j<nItemsPerBuffer; j++) { // for each item we extract
	*cur = *(signed long*) bufCur_;
	bufCur_ += sizeof(signed long);
	cur++;
      }
      // extracted a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, (void*)chunkBegin );
      }
      FCALL fillBuffer( );
    }
  } // end if
  // read the remainder from the buffer.  we know it fits
  int nRemaining = 0;
  chunkBegin = cur;
  while( cur < end ) {
    *cur = *(signed long*) bufCur_;
    bufCur_ += sizeof(signed long);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, (void*) chunkBegin );
  } 
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>( pair<unsigned long*, size_t>& p )"
SpindleArchive& 
SpindleArchive::operator>>( pair<unsigned long*, size_t>& p ) {
  FENTER;
  size_t nItems;
  FCALL this->operator>>( nItems );
  p.second = nItems;
  if ( p.first == 0 ) {
    p.first = new unsigned long[nItems];
  }
  unsigned long* cur = p.first;
  unsigned long* end = p.first + nItems;
  unsigned long* chunkBegin = cur;
  if ( ( bufCur_ + ( sizeof(unsigned long)*nItems ) ) > bufMax_ ) { 
    // the buffer does not contain enough for the whole array
    FCALL fillBuffer( );
    int nItemsPerBuffer = bufferSize_ / sizeof(unsigned long); 
    int nBufferPacks = nItems / nItemsPerBuffer;
    for( int i=0; i<nBufferPacks; i++ ) { // for each time we fill the buffer
      chunkBegin = cur;
      for( int j=0; j<nItemsPerBuffer; j++) { // for each item we extract
	*cur = *(unsigned long*) bufCur_;
	bufCur_ += sizeof(unsigned long);
	cur++;
      }
      // extracted a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, (void*)chunkBegin );
      }
      FCALL fillBuffer( );
    }
  } // end if
  // read the remainder from the buffer.  we know it fits
  int nRemaining = 0;
  chunkBegin = cur;
  while( cur < end ) {
    *cur = *(unsigned long*) bufCur_;
    bufCur_ += sizeof(unsigned long);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, (void*) chunkBegin );
  } 
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>( pair<float*, size_t>& p )"
SpindleArchive& 
SpindleArchive::operator>>( pair<float*, size_t>& p ) {
  FENTER;
  size_t nItems;
  FCALL this->operator>>( nItems );
  p.second = nItems;
  if ( p.first == 0 ) {
    p.first = new float[nItems];
  }
  float* cur = p.first;
  float* end = p.first + nItems;
  float* chunkBegin = cur;
  if ( ( bufCur_ + ( sizeof(float)*nItems ) ) > bufMax_ ) { 
    // the buffer does not contain enough for the whole array
    FCALL fillBuffer( );
    int nItemsPerBuffer = bufferSize_ / sizeof(float); 
    int nBufferPacks = nItems / nItemsPerBuffer;
    for( int i=0; i<nBufferPacks; i++ ) { // for each time we fill the buffer
      chunkBegin = cur;
      for( int j=0; j<nItemsPerBuffer; j++) { // for each item we extract
	*cur = *(float*) bufCur_;
	bufCur_ += sizeof(float);
	cur++;
      }
      // extracted a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, (void*)chunkBegin );
      }
      FCALL fillBuffer( );
    }
  } // end if
  // read the remainder from the buffer.  we know it fits
  int nRemaining = 0;
  chunkBegin = cur;
  while( cur < end ) {
    *cur = *(float*) bufCur_;
    bufCur_ += sizeof(float);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, (void*) chunkBegin );
  } 
  FRETURN( *this );
}
#undef __FUNC__

#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>( pair<double*, size_t>& p )"
SpindleArchive& 
SpindleArchive::operator>>( pair<double*, size_t>& p ) {
  FENTER;
  size_t nItems;
  FCALL this->operator>>( nItems );
  p.second = nItems;
  if ( p.first == 0 ) {
    p.first = new double[nItems];
  }
  double* cur = p.first;
  double* end = p.first + nItems;
  double* chunkBegin = cur;
  if ( ( bufCur_ + ( sizeof(double)*nItems ) ) > bufMax_ ) { 
    // the buffer does not contain enough for the whole array
    FCALL fillBuffer( );
    int nItemsPerBuffer = bufferSize_ / sizeof(double); 
    int nBufferPacks = nItems / nItemsPerBuffer;
    for( int i=0; i<nBufferPacks; i++ ) { // for each time we fill the buffer
      chunkBegin = cur;
      for( int j=0; j<nItemsPerBuffer; j++) { // for each item we extract
	*cur = *(double*) bufCur_;
	bufCur_ += sizeof(double);
	cur++;
      }
      // extracted a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, (void*)chunkBegin );
      }
      FCALL fillBuffer( );
    }
  } // end if
  // read the remainder from the buffer.  we know it fits
  int nRemaining = 0;
  chunkBegin = cur;
  while( cur < end ) {
    *cur = *(double*) bufCur_;
    bufCur_ += sizeof(double);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, (void*) chunkBegin );
  } 
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "SpindleArchive& SpindleArchive::operator>>( pair<long double*, size_t>& p )"
SpindleArchive& 
SpindleArchive::operator>>( pair<long double*, size_t>& p ) {
  FENTER;
  size_t nItems;
  FCALL this->operator>>( nItems );
  p.second = nItems;
  if ( p.first == 0 ) {
    p.first = new long double[nItems];
  }
  long double* cur = p.first;
  long double* end = p.first + nItems;
  long double* chunkBegin = cur;
  if ( ( bufCur_ + ( sizeof(long double)*nItems ) ) > bufMax_ ) { 
    // the buffer does not contain enough for the whole array
    FCALL fillBuffer( );
    int nItemsPerBuffer = bufferSize_ / sizeof(long double); 
    int nBufferPacks = nItems / nItemsPerBuffer;
    for( int i=0; i<nBufferPacks; i++ ) { // for each time we fill the buffer
      chunkBegin = cur;
      for( int j=0; j<nItemsPerBuffer; j++) { // for each item we extract
	*cur = *(long double*) bufCur_;
	bufCur_ += sizeof(long double);
	cur++;
      }
      // extracted a whole buffer
      if ( ! ( mode_ & noByteSwap ) ) {
	FCALL byteSwap( *cur, nItemsPerBuffer, (void*)chunkBegin );
      }
      FCALL fillBuffer( );
    }
  } // end if
  // read the remainder from the buffer.  we know it fits
  int nRemaining = 0;
  chunkBegin = cur;
  while( cur < end ) {
    *cur = *(long double*) bufCur_;
    bufCur_ += sizeof(long double);
    cur++;
    nRemaining++;
  }
  if ( ! ( mode_ & noByteSwap ) ) {
    FCALL byteSwap( *cur, nRemaining, (void*) chunkBegin );
  } 
  FRETURN( *this );
}
#undef __FUNC__


#define __FUNC__ "void byteSwap<T>( T t, int nItems, void* buff )"
template < class T >
void byteSwap( T t, int nItems, void* buff ) {
  FENTER;
  char* buffer = (char*) buff;
  T temp;
  T* ptemp = &temp;
  char *p2 = (char *) &temp;
  int size_T = sizeof(T);
  (void) t;
  
  for( int j=0; j<nItems; j++ ) {
    char *p1 = (char*) ( buffer + j );
    for( int i=0; i<size_T; i++ ) {
      p2[i] = p1[ size_T - 1 - i ];
    }
    (T &)(buffer[j]) = *ptemp;
  }
  FEXIT;
}
#undef __FUNC__


