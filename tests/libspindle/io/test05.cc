//
// src/io/drivers/tests/test05.ccn -- tests the DobrianFile class
//
//  $Id: test05.cc,v 1.2 2000/02/18 01:32:08 kumfert Exp $
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
// test drives the DobrianFile class
//


#include "spindle/spindle.h"

#include "spindle/DobrianFile.h"

#include <iostream.h>

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

main() {
  cout.setf(ios::stdio); // flushes stdio appropriately
  DobrianFile InputFile;

  fprintf(stdout, "********** input/bcsstk01.ld.gz **********\n" ); 
  InputFile.open("input/bcsstk01.ld.gz","r");
  InputFile.read();
  InputFile.close();
  InputFile.write(stdout);
}
  
