//
// src/sys/drivers/tests/test02.cc
//
// $Id: test02.cc,v 1.2 2000/02/18 01:32:11 kumfert Exp $
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
// tests the PersistanceRegistry
//

#include "spindle/PersistanceRegistry.h"

#ifndef SPINDLE_CLASS_META_DATA_H_
#include "spindle/ClassMetaData.h"
#endif 


#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

static ClassMetaData SpindleBaseClass_MetaData =  { "SpindleBaseClass", 12, 0, 0, 0, 0 };
static ClassMetaData SpindlePersistant_MetaData = { "SpindlePersistant", 16, &SpindleBaseClass_MetaData, 0, 0, 0 };
static ClassMetaData SpindleFile_MetaData = { "SpindleFile", 20, &SpindleBaseClass_MetaData, 0, 0, 0 };
static ClassMetaData SpindleStdioFile_MetaData = { "SpindleStdioFile", 20, &SpindleFile_MetaData, 0, 0, 0 };
static ClassMetaData MatrixFile_MetaData = { "MatrixFile", 50, &SpindleStdioFile_MetaData, 0, 0, 0 };
static ClassMetaData ChacoFile_MetaData = { "ChacoFile", 50, &MatrixFile_MetaData, 0, 0, 0 };
static ClassMetaData HarwellBoeingFile_MetaData = { "HarwellBoeingFile", 80, &MatrixFile_MetaData, 0, 0, 0 };
static ClassMetaData IJValFile_MetaData = { "IJValFile", 50, &MatrixFile_MetaData, 0, 0, 0 };
static ClassMetaData MatrixMarketFile_MetaData = { "MatrixMarketFile", 50, &MatrixFile_MetaData, 0, 0, 0 };

#define REGISTRY PersistanceRegistry::get()

int main() {
  REGISTRY.registerClass( &SpindleBaseClass_MetaData );
  REGISTRY.registerClass( &SpindlePersistant_MetaData );
  REGISTRY.registerClass( &SpindleFile_MetaData );
  REGISTRY.registerClass( &SpindleStdioFile_MetaData );
  REGISTRY.registerClass( &MatrixFile_MetaData );
  REGISTRY.registerClass( &HarwellBoeingFile_MetaData );
  REGISTRY.registerClass( &MatrixMarketFile_MetaData);
  REGISTRY.registerClass( &ChacoFile_MetaData );
  REGISTRY.registerClass( &IJValFile_MetaData );
  
#ifdef SPINDLE__NO_INSTANCE_COUNT
  fprintf(stdout,"WARNING: Instance counting disabled for this compilation platform\n");
#endif
  REGISTRY.dump(stdout);

  fprintf(stdout,"So, what is the parent of \"ChacoFile\"?...\n");
  const ClassMetaData* metaData = REGISTRY.findClass("ChacoFile");
  fprintf(stdout,"\t why, its \"%s\".\n", metaData->parentMetaData->className );
  fprintf(stdout,"And how big is a HarwellBoeingFile object?\n");
  metaData = REGISTRY.findClass("HarwellBoeingFile");
  fprintf(stdout,"\t why, its %d bytes.\n", metaData->classSize );

  
}
