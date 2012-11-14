//
// src/adt/Map/drivers/tests/test01.cc -- tests Compression
//
//  $Id: test02.cc,v 1.2 2000/02/18 01:32:07 kumfert Exp $
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
  CompressionMap cmap( 12, 5 );
  cmap.getFine2Coarse().import( comp_array, 12 );
  cmap.validate();

  if ( ! cmap.isValid() ) { 
    cerr << "WARNING: cannot create valid CompressionMap." << endl;
  }

  cout << "Size of CompressionMap class = " << sizeof(CompressionMap) << endl;
  
  const ClassMetaData* metaData = REGISTRY.findClass("CompressionMap");
  
  cout << "According to the registry it is = " << metaData->classSize << endl;
  cout << "This cmap object is serial #" << cmap.queryIDNumber() << endl;
  
  cout << "Dumping the Object { " << endl << "  ";
  cmap.dump( stdout );
  cout << endl << "}" << endl;
  cout << "coarse2fine map = {" << endl;
  const int * colPtr = cmap.getCoarse2Fine()->getColPtr().lend();
  const int * rowIdx = cmap.getCoarse2Fine()->getRowIdx().lend();
  const int n = cmap.getCoarse2Fine()->getColPtr().size() - 1;
  cout << "  nCoarse = " << n << endl;
  {for ( int i = 0; i<n; ++i ) { 
    cout << "  " << setw(5) << i << ":  ";
    for ( int jj=colPtr[i]; jj<colPtr[i+1]; ++jj ) { 
      int j = rowIdx[jj];
      cout << setw(5) << j << " ";
    }
    cout << endl;
  }}
  cout << "}" << endl;

}
