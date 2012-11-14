//
// SpindleSystem.h
//
// $Id: SpindleSystem.h,v 1.2 2000/02/18 01:32:01 kumfert Exp $
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
// ========================================================================
//
//  This header consolidates many of the system resources available.
//  It handles the stacktrace and the object registry for classes
//  derived from SpindleBaseClass
//
//  It also defines the applicable timer for the system and the
//  files to which warnings, errors, and trace prints are directed.
//
//  Users should not use the assert/warn/trace static members of 
//  SpindleSystem directly, but the macros defined later in this file
//

#ifndef SPINDLE_SYSTEM_H_
#define SPINDLE_SYSTEM_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#  ifdef SPINDLE__IOSTREAM_BEFORE_CSTDIO_BUG
#    include <iostream.h>
#  endif
#  include <stdio.h>
#  include <stdarg.h>
#else
#  ifdef SPINDLE__IOSTREAM_BEFORE_CSTDIO_BUG
#    include <iostream>
#  endif
#  include <cstdio>
#  include <cstdarg>
using namespace std;
#endif

#ifdef HAVE_NAMESPACES
using std::FILE;
#endif

#ifndef SPINDLE_STACK_TRACE_H_
#include "spindle/StackTrace.h"
#endif

#ifndef SPINDLE_PERSISTANCE_REGISTRY_H_
#include "spindle/PersistanceRegistry.h"
#endif

typedef int merr;

#ifndef SPINDLE_ERROR_CODES_H_
#include "spindle/SpindleErrorCodes.h"
#endif

#ifndef SPINDLE_TRACE_CODE_H_
#include "spindle/SpindleTraceCodes.h"
#endif

// now check if using StackTrace is enabled or disabled.
// default is disabled

/** @name General Error Handling Configuration*/
//@{
/**
 * @type macro
 * @name SPINDLE_DISABLE_STACK_TRACE
 * 
 * Disables the stack trace since it can incur a substantial overhead
 * and a good debugger can do the job better anyway. 
 *
 * If neither #SPINDLE_ENABLE_STACK_TRACE# nor #SPINDLE_DISABLE_STACK_TRACE#
 * is defined, the default is to leave it disabled.
 *
 * Used in #SpindleSystem.h#
 */

/**
 * @type macro
 * @name SPINDLE_ENABLE_STACK_TRACE
 * 
 * Enables an explicit stack trace.  It can be useful in limited
 * circumstances, particularly when a good debugger cannot be found.
 *
 * If neither #SPINDLE_ENABLE_STACK_TRACE# nor #SPINDLE_DISABLE_STACK_TRACE#
 * is defined, the default is to leave it disabled.
 *
 * Used in #SpindleSystem.h#
 */

#if defined( SPINDLE_DISABLE_STACK_TRACE )
#  if defined( SPINDLE_ENABLE_STACK_TRACE )
#    error cannot define both SPINDLE_ENABLE_STACK_TRACE and SPINDLE_DISABLE_STACK_TRACE
#  endif
#endif
#ifndef SPINDLE_DISABLE_STACK_TRACE
#  ifndef SPINDLE_ENABLE_STACK_TRACE
#    define SPINDLE_DISABLE_STACK_TRACE
#  endif
#endif


/**
 * @type macro
 * @name SPINDLE_DISABLE_ASSERTS
 * 
 * Disables the spindle assert system, which is more
 * robust than #std::assert()# and doesn't force the code to crash.
 * Define #SPINDLE_ENABLE_ASSERTS# to override.
 *
 * If neither  #SPINDLE_ENABLE_ASSERTS# nor #SPINDLE_DISABLE_ASSERTS#
 * is defined, the default is to leave them enabled.
 * 
 * Used in #SpindleSystem.h#
 */

/**
 * @type macro
 * @name SPINDLE_ENABLE_ASSERTS
 * 
 * Enables the spindle assert system, which is more
 * robust than #std::assert()# and doesn't force the code to crash.
 *
 * If neither  #SPINDLE_ENABLE_ASSERTS# nor #SPINDLE_DISABLE_ASSERTS#
 * is defined, the default is to leave them enabled.
 * 
 * Used in #SpindleSystem.h#
 */

