//
// src/io/drivers/tests/test08.cc -- tests the MatrixMarketFile class
//
//  $Id: test08.cc,v 1.2 2000/02/18 01:32:09 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1999, Old Dominion University.  All rights reserved.
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
// test drives the MatrixMarketFile class
//


#include "spindle/spindle.h"

#include "spindle/MatrixMarketFile.h"
#include "spindle/Graph.h"

#include <iostream.h>

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main() {
  cout.setf(ios::stdio); // flushes stdio appropriately

  fprintf(stdout, "********** input/simple.coord **********\n" ); 
  {
    MatrixMarketFile InputFile;
    InputFile.open("input/simple.coord","rb");
    InputFile.read();
    InputFile.close();
    
    Graph g ( &InputFile);
    g.validate();
    if ( !g.isValid() ) { 
      cerr << "Test08.exe Error: cannot create valid graph" << endl;
    }
    g.prettyPrint( stdout );
  }
 fprintf(stdout, "********** input/simple.coord **********\n" ); 
  {
    MatrixMarketFile InputFile;
    InputFile.open("input/simple.matrixmarket","rb");
    InputFile.read();
    InputFile.close();
    
    Graph g ( &InputFile);
    g.validate();
    if ( !g.isValid() ) { 
      cerr << "Test08.exe Error: cannot create valid graph" << endl;
    }
    g.prettyPrint( stdout );
  }
}
  


