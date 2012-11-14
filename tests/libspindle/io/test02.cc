//
// src/io/drivers/tests/test02.cc
//
//  $Id: test02.cc,v 1.2 2000/02/18 01:32:08 kumfert Exp $
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
// test drives the HarwellBoeingFile class by  opening an printing a
// bunch of files
//


#include "spindle/spindle.h"

#include "spindle/HarwellBoeingFile.h"

#include <iostream.h>

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

main() {
  cout.setf(ios::stdio); // flushes stdio appropriately
  HarwellBoeingFile InputFile;

  fprintf(stdout, "********** input/template.rua.hb.gz **********\n" ); 
  InputFile.open("input/template.rua.hb.gz","r");
  InputFile.read();
  InputFile.close();
  InputFile.write(stdout);

  fprintf(stdout, "********** input/smallex.rua.hb.gz **********\n" );
  InputFile.open("input/smallex.rua.hb.gz","r");
  InputFile.read();
  InputFile.close();
  fprintf(stdout, "********** NOTE:  This is input as an explicitly symmetric matrix\n");
  fprintf(stdout, "**********        which does not conform to the Harwell-Boeing standard\n");
  fprintf(stdout, "**********        so it is actually written with the symmetry implicit.\n");
  InputFile.write(stdout);


  fprintf(stdout, "********** input/pores_1.rua.hb.gz **********\n" ); 
  InputFile.open("input/pores_1.rua.hb.gz","r");
  InputFile.read();
  InputFile.close();
  InputFile.write(stdout);

  fprintf(stdout, "********** input/2rhs.rua.hb.gz **********\n" ); 
  InputFile.open("input/2rhs.rua.hb.gz","r");
  InputFile.read();
  InputFile.close();
  InputFile.write(stdout);
}
  
