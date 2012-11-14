//
// StackTrace.h
//
// $Id: StackTrace.h,v 1.2 2000/02/18 01:32:01 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1997, Old Dominion University.  All rights reserved.
// 
//  Permission to use, copy, modify , distribute and sell this software and
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

#ifndef SPINDLE_STACK_TRACE_H_
#define SPINDLE_STACK_TRACE_H_

#include <stdio.h>

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include "vector.h"
#else
#include <vector>
using std::vector;
#endif

SPINDLE_BEGIN_NAMESPACE

struct StackInfo {
  int line;
  const char * file;
  const char * function;
  StackInfo( const int lineNumber, const char* fileName, const char* functionName) :
    line(lineNumber), file(fileName), function(functionName) { }
  StackInfo(): line(-1), file(0), function(0) { }
}; 

class SpindleSystem;

class StackTrace {
  friend SpindleSystem;
public:
  typedef vector< StackInfo > StackType;

private:
  StackType stack;
  StackTrace( const StackTrace& src );    // no implementation
  void operator=( const StackTrace& src); // no implementation
  StackTrace();

public:
  static StackTrace& get();
  ~StackTrace();
  void push( int lineNumber, const char* fileName="Unknown", const char* functionName="Unknown" );
  const StackInfo& top() const;
  void pop();
  int size() const;
  void resize( size_t targetSize, int lineNumber, char* fileName, char* functionName);
  void setLastLine( const int i );

  void dump( FILE * stream ) const;
  void reset();
};

inline void
StackTrace::push(int lineNumber, const char* fileName, const char* functionName ) {
  stack.push_back( StackInfo( lineNumber, fileName, functionName ) );
}

inline const StackInfo& 
StackTrace::top() const { 
  return stack.back();
}

inline void
StackTrace::pop() {
  if ( stack.size() == 0) { return; }
  stack.pop_back();
}

inline int
StackTrace::size() const { 
  return stack.size();
}

inline void
StackTrace::setLastLine( const int i ) {
  stack.back().line = i;
}

SPINDLE_END_NAMESPACE

#endif


