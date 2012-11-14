//
// src/adt/Map/drivers/tests/test03.cc -- test MapUtils
//
//  $Id: test03.cc,v 1.2 2000/02/18 01:32:07 kumfert Exp $
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


#include "spindle/CompressionMap.h"
#include "spindle/PermutationMap.h"
#include "spindle/MapUtils.h"

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#include <iostream.h>
#include <iomanip.h>

#define REGISTRY SpindleSystem::registry()

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main() {
  ios::sync_with_stdio();
  SpindleSystem::registry().setDumpOnDestroy( stdout );
  SpindleSystem::setErrorFile( stdout );

  int comp_array[] = { 0, 0, 1, 0, 1, 1, 2, 3, 4, 4, 3, 4 };
  CompressionMap cmap(12,5);
  cmap.getFine2Coarse().import( comp_array, 12 );
  cmap.validate();
  if ( ! cmap.isValid() ) { 
    cerr << "WARNING: cannot create valid CompressionMap." << endl;
  }

  int perm_array[] = { 2, 3, 0, 1, 4 }; // old2new ordering by minimum supernode weight
  PermutationMap perm(5);
  perm.getNew2Old().import( perm_array, 5);
  perm.validate();
  if ( ! perm.isValid() ) {
    cerr << "WARNING: cannot create valid PermutationMap." << endl;
  }

  PermutationMap* newPerm = MapUtils::createUncompressedPermutationMap( &perm, &cmap );
  if ( (newPerm == 0) || ( !newPerm->isValid() ) ) { 
    cerr << "ERROR: could not create an uncompressed permutation" << endl;
  } else {

    cout << "This new permutation object is serial #" << newPerm->queryIDNumber() << endl;
    
    cout << "Dumping the uncompressed PermutationMap { " << endl << "  ";
    newPerm->dump( stdout );
    cout << endl << "}" << endl;
    
    delete newPerm;
  }

}
