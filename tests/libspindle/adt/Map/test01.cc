//
// src/adt/Map/drivers/tests/test01.cc -- tests PermutationMap
//
//  $Id: test01.cc,v 1.2 2000/02/18 01:32:07 kumfert Exp $
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
//


#include "spindle/PermutationMap.h"

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#define REGISTRY SpindleSystem::registry()

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main() {
  SpindleSystem::registry().setDumpOnDestroy( stdout );
  SpindleSystem::setErrorFile( stdout );

  int perm_array[] = { 3, 4, 2, 0, 1};
  PermutationMap perm(5);
  perm.getNew2Old().take( perm_array, 5 );
  perm.validate();


  cout << "Size of PermutationMap class = " << sizeof(PermutationMap) << endl;
  
  const ClassMetaData* metaData = REGISTRY.findClass("PermutationMap");
  
  cout << "According to the registry it is = " << metaData->classSize << endl;
  cout << "This perm object is serial #" << perm.queryIDNumber() << endl;

  PermutationMap perm2(8);
  cout << "The second PermutationMap object has serial #" << perm2.queryIDNumber() << endl;

}
