//
// SharedArray.h  -- An array version of a smart pointer.
//
//  $Id: SharedArray.h,v 1.2 2000/02/18 01:31:56 kumfert Exp $
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
//  SharedArray<T>'s have a concept of ``ownership''.  Each object pointed
//  to by SharedArray<T>'s are owned by only one SharedArray<T>.  This one 
//  ``owner'' is allowed read, write, and delete priveledges, all other
//  SharedArray<T>'s that point to that object, only have read priviledges.
//
//  This ownership and bounds checking is strictly enforced, unless the 
//  symbol NDEBUG is #define'd, in which case the checking is relaxed.
//
//  Access is granted by borrow() and lend().  Ownership is granted
//  by take() and give().  In the real world, access and ownership is
//  clear in these cases.  The import() and export() pair is for making
//  private copies of data and releasingthe rest.
//

#ifndef AUTO_ARRAY_H_
#define AUTO_ARRAY_H_

#define export exportIt
// export is now a C++ keyword???

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

SPINDLE_BEGIN_NAMESPACE

template<class T>
class SharedArray { 
private:
  T *array;
  int sz;
  int maxSize;  // maxSize > 0 implies I own array, maxSize < 0 implies not.
  void makePrivateCopy();

public:
  SharedArray();
  SharedArray( T* p, const int length );  // constructor
  SharedArray( const T* p, const int length );  // constructor
  explicit SharedArray( const int length);             // constructor;
  ~SharedArray();                                          // destructor

  // Accessors
  bool operator!() const; // test for null pointer
  bool isNull() const;
  bool notNull() const;
  int size() const;       // return length of array
  const T& operator[](int i) const;
  T& operator[](int i);

  // Random Access Iterators
  const T* begin() const;
  const T* end() const;
  T* begin();
  T* end();

  // Manipulators
  const T* lend() const;
  void borrow( const T* p, const int length );
  void borrow( const SharedArray<T>& src );
  T* export() const;
  void import( const T* p, const int length );
  void import( const SharedArray<T> &src );
  T* give();
  void take( T* p, const int length );
  void take( SharedArray<T>& src );
  bool giveBack( SharedArray<T>& src );
  bool takeBack( T* p, const int length );

  bool isMine() const;
  bool notMine() const;

  void reset();
  void resize(const int i=0);

  bool init( const T& t );
  bool init( const T& base, const T& increment );
  void swap (SharedArray<T>& other);
  // friend void swap<class T>( SharedArray<T>& first, SharedArray<T>& second );
private:
  typedef alloc Alloc;
  typedef simple_alloc< T, Alloc > T_alloc;

  // private copy and assignment constructors
  SharedArray<T>& operator=( const SharedArray<T>& ) { return *this; }
  void free();
};
#ifdef __FUNC__
#undef __FUNC__
#endif

#define __FUNC__ "void SharedArray<T>::makePrivateCopy()"
template < class T >
inline void SharedArray<T>::makePrivateCopy() {
  FENTER;
  if ( maxSize > 0 ) { FEXIT; } // already own array
  const T* src = array;
  T* dest = T_alloc::allocate( sz );
  for( int i=0; i<sz; i++) { *dest++ = *src++; }
  maxSize = sz; // ownArray == true
  FEXIT;
}
#undef __FUNC__

template < class T >
inline SharedArray<T>::SharedArray()
  : array(0), sz(0), maxSize( -1 )
{ }

template < class T >
inline SharedArray<T>::SharedArray( T* p, const int length )
  : array(p), sz(length), maxSize( length )
{ }


template < class T >
inline SharedArray<T>::SharedArray( const T* p, const int length )
  : array(0), sz(length), maxSize( -1 ) {
    if ( p != 0 ) {
#ifdef HAVE_CONST_CAST
    array = const_cast<T*>( p );
#else
    array = (T*) p;
#endif
    } else {
      array = 0;
      sz = 0;
    }
}


#define __FUNC__ "SharedArray<T>::SharedArray(const int length)"
template < class T >
inline SharedArray<T>::SharedArray(const int length) {
  FENTER;
  WARN_IF( length < 0, "Constructing an SharedArray<T> of length<=0, (requested length = %d).", length);
  if (length > 0 ) {
    array = T_alloc::allocate( length );
    sz = length;
    maxSize = length;  // own the length of memory
  } else {
    array = 0;
    maxSize = -1;
    sz = 0;
  }
  FEXIT;
}
#undef __FUNC__


#define __FUNC__ "SharedArray<T>::~SharedArray()"
template < class T >
inline SharedArray<T>::~SharedArray() {
  FENTER;
  FCALL free();
  FEXIT;
}
#undef __FUNC__