#if defined( SPINDLE_DISABLE_ASSERTS )
#  if defined( SPINDLE_ENABLE_ASSERTS )
#    error cannot define both SPINDLE_ENABLE_ASSERTS and SPINDLE_DISABLE_ASSERTS
#  endif
#endif
#ifndef SPINDLE_DISABLE_ASSERTS
#  ifndef SPINDLE_ENABLE_ASSERTS
#    define SPINDLE_ENABLE_ASSERTS
#  endif
#endif

// finally see if errchk is enabled or disabled
#if defined( SPINDLE_DISABLE_MERRCHK )
#  if defined( SPINDLE_ENABLE_MERRCHK )
#    error cannot define both SPINDLE_ENABLE_MERRCHK and SPINDLE_DISABLE_MERRCHK
#  endif
#endif
#ifndef SPINDLE_DISABLE_MERRCHK
#  ifndef SPINDLE_ENABLE_MERRCHK
#    define SPINDLE_ENABLE_MERRCHK
#  endif
#endif

//@}

SPINDLE_BEGIN_NAMESPACE
/**
 *  @memo Consolidates and Abstracts various system resources
 *  @type class
 *  
 * This class can handle a lot of details such as managing a 
 * persistance registry for descendants of #SpindleBaseClass#
 * as well as funnelling error, warning, and trace messages
 * various places.
 * 
 * @author Gary Kumfert
 * @version #$Id: SpindleSystem.h,v 1.2 2000/02/18 01:32:01 kumfert Exp $#
 * 
 */
class SpindleSystem { 
private:
  static void startSystem();  //   > used for insuring services are available in all units
  static void stopSystem();   //  /
  static void printWhere( FILE * fp );
public:
  typedef stopwatch timer;

  /** @name constructors/destructors 
   *
   */
  //@{
  /// custom default constructor. creates a unique #PersistanceRegistry# and #StackTrace#
  SpindleSystem();
  /// custom default destructor.
  ~SpindleSystem();
  //@}

  /** @name accessors to system resources 
   *
   */
  //@{
  /// get a reference to the persistance registry
  static PersistanceRegistry& registry(); 
  /// get a reference to the stack trace
  static StackTrace& stack();
  //@}
  static void assert_that( bool arg, const char* format, ... );
  static void error( unsigned long errorCode, const char* format, ... );
  static merr mError( int line, char* file, char* func, merr errcode, char* msg );
  static void warn_if( bool arg, const char* format, ... );
  static void warning( const char* format, ... );
  static void trace( unsigned int traceLevel, const char* format, ... );
  static void printf( const char* format, ... );
  static void do_nothing( ... );

  /** manipulators for warning/trace/error reporting 
   *
   */
  //@{
  /// set the #FILE# where trace messages go
  static void setTraceFile( FILE* traceFile );
  /// set the #FILE# where warning messages go
  static void setWarnFile( FILE* warnFile );
  /// set the #FILE# where error messages go
  static void setErrorFile( FILE* errorFile );
  /// set the trace level (see #SpindleTraceCodes.h#
  static void setTraceLevel( unsigned int traceLevel );
  /// get the current trace level
  static unsigned int getTraceLevel();
  //@}
  static void setWhere( char* func, char* file, int line );

};

static SpindleSystem  dummySpindleSystemClassThatIsStaticInEveryFileThatIncludesThisHeaderFile;

inline
SpindleSystem::SpindleSystem() {
  startSystem();
}

inline
SpindleSystem::~SpindleSystem() {
  stopSystem();
}

inline void
SpindleSystem::do_nothing( ... ) {
  //  va_list ap;
  //  va_start(ap, );
  //  vfprintf(stderr, format, ap );
  //  va_end(ap);
}           

SPINDLE_END_NAMESPACE 

