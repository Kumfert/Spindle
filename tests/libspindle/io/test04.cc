//
// src/io/drivers/tests/test04.cc
//
//  $Id: test04.cc,v 1.2 2000/02/18 01:32:08 kumfert Exp $
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
// creates the same graph via a Chaco file and a Harwell Boeing File.
//


#include "spindle/spindle.h"

#include "spindle/HarwellBoeingFile.h"

#include "spindle/ChacoFile.h"

#include "spindle/Graph.h"


#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifdef __FUNC__
#undef __FUNC__
#endif

#define __FUNC__ "main()"
#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

main() {
  SpindleSystem::setErrorFile( stdout );       // set error messages to stdout
  SpindleSystem::setWarnFile( stdout );      // set warning messages to stdout

  Graph* graph1;
  Graph* graph2;

  // first create the graph by reading in a Harwell-Boeing File
  { 
    HarwellBoeingFile InputFile;
    InputFile.open("input/smallex.rua.hb.gz","r");
    InputFile.read();
    InputFile.close();
    graph1 = new Graph( &InputFile );
  }
  graph1->validate();
  if ( ! graph1->isValid() ) {
    ERROR( SPINDLE_ERROR, "Cannot Validate graph1" );
  } else { 
    fprintf( stdout, "*** Graph from Harwell-Boeing File ***\n");
    graph1->dump( stdout );
  }

  // now read in the same graph in Chaco Format
  { 
    ChacoFile InputFile;
    InputFile.open("input/smallex.chac.gz","r");
    InputFile.read();
    InputFile.close();
    graph2 = new Graph( &InputFile );
  }
  graph2->validate();
  if ( ! graph2->isValid() ) {
    ERROR( SPINDLE_ERROR, "Cannot Validate graph2" );
  } else { 
    fprintf( stdout, "*** Graph from Chaco File ***\n");
    graph2->dump( stdout );
  }

  delete graph1;
  delete graph2;
}
  
