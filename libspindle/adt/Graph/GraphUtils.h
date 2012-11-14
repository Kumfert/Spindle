//
// GraphUtils.h
//
//  $Id: GraphUtils.h,v 1.2 2000/02/18 01:31:46 kumfert Exp $
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

#ifndef SPINDLE_GRAPH_UTILS_H_
#define SPINDLE_GRAPH_UTILS_H_

#ifndef SPINDLE_H_
#include "spindle.h"
#endif

#if defined (HAVE_NAMESPACES) && defined ( SPINDLE__NAMESPACE_DECL_BUG )
// this looks like a bug in egcs 1.1  I should only
// have to put in forward declarations of these three
// classes, but when I use namespaces, I have to include
// the headers.
# ifndef SPINDLE_SCATTER_MAP_H_
#  include "spindle/ScatterMap.h"
# endif
# ifndef SPINDLE_PERMUTATION_MAP_H_
#  include "spindle/PermutationMap.h"
# endif
# ifndef SPINDLE_GRAPH_H_
#  include "spindle/Graph.h"
# endif
#else //if def HAVE_NAMESPACES && SPINDLE__NAMESPACE_DECL_BUG
SPINDLE_BEGIN_NAMESPACE
   class ScatterMap;
   class PermutationMap;
   class Graph;
SPINDLE_END_NAMESPACE;
#endif

SPINDLE_BEGIN_NAMESPACE


class GraphUtils { 
public:
  static Graph* createPermutedGraph( const Graph* graph, const PermutationMap * perm );
  static bool createSubgraph( const Graph* originalGraph, const int * mask, 
			      Graph ** subGraph, ScatterMap ** scatterMap, 
			      bool includeGhostNodes = false );
};

SPINDLE_END_NAMESPACE

#endif