/** @name StackTrace Helper Macros
 *
 * Deprecated.
 *
 * These macros were set up for specific cases where
 * a debugger was not available and some stack information
 * was needed.  These macros are disabled with
 * #SPINDLE_DISABLE_STACK_TRACE#
 */
//@{
/** 
 * @type macro
 * @name __FUNC__
 *
 * Represents a zero terminated character array (aka a string)
 * of function/method name much in the
 * same way that #__FILE__# and #__LINE__# are built
 * in by the preprocessor.
 *
 * Used by #FENTER#, #FCALL#, #FRETURN#, #FEXIT#
 *
 */

/**
 * @type macro
 * @name FENTER
 *
 * A macro that appears once at the beginning of any function/method
 * that meaninfully interacts with the #StackTrace#
 * 
 * if #SPINDLE_ENABLE_STACK_TRACE# is not defined, it does nothing.
 *
 * Used with #FCALL#, #FRETURN#, #FEXIT#
 *
 */

/**
 * @type macro
 * @name FCALL
 *
 * A macro that marks the last line visited in the current function/method
 * before calling another.  Appears only in functions that interact
 * with the #StackTrace#.
 * 
 * if #SPINDLE_ENABLE_STACK_TRACE# is not defined, it does nothing.
 *
 * Used with #FENTER#, #FRETURN#, #FEXIT#
 *
 */

/**
 * @type macro
 * @name FRETURN
 * @args val
 *
 * A macro that appears instead of #return val# for any function/method
 * that meaninfully interacts with the #StackTrace#
 * 
 * if #SPINDLE_ENABLE_STACK_TRACE# is not defined, it is simply #return val#.
 *
 * Used with #FENTER#, #FCALL#, #FRETURN#, #FEXIT#
 *
 */

/**
 * @type macro
 * @name FEXIT
 *
 * A macro that appears instead of #return# for any function/method
 * that meaninfully interacts with the #StackTrace#
 * 
 * if #SPINDLE_ENABLE_STACK_TRACE# is not defined, it is simply #return#.
 *
 * Used with #FENTER#, #FCALL#, #FRETURN#
 *
 */
//@}

#ifdef SPINDLE_ENABLE_STACK_TRACE

#define FENTER \
        SpindleSystem::stack().push(__LINE__,__FILE__,__FUNC__); \
        int SPINDLE_StackSize_local_variable = SpindleSystem::stack().size(); \
	SpindleSystem::trace( SPINDLE_TRACE_STACK,"\n%*d: Entered \"%s\"", \
                     SPINDLE_StackSize_local_variable*4, SPINDLE_StackSize_local_variable,__FUNC__);
          


#define FCALL \
        SpindleSystem::stack().setLastLine( __LINE__ ); 

#define FRETURN( RTRN ); \
        SpindleSystem::stack().resize(SPINDLE_StackSize_local_variable,__LINE__,__FILE__,__FUNC__); \
	SpindleSystem::trace( SPINDLE_TRACE_STACK,"\n%*d: Exited \"%s\"", \
                     SPINDLE_StackSize_local_variable*4, SPINDLE_StackSize_local_variable,__FUNC__);\
	return (RTRN);
 
#define FEXIT \
        SpindleSystem::stack().resize(SPINDLE_StackSize_local_variable,__LINE__,__FILE__,__FUNC__); \
	SpindleSystem::trace( SPINDLE_TRACE_STACK,"\n%*d: Exited \"%s\"", \
                     SPINDLE_StackSize_local_variable*4, SPINDLE_StackSize_local_variable,__FUNC__);\
	return;

#define ERROR \
        FCALL; \
	SpindleSystem::error

#define WARNING \
	FCALL; \
	SpindleSystem::warning

#else // ifdef SPINDLE_DISABLE_STACK_TRACE

#ifdef SPINDLE_ENABLE_LIMITED_TRACE

#define FENTER              ;
#define FCALL               SpindleSystem::setWhere( __FUNC__, __FILE__, __LINE__ );
#define FRETURN( RTRN )     return (RTRN)
#define FEXIT               return

