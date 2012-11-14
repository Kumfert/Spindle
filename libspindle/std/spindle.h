//
// spindle.h  -- default header file
//
//  $Id: spindle.h,v 1.3 2000/03/07 22:24:34 kumfert Exp $
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
//  purpose.  It is povided "as is" without express or implied warranty.
//
///////////////////////////////////////////////////////////////////////
//
//  This is a standard header file included by every piece of SPINDLE
//  code.   It requires the macro ``SPINDLE_TARGET'' to be defined.
//  If the target is not recognized, std is assumed.
// 
//  This header file accomplishes the following:
//      + chooses target-specific header files for I/O and custom allocators.  
//      + loads ``spindle_config.h'' which sets various platform specific flags.
//      + loads a platform specific ``stopwatch'' class.  
// 

#ifndef SPINDLE_H_
#define SPINDLE_H_

//
// Make sure this is compiled with C++
//
#ifndef __cplusplus
#error This is a C++ header file
#endif

//
// Verify that SPINDLE_TARGET is defined
// if not, default to std.

#ifndef SPINDLE_TARGET
//#error macro SPINDLE_TARGET is not defined
#define SPINDLE_TARGET std
#endif


// load site specific configuration 
#ifndef SPINDLE_CONFIG_H_
#include "spindle_config.h"
#endif 

//
// Set Allocator.  
//
#ifdef HAVE_ALLOC_H_
#    if ( SPINDLE_TARGET == std )
#    	 include "alloc.h"
#    elif ( SPINDLE_TARGET == standard )
#    	 include "alloc.h"
#    elif ( SPINDLE_TARGET == matlab )
#    	 include "interfaces/matlab/include/mexalloc.h"
#    elif ( SPINDLE_TARGET == petsc )
#    	 include "interfaces/petsc/include/petscalloc.h"
#    else   // assume SPINDLE_TARGET == std
#    	 include "alloc.h"
#    endif
#else
#    include <memory>
#endif 

#ifndef HAVE_BOOL
   typedef int bool;
#  define true 1
#  define false 0
#endif
 
#ifndef HAVE_TYPENAME
#  define typename
#endif
 
#ifndef HAVE_EXPLICIT
#  define explicit
#endif
 
#ifndef HAVE_MUTABLE
#  define mutable
#  define constExceptWhenMissingMutable
#else
#  define constExceptWhenMissingMutable const
#endif

#ifdef HAVE_NAMESPACES
#  define SPINDLE_NAMESPACE spindle
#  define SPINDLE_BEGIN_NAMESPACE namespace spindle { 
#  define SPINDLE_END_NAMESPACE }
#else
#  define SPINDLE_NAMESPACE  //
#  define SPINDLE_BEGIN_NAMESPACE //
#  define SPINDLE_END_NAMESPACE //
#endif

#include "spindle/stopwatch.h"

#define SPINDLE__NO_INSTANCE_COUNT

#endif