template < class T >
inline bool SharedArray<T>::operator!() const {
  return (array == 0);
}

template < class T >
inline bool SharedArray<T>::isNull() const {
  return (array == 0);
}

template < class T >
inline bool SharedArray<T>::notNull() const {
  return (array != 0);
}

template < class T >
inline bool SharedArray<T>::isMine() const {
  return (maxSize > 0);
}

template < class T >
inline bool SharedArray<T>::notMine() const {
  return (maxSize < 0);
}

template < class T >
inline int SharedArray<T>::size() const {
  return sz;
}

#define __FUNC__ "const T& SharedArray<T>::operator[]( int i ) const"
template < class T > 
inline const T& SharedArray<T>::operator[]( int i ) const {
#ifdef NDEBUG
  return array[i];
#else
  FENTER;
  ASSERT( i>=0, "Index %d is too small.", i);
  ASSERT( i<sz, "Index %d is too large.", i);
  FRETURN( array[i] );
#endif
}
#undef __FUNC__

#define __FUNC__ "T& SharedArray<T>::operator[]( int i )"
template < class T > 
inline T& SharedArray<T>::operator[]( int i ) { 
#ifdef NDEBUG
  return array[i];
#else
  FENTER;
  ASSERT( i>=0, "Index %d is too small.", i);
  ASSERT( i<sz, "Index %d is too large.", i);
  if (maxSize < 0) { 
    WARNING( "non-const access to an unowned SharedArray... cloning.");
    makePrivateCopy();
  }
  FRETURN( array[i] );
#endif
}
#undef __FUNC__


template < class T > 
inline const T* SharedArray<T>::begin() const { 
  return array; 
}

template < class T > 
inline const T* SharedArray<T>::end() const { 
  return ( array + sz ); 
}


#define __FUNC__ "T* SharedArray<T>::begin()"
template < class T > 
inline T* SharedArray<T>::begin() { 
  FENTER;
  if (maxSize<0){
    WARNING("non-const access to an unowned SharedArray... cloning." );
    makePrivateCopy();
  }
  FRETURN( array );
}
#undef __FUNC__

#define __FUNC__ "T* SharedArray<T>::end()"
template < class T > 
inline T* SharedArray<T>::end() { 
  FENTER;
  if (maxSize<0) {
    WARNING( "non-const access to an unowned SharedArray... cloning,");
    makePrivateCopy();
  }
  FRETURN( array + sz);
}
#undef __FUNC__


template < class T > 
inline const T* SharedArray<T>::lend() const { 
  return array; 
}


#define __FUNC__ "void SharedArray<T>::borrow( const T* p, const int length )"
template < class T > 
inline void SharedArray<T>::borrow( const T* p, const int length ) {
  FENTER;
  FCALL free();
  
#ifdef HAVE_CONST_CAST
  array = const_cast<T*>( p );
#else
  array = (T*) p;
#endif
  sz = length;
  maxSize = -1;
  FEXIT;
}
#undef __FUNC__


