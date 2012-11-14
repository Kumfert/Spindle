//
// SpindleSystem.cc
//
// $Id: SpindleSystem.cc,v 1.2 2000/02/18 01:32:01 kumfert Exp $
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

#include "spindle/SpindleSystem.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

static int count = 0;

static FILE* TraceFile = stderr;
static FILE* WarnFile = stderr;
static FILE* ErrorFile = stderr;

static unsigned int TraceLevel = 0;
static PersistanceRegistry * Registry = 0;
static StackTrace * Stack = 0;

static char* currentFunction;
static char* currentFile;
static int currentLine;

PersistanceRegistry& 
SpindleSystem::registry() {
  return *Registry;
}

StackTrace& 
SpindleSystem::stack() {
  return *Stack;
}

void
SpindleSystem::startSystem() {
  if (count++ == 0) {
    Registry = new PersistanceRegistry();
    Stack = new StackTrace();
  }
}

void
SpindleSystem::stopSystem() {
  if ( --count==0 ) {
    delete Registry;
    delete Stack;
  }
}



void
SpindleSystem::setWhere( char* func, char* file, int line ) {
  currentFunction = func;
  currentFile = file;
  currentLine = line;
}

void 
SpindleSystem::setTraceFile( FILE* traceFile ) {
  TraceFile = traceFile;
}

void 
SpindleSystem::setWarnFile( FILE* warnFile ) {
  WarnFile = warnFile;
}


void 
SpindleSystem::setErrorFile( FILE* errorFile ) {
  ErrorFile = errorFile;
}

void 
SpindleSystem::setTraceLevel( unsigned int traceLevel ) {
  if ( traceLevel >= 0 ) {
    TraceLevel = traceLevel;
  }
}

unsigned int 
SpindleSystem::getTraceLevel() { 
  return TraceLevel;
}

//
// Now implement the print functions for standard libraries.
//
//
#if SPINDLE_TARGET == std

void 
SpindleSystem::assert_that( bool arg, const char* format, ... ) {
  if ( (! arg)  && (ErrorFile != 0 ) ) {
    va_list ap;
    va_start( ap, format);
    fprintf(ErrorFile, "\nASSERT FAILED:  ");
    vfprintf(ErrorFile, format, ap);
#ifdef SPINDLE_ENABLE_STACK_TRACE
    SpindleSystem::stack().dump( ErrorFile );
    SpindleSystem::stack().reset();
#else
    SpindleSystem::printWhere( ErrorFile );
#endif
    va_end(ap);
    // exit(-1);
  }
}

void 
SpindleSystem::error( unsigned long errorCode, const char* format, ... ) {
  va_list ap;
  va_start( ap, format );
  if ( ErrorFile != 0 ) {
    fprintf(ErrorFile, "\nERROR(%ld):  ",errorCode);
    vfprintf(ErrorFile, format, ap);
#ifdef SPINDLE_ENABLE_STACK_TRACE
    SpindleSystem::stack().dump( ErrorFile );
#else
    printWhere( ErrorFile );
#endif
  }
  va_end(ap);
}

merr
SpindleSystem::mError(  int line, char* file, char* func, merr errcode, char* msg ) {
  if ( ErrorFile != 0 ) {
    fprintf(ErrorFile, "\nERROR(%d): %s in \"%s\" %s[%d]",errcode, msg, func, file, line );
  }
  return errcode;
}

void 
SpindleSystem::warn_if( bool arg, const char* format, ... ) {
  if ( arg && ( WarnFile != 0 ) ) {
    va_list ap;
    va_start( ap, format);
    fprintf(WarnFile, "\nWARNING:  ");
    vfprintf(WarnFile, format, ap);
#ifdef SPINDLE_ENABLE_STACK_TRACE
    SpindleSystem::stack().dump( WarnFile );
#else 
    SpindleSystem::printWhere( WarnFile );
#endif
    va_end(ap);
  }
}

void 
SpindleSystem::warning( const char* format, ... ) {
  va_list ap;
  va_start( ap, format);
  if ( WarnFile != 0 ) {
    fprintf(WarnFile, "\nWARNING:  ");
    vfprintf(WarnFile, format, ap);
#ifdef SPINDLE_ENABLE_STACK_TRACE
    SpindleSystem::stack().dump( WarnFile );
#else
    printWhere( ErrorFile );
#endif
  }
  va_end(ap);
}

void 
SpindleSystem::trace( unsigned int traceLevel, const char* format, ... ) {
  if ( traceLevel & TraceLevel ) {
    va_list ap;
    va_start( ap, format);
    //    const StackInfo& si = SpindleSystem::stack().top();
    //    if ( traceLevel != SPINDLE_TRACE_STACK ) {
    //      fprintf(TraceFile, "\nTrace(%d) %s [%d] - \"%s\"\n\t",traceLevel, si.file, si.line, si.function );
    //    }
    if ( TraceFile != 0 ) {
      vfprintf(TraceFile, format, ap);
    }
    va_end(ap);
  }
}

