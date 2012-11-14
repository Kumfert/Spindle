//
// StackTrace.cc
//
// $Id: StackTrace.cc,v 1.2 2000/02/18 01:32:01 kumfert Exp $
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

#include "spindle/StackTrace.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

StackTrace::StackTrace() { }


StackTrace::~StackTrace() {
  if (stack.size() > 0 ) { 
    fprintf(stderr,"\nError: Stack not empty");
    dump(stderr);
  }
}

void
StackTrace::resize( size_t targetSize, int lineNumber, char* fileName, char* functionName) {
  if ( targetSize == stack.size() ) { 
    pop();
    return;
  }
  const char* fn = ( fileName==0 ) ? "unknown" : fileName;
  const char* ft = ( functionName==0) ? "unknown" : functionName;
  fprintf(stderr,"\nWARNING: StackSize %d is incorrect! %s in \"%s\" - %d",stack.size(),ft,fn,lineNumber );
  if ( targetSize < stack.size() ) { 
    // somehow we've forgotten to pop some members off of the stack
    fprintf(stderr,"\n         Probably forgot to pop %d members off the stack", stack.size() - targetSize);
    fprintf(stderr,"\n         Before modification...");
    dump( stderr );
    while( targetSize < stack.size() ) {
      pop();
    }
    fprintf(stderr,"\n         After modification...");
    dump( stderr );
  } else if ( targetSize > stack.size() ) {
    // Somehow, we've forgotten to push some members onto the stack
    fprintf(stderr,"\n         Probably forgot to push %d members onto the stack", stack.size() - targetSize);
    fprintf(stderr,"\n         Be sure to check %s and the functions it calls", functionName);
    fprintf(stderr,"\n         Before Modification...");
    dump(stderr);
    while( targetSize > stack.size() ) {
      push(lineNumber, fileName, "Dummy entries: Check for missings pushes");
    }
    fprintf(stderr,"\n        After Modification...");
    dump(stderr);
  }
  pop();
}
  

void
StackTrace::dump( FILE * stream ) const {
  for( StackType::const_reverse_iterator it = stack.rbegin(), stop = stack.rend();
       it != stop; ++ it ) {
    const char * fn = (( (*it).function == 0 ) ? "unknown" : (*it).function );
    const char * fl = (( (*it).file == 0 ) ? "unknown" : (*it).file );
    fprintf( stream, "\n  %s in \"%s\" - %d",fn, fl, (*it).line );
  }
}

void 
StackTrace::reset() {
  stack.erase( stack.begin(), stack.end() );
}

StackTrace& 
StackTrace::get() {
  static StackTrace st;
  return st;
}
