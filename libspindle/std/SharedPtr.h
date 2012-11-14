//
// SharedPtr.h  -- A smart pointer implementation.
//
//  $Id: SharedPtr.h,v 1.2 2000/02/18 01:31:57 kumfert Exp $
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
//  SharedPtr<T>'s have a concept of ``ownership''.  Each object pointed
//  to by SharedPtr<T>'s are owned by only one SharedPtr<T>.  This one 
//  ``owner'' is allowed read, write, and delete priveledges, all other
//  SharedPtr<T>'s that point to that object, only have read priviledges.
//
//  This ownership is strictly enforced, unless the symbol NDEBUG is
//  #define'd, in which case the checking is relaxed.
//
//
//  Access is granted by borrow() and lend().  Ownership is granted
//  by take() and give().  In the real world, access and ownership is
//  clear in these cases.  The import() and export() pair is for making
//  private copies of data and releasingthe rest.
//
//

#ifndef AUTO_PTR_H_
#define AUTO_PTR_H_

#ifndef export
#define export exportIt
#endif

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif 

SPINDLE_BEGIN_NAMESPACE

template<class T>
class SharedPtr { 
private:
  T* pointee;
  bool ownPtr;

public:
  explicit SharedPtr( T* p );            // constructor
  explicit SharedPtr( const T* p = 0 );  // constructor
  ~SharedPtr();                         // destructor
  
  SharedPtr( const SharedPtr<T>& rhs ); // copy constructor: ownership doesn't transfer
  
  SharedPtr<T>& operator=( SharedPtr<T>& rhs ) ; // assignment operator: ownership transfers

  // Accessors
  bool operator!() const;       // test for null pointer
  bool isNull() const;
  bool notNull() const;
  bool isMine() const;
  bool notMine() const;
  const T& operator*() const;   // dereference (const and non-const versions)
  T& operator*();
  const T* operator->() const;
  T* operator->();

  // Manipulators
  const T* lend() const;
  void borrow( const T* p );
  void borrow( const SharedPtr<T>& ap );
  T* export() const;
  void import( const T* p );
  T* give();
  void take( T* p );
  void take( SharedPtr<T>& ap );
  bool giveBack( SharedPtr<T>& dest );

  void swap( SharedPtr<T>& other ); // will exchange ownership
  //friend void swap<class T>( SharedPtr<T>& a, SharedPtr<T>& b);

private:
  void reset();
};

#ifdef __FUNC__
#undef __FUNC__
#endif


template < class T >
inline SharedPtr<T>::SharedPtr( T* p ) {
  pointee = p;
  ownPtr = ( pointee == 0 ) ? false : true;
}

template < class T >
inline SharedPtr<T>::SharedPtr( const T* p ) {
#ifdef HAVE_CONST_CAST
    pointee = const_cast<T*>( p );
#else
    pointee = ( T* ) p;
#endif
    ownPtr = false; 
}

template < class T >
inline SharedPtr<T>::~SharedPtr() {
  if (ownPtr && (pointee != 0) ) { 
    delete pointee; 
  }
}

template < class T > // copy constructor
inline SharedPtr<T>::SharedPtr( const SharedPtr<T>& rhs ) {
    pointee = rhs.pointee;
    ownPtr = false;
}

template < class T >          // assignment operator
inline  SharedPtr<T>& SharedPtr<T>::operator=( SharedPtr<T>& rhs ) {
  if ( this != &rhs ) {  // make sure not assigning to self!
    take( rhs.give() ); 
  }
  return *this;
}

template < class T >
inline bool SharedPtr<T>::operator!() const { 
  return (pointee == 0);
}

template < class T >
inline bool SharedPtr<T>::isNull() const { 
  return (pointee == 0);
}

template < class T >
inline bool SharedPtr<T>::notNull() const { 
  return (pointee != 0);
}

template < class T >
inline bool SharedPtr<T>::isMine() const { 
  return ownPtr;
}

template < class T >
inline bool SharedPtr<T>::notMine() const { 
  return (! ownPtr);
}


