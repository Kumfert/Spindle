//
// src/io/drivers/tests/test05.ccn -- tests the ChristensenFile class
//
//  $Id: test06b.cc,v 1.2 2000/02/18 01:32:08 kumfert Exp $
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
// test drives the ChristensenFile class
//


#include "spindle/spindle.h"

#include "spindle/ChristensenFile.h"

#include <iostream.h>

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

main() {
  cout.setf(ios::stdio); // flushes stdio appropriately
  ChristensenFile InputFile;

  fprintf(stdout, "********** input/slask_2.dat **********\n" ); 
  fprintf(stdout, "*** NOTE: Christensen files do not necessarily have edges sorted\n" );
  fprintf(stdout, "***       Therefore the resulting file written out is *not*\n");
  fprintf(stdout, "***       identical to input\n");
  fprintf(stdout, "*** NOTE: Furthermore, Christensen files may have gaps in the sequence\n");
  fprintf(stdout, "***       These gaps are not preserved after reading in the file.\n");
  InputFile.open("input/slask_2.dat","r");
  InputFile.read();
  InputFile.close();
  InputFile.write(stdout);
}
  
