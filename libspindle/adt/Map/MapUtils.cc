//
// MapUtils.h
//
//  $Id: MapUtils.cc,v 1.2 2000/02/18 01:31:48 kumfert Exp $
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
//  purpose.  It is provided "as is" without express or implied warranty.
//
///////////////////////////////////////////////////////////////////////
//
//

#include "spindle/MapUtils.h"

#ifndef SPINDLE_PERMUTATION_H_
#include "spindle/PermutationMap.h"
#endif

#ifndef SPINDLE_COMPRESSION_MAP_H_
#include "spindle/CompressionMap.h"
#endif 

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

PermutationMap*
MapUtils::createUncompressedPermutationMap( const PermutationMap* perm, const CompressionMap* cmap ) {

  if ( (!perm->isValid()) || (!cmap->isValid()) ) { return 0; }

  const int n = cmap->queryNUncompressed();
  const int nC = cmap->queryNCompressed();
  
  if ( perm->size() != nC ) { return 0; }
  
  PermutationMap* newPerm = new PermutationMap(n); 
  int* old2new = newPerm->getOld2New().begin();
 
  const int * new2old = perm->getNew2Old().lend();
  const int * colPtr = cmap->getCoarse2Fine()->getColPtr().lend();
  const int * rowIdx = cmap->getCoarse2Fine()->getRowIdx().lend();
    
  // the ordering for the vertices in each supernode
  int nextidx = 0; 
  {for(int ii=0;ii<nC;++ii) {
    int i = new2old[ ii ];
    for(int jj=colPtr[i]; jj<colPtr[i+1]; ++jj ) {
      int j = rowIdx[jj];
      old2new[j] = nextidx++;
    }
  }}

  newPerm->validate();
  if ( newPerm->isValid() ) {
    return newPerm;
  } else {
    delete newPerm;
    return 0;
  }
}