#define __FUNC__ "const T& SharedPtr<T>::operator*() const"
template < class T > 
inline const T& SharedPtr<T>::operator*() const {
  FENTER;
  ASSERT( pointee != 0 , "Trying to de-reference a NULL pointer" );
  FRETURN ( *pointee ) ;
}
#undef __FUNC__


#define __FUNC__ "T& SharedPtr<T>::operator*()"
template < class T > 
inline T& SharedPtr<T>::operator*() {
  FENTER;
  ASSERT( pointee != 0 , "Trying to de-reference a NULL pointer" );
  WARN_IF( ! ownPtr, "non-const access requested of SharedPtr<T> that doesn't own pointee." );
  FRETURN( *pointee ) ;
}
#undef __FUNC__

#define __FUNC__ "const T* SharedPtr<T>::operator->() const"
template < class T >
inline const T* SharedPtr<T>::operator->() const {
  FENTER;
  ASSERT( pointee != 0 , "Trying to de-reference a NULL pointer" );
  FRETURN( pointee );
}
#undef __FUNC__

#define __FUNC__ "T* SharedPtr<T>::operator->()"
template < class T >
inline T* SharedPtr<T>::operator->() {
  FENTER;
  ASSERT( pointee != 0, "Trying to de-reference a NULL pointer" );
  WARN_IF( ! ownPtr, "non-const access requested of SharedPtr<T> that doesn't own pointee." );
  FRETURN( pointee );
}
#undef __FUNC__

template < class T > 
inline const T* SharedPtr<T>::lend() const { 
  return pointee; 
}

template < class T > 
inline void SharedPtr<T>::borrow( const T* p ) {
  reset();
#ifdef HAVE_CONST_CAST
  pointee = const_cast<T*>( p );
#else
  pointee = (T*) p;
#endif
  ownPtr = false;
}

template < class T > 
inline void SharedPtr<T>::borrow( const SharedPtr<T>& ap ) {
  if (ownPtr && ( pointee == ap.pointee ) ) { return; }
  reset();
#ifdef HAVE_CONST_CAST
  pointee = const_cast<T*>( ap.pointee );
#else
  pointee = (T*) ap.pointee;
#endif
  ownPtr = false;
}

template < class T > 
inline T* SharedPtr<T>::export() const {
  T* temp = new T(*pointee);
  return temp;
}


template < class T > 
inline void SharedPtr<T>::import( const T* p ) {
  reset();
  pointee = new T(*p);
  ownPtr = true;
}
  
template < class T > 
inline T* SharedPtr<T>::give() {
  if ( isNull() ) { return 0; }
  if (ownPtr == false) {
    WARNING("Trying to ownership to non-owned object, cloning.");
    T* temp = new T(*pointee);
    pointee = temp; 
  }
  ownPtr = false;
  return pointee;
}

template < class T > 
inline void SharedPtr<T>::take( T* p ) {
  reset();
  pointee = p;
  ownPtr = true;
}

template < class T > 
inline void SharedPtr<T>::take( SharedPtr<T>& ap ) {
  if (ownPtr && ( pointee == ap.pointee ) ) { return; }
  reset();
  pointee = ap.pointee;
  if (ap.ownPtr == true ) {
    ap.ownPtr = false;
    ownPtr = true;
  } else {
    ownPtr = false;
  }
}

template < class T > 
inline bool SharedPtr<T>::giveBack( SharedPtr<T>& ap ) {
  if ( ownPtr  && ( pointee == ap.pointee ) ) {
    ownPtr = false;
    ap.ownPtr = true;
    return true;
  }
  return false;
}

template < class T > 
inline void SharedPtr<T>::reset() {
  if (ownPtr && (pointee != 0) ) { 
    delete pointee;
  }
  ownPtr = false;
  pointee = 0;
}

template < class T> 
void SharedPtr<T>::swap( SharedPtr<T>& other ) {
  T* T_temp = pointee;
  pointee = other.pointee;
  other.pointee = T_temp;
  
  bool bool_temp = ownPtr;// assignment operator: ownership transfers
  ownPtr = other.ownPtr;
  other.ownPtr = bool_temp;
}

//template < class T> 
//inline void swap( SharedPtr<T>& first, SharedPtr<T>& second ) {
//  first.swap( second );
//}

SPINDLE_END_NAMESPACE

#endif