#define ERROR \
        SpindleSystem::setWhere( __FUNC__, __FILE__, __LINE__ ); \
	SpindleSystem::error

#define WARNING \
        SpindleSystem::setWhere( __FUNC__, __FILE__, __LINE__ ); \
	SpindleSystem::warning

#else  

#define FENTER              ;
#define FCALL               ;
#define FRETURN( RTRN )     return (RTRN)
#define FEXIT               return

#define ERROR \
	SpindleSystem::error

#define WARNING \
	SpindleSystem::warning
#endif // SPINDLE_ENABLE_LIMITED_TRACE

#endif // STACK_TRACE

#define WARN_IF \
        FCALL; \
	SpindleSystem::warn_if

#define TRACE \
        FCALL; \
	SpindleSystem::trace

#define PRINTF \
        FCALL; \
        SpindleSystem::printf

#if defined( SPINDLE_ENABLE_ASSERTS )

#define ASSERT \
	FCALL; \
        SpindleSystem::assert_that

#elif defined( SPINDLE_DISABLE_ASSERTS )

#define ASSERT \
	FCALL; \
        SpindleSystem::do_nothing 

#endif 

/** @name Printing Macros 
 *
 */
//@{

/**
 * @type function
 * @name PRINTF
 * @args ( const char format, ... )
 *
 * Like standard printf, but can change
 * if compiled for Matlab or PETSc
 */

/**
 * @type function
 * @name ASSERT
 * @args ( bool proposition, const char format, ... )
 *
 * If the proposition is false, it prints the
 * corrsponding text (formatted like #PRINTF#) to 
 * the #FILE# pointer for error messages specified by #SpindleSystem#
 */

/**
 * @type function
 * @name ERROR
 * @args ( unsigned int errorCode, const char format, ... )
 *
 * prints the 
 * corrsponding text (formatted like #PRINTF#) to 
 * the #FILE# pointer for error messages specified by #SpindleSystem#
 */

/**
 * @type function
 * @name TRACE
 * @args ( unsigned int tracelevel, const char format, ... )
 *
 * If the current trace level in #SpindleSystem# bitor
 * the #tracelevel# specified is not zero, then print
 * the corresponding text like #PRINTF# to the 
 * #FILE# pointer for trace messages as specified by #SpindleSystem#
 *
 */

/**
 * @type function
 * @name WARNING
 * @args ( const char format, ... )
 *
 * Print a warning on the #FILE# determined by #SpindleSystem#
 * for warning messages. The corresponding text is printed like #PRINTF#.
 *
 */

/**
 * @type function
 * @name WARN_IF
 * @args bool predicate, const char format, ...
 *
 * Print a warning on the #FILE# determined by #SpindleSystem#
 * for warning messages only if the #predicate == true#. 
 * The corresponding text is printed like #PRINTF#.
 *
 */

//@}

#if defined( SPINDLE_ENABLE_MERRCHK )
// default
#define MERR(n,s) { return SpindleSystem::mError(__LINE__, __FILE__, __FUNC__,n,s); }
#define MERRCHK(n) {if(n<0) MERR(n,(char *)0);}
#define MERR_A(n,s) { int _merr = SpindleSystem::mError(__LINE__, __FILE__, __FUNC__,n,s); exit(_merr) }
#define MERRCHK_A(n) {if(n<0) MERR_A(n,(char *)0);}
#define BERRCHK(n) {if (!n) MERR(n,(char *)0);}
#define BERRCHK_A(n) {if (!n) MERR_A(SPINDLE_ERR_MISC,(char *)0);}
#elif defined ( SPINDLE_DISABLE_MERRCHK )
#define MERR(n,s)    ;
#define MERRCHK(n)   ;
#define MERR_A(n,s)  ;
#define MERRCHK_A(n) ;
#define BERRCHK(n)   ;
#define BERRCHK_A(n) ;
#endif

#endif // SPINDLE_SYSTEM_H_
