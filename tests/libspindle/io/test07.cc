//
// src/io/drivers/tests/test07.cc -- tests the SpoolesFile class
//
//  $Id: test07.cc,v 1.2 2000/02/18 01:32:08 kumfert Exp $
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
// test drives the SpoolesFile class
//


#include "spindle/spindle.h"

#include "spindle/SpoolesFile.h"
#include "spindle/Graph.h"

#include <iostream.h>

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

main() {
  cout.setf(ios::stdio); // flushes stdio appropriately
  SpoolesFile InputFile;

  fprintf(stdout, "********** input/5x5grd5.graphb **********\n" ); 
  InputFile.open("input/5x5grid5.graphb","rb");
  InputFile.read();
  InputFile.close();
  
  Graph g ( &InputFile);
  g.validate();
  if ( !g.isValid() ) { 
    cerr << "Test07.exe Error: cannot create valid graph" << endl;
  }
  g.prettyPrint( stdout );
}
  
