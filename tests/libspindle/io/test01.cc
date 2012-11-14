//
// src/io/drivers/tests/test01.cc
//
//  $Id: test01.cc,v 1.2 2000/02/18 01:32:08 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1998, Old Dominion University.  All rights reserved.
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
//  tests ChacoFile
//

#include "spindle/spindle.h"

#include "spindle/ChacoFile.h"

#include <iostream.h>

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

main() {
  ChacoFile InputFile;
  cout.sync_with_stdio();
  cout << "************* testMatrix.chac ***********" << endl;
  InputFile.open("input/testMatrix.chac","r");
  InputFile.read();
  InputFile.close();
  InputFile.write(stdout);
  InputFile.open("tempout/testMatrix1.chac","w");
  InputFile.write();
  InputFile.close();


  cout << "************* testMatrix.chac.Z ***********" << endl;
  InputFile.open("input/testMatrix.chac.Z","r");
  InputFile.read();
  InputFile.close();
  InputFile.write(stdout);
  InputFile.open("tempout/testMatrix2.chac.Z","w");
  InputFile.write();
  InputFile.close();


  cout << "************* testMatrix.chac.gz ***********" << endl;
  InputFile.open("input/testMatrix.chac.gz","r");
  InputFile.read();
  InputFile.close();
  InputFile.write(stdout);
  InputFile.open("tempout/testMatrix3.chac.gz","w");
  InputFile.write();
  InputFile.close();


  cout << "************* testMatrix.chac.zip ***********" << endl;
  cout << "****** NOTE: cannot write a zipped file ******" << endl;
  InputFile.open("input/testMatrix.chac.zip","r");
  InputFile.read();
  InputFile.close();
  InputFile.write(stdout);
}
  