void 
SpindleSystem::printf( const char* format, ... ) {
  va_list ap;
  va_start( ap, format);
  if ( TraceFile != 0 ) {
    vfprintf(TraceFile, format, ap);
  }
  va_end(ap);
}

void 
SpindleSystem::printWhere( FILE * fp ) {
  if ( (currentFunction != 0) && (currentFile != 0 ) && (fp != 0 ) ) {
    fprintf(fp, "\n \"%s\" in %s-%d\n",currentFunction, currentFile, currentLine );
  }
}

//
// else make a library using matlab constructs...
//
//
#elif SPINDLE_TARGET == matlab 

extern "C" {
#include "mex.h"
}
static char buffer[512*8];

void 
SpindleSystem::assert_that( bool arg, const char* format, ... ) {
  if ( ! arg ) {
    va_list ap;
    va_start( ap, format);
    if ( ErrorFile == 0 )  { // print via Matlab's messaging system.
      int i = sprintf( buffer, "\nASSERT FAILED:  ");
      vsprintf( buffer+i, format, ap );
      mexWarnMsgTxt( buffer );
    } else {  // assume that a file is open
      fprintf(ErrorFile, "\nASSERT FAILED:  ");
      vfprintf(ErrorFile, format, ap);
#ifdef SPINDLE_ENABLE_STACK_TRACE
      SpindleSystem::stack().dump( ErrorFile );
      SpindleSystem::stack().reset();
#else
      SpindleSystem::printWhere( ErrorFile );
#endif
    }
    va_end(ap);
    // exit(-1);
  }
}

void 
SpindleSystem::error( unsigned long errorCode, const char* format, ... ) {
  va_list ap;
  va_start( ap, format );
  if ( ErrorFile == 0 ) {
      int i = sprintf( buffer, "\nASSERT FAILED:  ");
      vsprintf( buffer+i, format, ap );
      mexWarnMsgTxt( buffer );
  } else if ( ErrorFile != 0 ) {
    fprintf(ErrorFile, "\nERROR(%ld):  ",errorCode);
    vfprintf(ErrorFile, format, ap);
#ifdef SPINDLE_ENABLE_STACK_TRACE
    SpindleSystem::stack().dump( ErrorFile );
#else
    printWhere( ErrorFile );
#endif
  }
  va_end(ap);
}

merr
SpindleSystem::mError(  int line, char* file, char* func, merr errcode, char* msg ) {
  if ( ErrorFile == 0 ) {
      vsprintf( buffer, "\nERROR(%d): %s in \"%s\" %s[%d]",errcode, msg, func, file, line );
      mexWarnMsgTxt( buffer );
  } else if ( ErrorFile != 0 ) {
    fprintf(ErrorFile, "\nERROR(%d): %s in \"%s\" %s[%d]",errcode, msg, func, file, line );
  }
  return errcode;
}

void 
SpindleSystem::warn_if( bool arg, const char* format, ... ) {
  if ( arg ) {
    va_list ap;
    va_start( ap, format);
    if ( WarnFile == 0 ) {
      int i = sprintf( buffer, "\nWARNING:   ");
      vsprintf( buffer+i, format, ap );
      mexWarnMsgTxt( buffer );
    } else {
      fprintf(WarnFile, "\nWARNING:  ");
      vfprintf(WarnFile, format, ap);
#ifdef SPINDLE_ENABLE_STACK_TRACE
      SpindleSystem::stack().dump( WarnFile );
#else 
      SpindleSystem::printWhere( WarnFile );
#endif
    }
    va_end(ap);
  }
}

void 
SpindleSystem::warning( const char* format, ... ) {
  va_list ap;
  va_start( ap, format);
  if ( WarnFile == 0 ) {
    int i = sprintf( buffer, "\nWARNING:  ");
    vsprintf( buffer+i, format, ap );
    mexWarnMsgTxt( buffer );
  } else if ( WarnFile != 0 ) {
    fprintf(WarnFile, "\nWARNING:  ");
    vfprintf(WarnFile, format, ap);
#ifdef SPINDLE_ENABLE_STACK_TRACE
    SpindleSystem::stack().dump( WarnFile );
#else
    printWhere( ErrorFile );
#endif
  }
  va_end(ap);
}

void 
SpindleSystem::trace( unsigned int traceLevel, const char* format, ... ) {
  if ( traceLevel & TraceLevel ) {
    va_list ap;
    va_start( ap, format);
    //    const StackInfo& si = SpindleSystem::stack().top();
    //    if ( traceLevel != SPINDLE_TRACE_STACK ) {
    //      fprintf(TraceFile, "\nTrace(%d) %s [%d] - \"%s\"\n\t",traceLevel, si.file, si.line, si.function );
    //    }
    if ( TraceFile == 0 ) {
      vsprintf( buffer, format, ap );
      mexPrintf( "%s", buffer );
    } else if ( TraceFile != 0 ) {
      vfprintf(TraceFile, format, ap);
    }
    va_end(ap);
  }
}

