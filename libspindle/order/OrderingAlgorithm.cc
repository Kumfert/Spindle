//
// OrderingAlgorith.h
//
// $Id: OrderingAlgorithm.cc,v 1.2 2000/02/18 01:31:53 kumfert Exp $
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
//

#include "spindle/OrderingAlgorithm.h"

#ifndef SPINDLE_PERMUTATION_H_
#include "spindle/PermutationMap.h"
#endif

#ifndef SPINDLE_GRAPH_H_
#include "spindle/Graph.h"
#endif

#ifndef SPINDLE_GRAPH_COMPRESSOR_H_
#include "spindle/GraphCompressor.h"
#endif

#ifndef SPINDLE_MAP_UTILS_H_
#include "spindle/MapUtils.h"
#endif

#ifndef SPINDLE_COMPRESSION_MAP_H_
#include "spindle/CompressionMap.h"
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

OrderingAlgorithm::OrderingAlgorithm() { 
  fineGraph = 0;
  coarseGraph = 0;
  compressionRatio = 0.9;
  compressor = new GraphCompressor();
  finePermutation = 0;
  coarsePermutation = 0;
  reset();
}
OrderingAlgorithm::OrderingAlgorithm(const Graph* graph) { 
  fineGraph = 0;
  coarseGraph = 0;
  compressionRatio = 0.9;
  compressor = new GraphCompressor();
  finePermutation = 0;
  coarsePermutation = 0;
  reset();
  setGraph ( graph );
}

bool
OrderingAlgorithm::reset() {
  fineGraph =0; // never delete since we never own this.
  if ( coarseGraph != 0 ) { 
    delete coarseGraph;
    coarseGraph =0;
  }
  compressionRatio = 0.9;
  compressor->reset();
  if ( finePermutation != 0 ) { 
    delete finePermutation;
    finePermutation =0;
  }
  if ( coarsePermutation != 0 ) { 
    delete coarsePermutation;
    coarsePermutation =0;
  }
  algorithmicState = EMPTY;
  return true;
}

OrderingAlgorithm::~OrderingAlgorithm() { 
  reset();
  delete compressor;
}

bool
OrderingAlgorithm::setGraph( const Graph* graph) { 
  if ( !graph || !graph->isValid() ) {
    algorithmicState = INVALID;
    return false;
  }
  if ( algorithmicState == INVALID ) { 
    return false;
  }
  fineGraph = graph;
  if ( finePermutation == 0 ) { 
    finePermutation = new PermutationMap( fineGraph->queryNVtxs() );
  } else { 
    finePermutation->resize( fineGraph->queryNVtxs() );
  }

  compressor->reset();
  compressor->setGraph( fineGraph );
  compressor->execute();

  recomputeCompression();
  algorithmicState = READY;
  return true;
}

bool OrderingAlgorithm::setCompressionRatio( const float ratio ) { 
  if ( algorithmicState == INVALID ) { return false; }
  if ( ratio > 1.0 ) { return false; }
  if ( ratio < 0.0 ) { return false; }
  if ( ratio == compressionRatio ) { return true; }
  if ( ratio > compressionRatio ) { 
    compressionRatio = ratio;
    return true;
  } 
  compressionRatio = ratio;
  recomputeCompression();
  return true;
}

void OrderingAlgorithm::recomputeCompression() { 
  float actualRatio = (float) compressor->queryCompressedNVtxs() / 
    fineGraph->queryNVtxs();
  if ( compressionRatio > 0.0 && actualRatio <= compressionRatio ) { 
    coarseGraph = compressor->createCompressedGraph();
    if ( ( coarseGraph != 0 ) && ( coarseGraph->isValid() ) ) { 
      useGraphCompression = true;
    } else { 
      useGraphCompression = false;
    }
  } else { 
    useGraphCompression = false;
  }
  if ( useGraphCompression ) { 
    if ( coarsePermutation == 0 ) { 
      coarsePermutation = new PermutationMap ( coarseGraph->queryNVtxs() );
    } else { 
      coarsePermutation->resize( coarseGraph->queryNVtxs() );
    }
  } else {
    if ( coarseGraph != 0 ) { 
      delete coarseGraph;
      coarseGraph = 0;
    }
    if ( coarsePermutation != 0 ) { 
      delete coarsePermutation;
      coarsePermutation =0;
    }
  }
}

const PermutationMap* 
OrderingAlgorithm::getPermutation() const { 
  if ( algorithmicState != DONE ) { return 0; }
  if ( ! finePermutation->isValid() ) { return 0; }
  return finePermutation;
}

PermutationMap* 
OrderingAlgorithm::givePermutation()  { 
  if ( algorithmicState != DONE ) { return 0; }
  if ( ! finePermutation->isValid() ) { return 0; }
  if ( finePermutation == 0 ) { 
    recomputePermutations();
  }
  PermutationMap * temp = finePermutation;
  finePermutation = 0;
  return temp;
}

bool OrderingAlgorithm::recomputePermutations(const int *new2old, const int *old2new) {
  if ( algorithmicState != DONE ) { return 0; }
  if ( !useGraphCompression ) { 
    if ( new2old != 0 ) { 
      finePermutation->getNew2Old().import( new2old, fineGraph->queryNVtxs() );
    } else if ( old2new != 0 ) { 
      finePermutation->getOld2New().import( old2new, fineGraph->queryNVtxs() );
    } 
    finePermutation->validate();
    if ( !finePermutation->isValid() ) { 
      algorithmicState = INVALID;
      return 0;
    }
  } else {  // if compressed, and uncompressed perm not already formed
    if ( new2old != 0 ) { 
      coarsePermutation->getNew2Old().import( new2old, coarseGraph->queryNVtxs() );
    } else if ( old2new != 0 ) { 
      coarsePermutation->getOld2New().import( old2new, coarseGraph->queryNVtxs() );
    } 
    coarsePermutation->validate();
    if ( !coarsePermutation->isValid() ) { 
      algorithmicState = INVALID;
      return 0;
    }
    CompressionMap cMap( fineGraph->queryNVtxs(), coarseGraph->queryNVtxs() );
    cMap.getFine2Coarse().import( *(compressor->getFine2Coarse()) );
    cMap.validate();
    if ( ! cMap.isValid() ) { 
      algorithmicState = INVALID;
      return 0;
    }
    if ( finePermutation != 0 ) { 
      delete finePermutation;
      finePermutation = 0;
    }
    finePermutation = MapUtils::createUncompressedPermutationMap( coarsePermutation, &cMap );
    finePermutation->validate();
    if ( !finePermutation->isValid() ) { 
      algorithmicState = INVALID;
      return 0;
    }
  }
  return finePermutation;
}


SPINDLE_IMPLEMENT_DYNAMIC( OrderingAlgorithm, SpindleAlgorithm )