#define __FUNC__ "void SharedArray<T>::borrow( const SharedArray<T>& src )"
template < class T >
inline void SharedArray<T>::borrow( const SharedArray<T>& src ) { 
  FENTER;
  if ( array == src.array ) { FEXIT; }
  FCALL borrow( src.lend(), src.size() );
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "T* SharedArray<T>::export()"
template < class T > 
inline T* SharedArray<T>::export() const {
  FENTER;
  T* temp = T_alloc::allocate(sz);
  const T* src = array;
  T* dest = temp;
  for (int i=0; i<sz; i++) { *dest++ = *src++; }
  FRETURN ( temp );
}
#undef __FUNC__

#define __FUNC__ "void SharedArray<T>::import( const T* p, const int length )"
template < class T > 
inline void SharedArray<T>::import( const T* p, const int length ) {
  FENTER;
  ASSERT( p != 0 ,"Cannot import a null pointer p that should point to an array.");
  ASSERT( length > 0 ,"Cannot import an array of length=%d", length);

  free();
  array = T_alloc::allocate( length );
  const T* src = p;
  T* dest = array;
  for (int i=0; i<length; i++) { *dest++ = *src++; }
  maxSize = length;
  sz = length;
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "void SharedArray<T>::import( const SharedArray<T>& src )"
template < class T > 
inline void SharedArray<T>::import( const SharedArray<T>& src ) {
  FENTER;
  free();
  maxSize = ( src.maxSize > src.sz ) ? src.maxSize : src.sz ;
  sz = src.sz;
  array = T_alloc::allocate( maxSize );
  const T* source = src.array;
  T* dest = array;
  const int size = sz;
  for (int i=0; i<size; i++) { *dest++ = *source++; }
  FEXIT;
}
#undef __FUNC__


#define __FUNC__ "T* SharedArray<T>::give()"
template < class T > 
inline T* SharedArray<T>::give() {
  FENTER;
  if (maxSize < 0) { 
    makePrivateCopy();
  }
  maxSize = -1; // set copy as unowned
  FRETURN( array );
}
#undef __FUNC__


#define __FUNC__ "void SharedArray<T>::take( T* p, const int length )"
template < class T > 
inline void SharedArray<T>::take( T* p, const int length ) {
  FENTER;
  ASSERT( p != 0 ,"Cannot import a null pointer p that should point to an array.");
  ASSERT( length > 0 ,"Cannot import an array of length=%d", length);

  free();
  array = p;
  maxSize = length;
  sz = length;
  FEXIT;
}
#undef __FUNC__


#define __FUNC__ "void SharedArray<T>::take( SharedArray<T>& src )"
template < class T >
inline void SharedArray<T>::take( SharedArray<T>& src ) {
  FENTER;
  if ( src.array == 0 ) { 
    FCALL reset(); 
  } else { 
    ASSERT( src.maxSize > 0, "Cannot take ownership from an SharedArray<T> that itself doesn't own the data.");
    if (( array == src.array ) && (maxSize > 0 ) ) { FEXIT; }  // already own autoarray in question
    int temp = src.maxSize;  // src.maxSize is reset when it gives
    FCALL take( src.give(), src.size() );
    maxSize = temp;
  }
  FEXIT;
}
#undef __FUNC__


template < class T >
inline bool SharedArray<T>::giveBack( SharedArray<T>& src ) {
  if (( src.array == array ) && ( src.array != 0 ) ) {
    if (maxSize>0){
      src.maxSize = maxSize;
      maxSize = -1;
      return true;
    }
  }
  return false;
}

template < class T >
inline bool SharedArray<T>::takeBack( T* p, const int size ) {
  if (( array == p ) && ( array != 0 ) ) {
    if ( maxSize == -1 ) {
      maxSize = size;
      return true;
    }
  }
  return false;
}


#define __FUNC__ "void SharedArray<T>::reset()"
template < class T > 
inline void SharedArray<T>::reset() { 
  FENTER;
  FCALL free();
  array = 0;
  sz = 0;
  maxSize = -1;
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "void SharedArray<T>::resize( const int n )"
template < class T > 
inline void SharedArray<T>::resize( const int n ) { 
  FENTER;
  ASSERT( n >= 0, "cannot resize an SharedArray<T> to length=%d",n );
  if ( n == 0 ) {
    reset();
  } else if ( array == 0 ) {
    FCALL take( T_alloc::allocate( n ), n );
  } else if ( ( maxSize > 0 ) && ( n <= maxSize ) ) { // if own array and size <= n;
    sz = n;
  } else if (( maxSize < 0 ) && ( n <= sz )) { // if don't own array, but size is okay
    makePrivateCopy();
    sz = n;
  } else {
    FCALL take( T_alloc::allocate( n ), n );
  } 
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "SharedArray<T>::free()"
template < class T > 
inline void SharedArray<T>::free() {
  FENTER;
  if ( (maxSize > 0) && (array != 0) ) { 
    T_alloc::deallocate( array, maxSize );
  }
  maxSize = -1;
  sz = 0;
  array = 0;
  FEXIT;
}
#undef __FUNC__

template < class T >
inline bool SharedArray<T>::init( const T& t ){
  if ( maxSize>0 ) {
    T* T_temp = array;
    for(int i=0;i<sz;++i) { *T_temp++ = t; }
    return true;
  } else {
    return false;
  }
}


template < class T >
inline bool SharedArray<T>::init( const T& base, const T& increment ){
  if ( maxSize>0 ) {
    T* T_temp = array;
    int offset = base;
    int step = increment;
    for(int i=0;i<sz;++i) { 
      *T_temp = offset; 
      offset += step;
      ++T_temp;
    }
    return true;
  } else {
    return false;
  }
}


template < class T > 
inline void SharedArray<T>::swap( SharedArray<T>& other ) {
  T* T_temp = array;
  array = other.array;
  other.array = T_temp;
  
  int int_temp = sz;
  sz = other.sz;
  other.sz = int_temp;

  int_temp = maxSize;
  maxSize = other.maxSize;
  other.maxSize = int_temp;
}

//template < class T >
//void swap( SharedArray<T>& first, SharedArray<T>& second) {
//  first.swap(second);
//}

SPINDLE_END_NAMESPACE 

#endif