void 
SpindleSystem::printf( const char* format, ... ) {
  va_list ap;
  va_start( ap, format);
  if ( TraceFile == 0 ) {
    vsprintf( buffer, format, ap );
    mexPrintf( "%s", buffer );
  } else  if ( TraceFile != 0 ) {
    vfprintf(TraceFile, format, ap);
  }
  va_end(ap);
}

void 
SpindleSystem::printWhere( FILE * fp ) {
  if ( (currentFunction != 0) && (currentFile != 0 ) && (fp != 0 ) ) {
    fprintf(fp, "\n \"%s\" in %s-%d\n",currentFunction, currentFile, currentLine );
  }
  if ( (currentFunction != 0) && (currentFile != 0 ) && (fp == 0 ) ) {
    sprintf( buffer, "\n \"%s\" in %s-%d\n",currentFunction, currentFile, currentLine );
    mexPrintf( "%s", buffer );
  }
}

#elif  SPINDLE_TARGET == petsc 

extern "C" {
#include "petsc.h"
}
static char buffer[512*8];

void 
SpindleSystem::assert_that( bool arg, const char* format, ... ) {
  if ( ! arg  && ( ErrorFile != 0 ) ) {
    va_list ap;
    va_start( ap, format);
    int i = sprintf( buffer, "\nASSERT FAILED:  ");
    vsprintf( buffer+i, format, ap);
    PetscFPrintf( PETSC_COMM_SELF, ErrorFile, "%s", buffer );
    SpindleSystem::printWhere( ErrorFile );
    va_end(ap);
    // exit(-1);
  }
}

void 
SpindleSystem::error( unsigned long errorCode, const char* format, ... ) {
  va_list ap;
  va_start( ap, format );
  if ( ErrorFile != 0 ) {
    int i = sprintf( buffer, "\nERROR(%ld):  ", errorCode );
    vsprintf( buffer+i, format, ap);
    PetscFPrintf( PETSC_COMM_SELF, ErrorFile, "%s", buffer );
    SpindleSystem::printWhere( ErrorFile );
    printWhere( ErrorFile );
  }
  va_end(ap);
}

merr
SpindleSystem::mError(  int line, char* file, char* func, merr errcode, char* msg ) {
  if ( ErrorFile != 0 ) {
    PetscFPrintf( PETSC_COMM_SELF, ErrorFile, "\nERROR(%d): %s in \"%s\" %s[%d]",errcode, msg, func, file, line );
  }
  return errcode;
}

void 
SpindleSystem::warn_if( bool arg, const char* format, ... ) {
  if ( arg ) {
    va_list ap;
    va_start( ap, format);
    if ( WarnFile != 0 ) {
      int i = sprintf( buffer, "\nWARNING:  ");
      vsprintf( buffer+i, format, ap );
      PetscFPrintf( PETSC_COMM_SELF, WarnFile, "%s", buffer );
      SpindleSystem::printWhere( WarnFile );
    }
    va_end(ap);
  }
}

void 
SpindleSystem::warning( const char* format, ... ) {
  va_list ap;
  va_start( ap, format);
  if ( WarnFile != 0 ) {
    int i = sprintf( buffer, "\nWARNING:  ");
    vsprintf(buffer+i, WarnFile, format, ap);
    PetscFPrintf( PETSC_COMM_SELF, WarnFile, "%s", buffer );
    printWhere( WarnFile );
  }
  va_end(ap);
}

void 
SpindleSystem::trace( unsigned int traceLevel, const char* format, ... ) {
  if ( traceLevel & TraceLevel ) {
    va_list ap;
    va_start( ap, format);
    //    const StackInfo& si = SpindleSystem::stack().top();
    //    if ( traceLevel != SPINDLE_TRACE_STACK ) {
    //      fprintf(TraceFile, "\nTrace(%d) %s [%d] - \"%s\"\n\t",traceLevel, si.file, si.line, si.function );
    //    }
    if ( TraceFile != 0 ) {
      vsprintf(buffer , format, ap);
      PetscFPrintf( PETSC_COMM_SELF, TraceFile, "%s", buffer );
    }
    va_end(ap);
  }
}

void 
SpindleSystem::printf( const char* format, ... ) {
  va_list ap;
  va_start( ap, format);
  vsprintf( buffer , format, ap );
  PetscPrintf( PETSC_COMM_SELF,  "%s", buffer );
  va_end(ap);
}

void 
SpindleSystem::printWhere( FILE * fp ) {
  if ( (currentFunction != 0) && (currentFile != 0 ) && (fp != 0 ) ) {
    PetscFPrintf( PETSC_COMM_SELF, fp, "\n \"%s\" in %s-%d\n",currentFunction, currentFile, currentLine );
  }
  if ( (currentFunction != 0) && (currentFile != 0 ) && (fp == 0 ) ) {
    PetscPrintf( PETSC_COMM_SELF, "\n \"%s\" in %s-%d\n",currentFunction, currentFile, currentLine );
  }
}

#else
#error Error do not recognize SPINDLE TARGET of SPINDLE_TARGET
#endif
