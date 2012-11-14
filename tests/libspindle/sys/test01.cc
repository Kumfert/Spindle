//
// src/sys/drivers/tests/test01.cc
//
// $Id: test01.cc,v 1.2 2000/02/18 01:32:11 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1998, Old Dominion University.  All rights reserved.
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
//  Tests the stack trace
//

#include "spindle/StackTrace.h"

#define STACK StackTrace::get()
#define SpindleFunctionEnter \
        STACK.push(__LINE__,__FILE__,__FUNC__); \
	int SPINDLE_StackSize_local_variable = STACK.size(); 


#define SpindleFunctionCall \
        STACK.setLastLine( __LINE__ );

#define SpindleFunctionReturn( RTRN );\
	STACK.resize(SPINDLE_StackSize_local_variable,__LINE__,__FILE__,__FUNC__); \
	return (RTRN); 

#define SpindleFunctionReturnVoid \
	STACK.resize(SPINDLE_StackSize_local_variable,__LINE__,__FILE__,__FUNC__); \
	return; 

void f1();
void f2();
void f3();

int g1();
int g2();
int g3();

void h1();
void h2();
void h3();

#define __FUNC__ "main(argv,argc)"
#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main() {
  SpindleFunctionEnter;

#ifdef SPINDLE__NO_INSTANCE_COUNT
  fprintf( stderr ,"WARNING: Instance counting disabled for this compilation platform\n");
#endif

  SpindleFunctionCall f1();

  SpindleFunctionCall g1();

  SpindleFunctionCall h1(); // intentionally forgot to pop one in h3

  // SpindleFunctionReturnVoid;  // intentionally forgot to pop end to test 
  ++SPINDLE_StackSize_local_variable; --SPINDLE_StackSize_local_variable;  // for when stack is destroyed. 
}

#undef __FUNC__
#define __FUNC__ "f1()"
void f1() {
  SpindleFunctionEnter;
  SpindleFunctionCall  f2();
  SpindleFunctionReturnVoid;
}


#undef __FUNC__
#define __FUNC__ "f2()"
void f2() {
  SpindleFunctionEnter;
  SpindleFunctionCall  f3();




  SpindleFunctionCall  f3();



  SpindleFunctionCall  f3();
  SpindleFunctionReturnVoid;
}


#undef __FUNC__
#define __FUNC__ "f3()"
void f3() {
  SpindleFunctionEnter;
  STACK.dump( stdout );
  SpindleFunctionReturnVoid;
}


#undef __FUNC__
#define __FUNC__ "g1()"
int g1() {
  SpindleFunctionEnter;
  SpindleFunctionCall  g2();
  SpindleFunctionReturn( 1 );
}


#undef __FUNC__
#define __FUNC__ "g2()"
int g2() {
  SpindleFunctionEnter;
  SpindleFunctionCall g3();
  SpindleFunctionReturn( 2 );
}


#undef __FUNC__
#define __FUNC__ "g3()"
int g3() {
  SpindleFunctionEnter;
  STACK.dump( stdout );
  SpindleFunctionReturn( 3 );
}


#undef __FUNC__
#define __FUNC__ "h1()"
void h1() {
  SpindleFunctionEnter;
  SpindleFunctionCall h2();
  SpindleFunctionReturnVoid;
}


#undef __FUNC__
#define __FUNC__ "h2()"
void h2() {
  SpindleFunctionEnter;
  SpindleFunctionCall h3();
  SpindleFunctionReturnVoid;
}


#undef __FUNC__
#define __FUNC__ "h3()"
void h3() {
  SpindleFunctionEnter;
  STACK.dump( stdout );
  // SpindleFunctionReturnVoid; // intentionally forgot this one
  ++SPINDLE_StackSize_local_variable; --SPINDLE_StackSize_local_variable;  // for when stack is destroyed. 
  // keeps some compilers from complaining 
  // would be used if I didn't intentionally forget
				   
}

