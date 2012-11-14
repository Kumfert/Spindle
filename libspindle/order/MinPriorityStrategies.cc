//
// MinPriorityStrategies.cc -- 
//
// $Id: MinPriorityStrategies.cc,v 1.2 2000/02/18 01:31:53 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1997-1998, Old Dominion University.  All rights reserved.
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

#include "spindle/MinPriorityStrategies.h"

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include <math.h>  // needed for sqrt
#else
#include <cmath>
using std::sqrt;
#endif


#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

#ifndef LARGE_INT
#  ifndef MAX_INT
#    define LARGE_INT 50000
#  else 
#    define LARGE_INT MAX_INT
#  endif
#endif

MinPriorityStrategies::MinPriorityStrategies() {
  visited = 0;
  size = -1;
  lastStamp = 0;
  timer.reset();
}

MinPriorityStrategies::MinPriorityStrategies( const int Size ) {
  visited = 0;
  size = -1;
  lastStamp = 0;
  setSize( Size );
  timer.reset();
}

int 
MinPriorityStrategies::reduceRangeFromNSquaredToN( const double priority ) const {
  
  int iPriority = (int) priority;
  
  if (iPriority <= 0) { 
    return 0;
  } else if ( priority <= halfN ) {
    return iPriority;
  } else { 
    int score;
    score = ((int) sqrt( priority - halfN ));
    score += ((int) halfN );
    score = ( score >= size ) ? size-1 : score ;
    return score;
  }
}

bool
MinPriorityStrategies::setSize( const int Size ) {
  if ( Size <= 0 ) { 
    return false;
  }
  size = Size;
  halfN = Size/2;
  vec.reserve( Size );
  vec.resize(0);
  visited = vec.begin();
  lastStamp = 0;
  vec.insert( vec.begin(), Size, 0 );
  return true;
}

int
MinPriorityStrategies::getStamp() {
  if (lastStamp >= LARGE_INT ) {
    vec.resize(0);
    vec.insert( vec.begin(), size, 0 );
    lastStamp=1;
    visited = vec.begin();
  } else {
    ++lastStamp;
  }
  return lastStamp;
}

bool
MinPriority_ExactDegree::prioritize( const VertexList& reachableVtxs, 
					const QuotientGraph* g, 
					BucketSorter* degStruct ) {
  timer.start();
  const int *weight = g->getWeightArray();
  const int *extdeg = g->getExternDegreeArray();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, 
	 "\n\tMinPriority_ExactDegree::prioritize(){ // (vtx,degree)\n\t\t" );
  for( VertexList::const_iterator it=reachableVtxs.begin(), stop=reachableVtxs.end();
       it != stop; ++it ) {
    int vtx = *it;
    if ( g->isEliminated( vtx ) ) { 
      WARNING("Cannot compute priority of eliminated node %d.  Skipping.",vtx);
      continue;
    }
    int priority = extdeg[vtx] + weight[vtx];
    TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " (%d,%d) ", vtx, priority );
    degStruct->insert( priority, vtx );
  } // end for all reachable vtxs
  timer.stop();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " \n\t}" );
  return true;
}

bool
MinPriority_ExternalDegree::prioritize( const VertexList& reachableVtxs, 
					const QuotientGraph* g, 
					BucketSorter* degStruct ) {
  timer.start();
#ifdef DEBUG_MINPRIORITY_STRATEGIES
  const int *weight = g->getWeightArray();
#endif
  bool dumpgraph = false;
  const int *extdeg = g->getExternDegreeArray();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, 
	 "\n\tMinPriority_ExternalDegree::prioritize(){ // (vtx,degree)\n\t\t" );
  for( VertexList::const_iterator it=reachableVtxs.begin(), stop=reachableVtxs.end();
       it != stop; ++it ) {
    int vtx = *it;
    if ( g->isEliminated( vtx ) ) { 
      WARNING("Cannot compute priority of eliminated node %d.  Skipping.",vtx);
      continue;
    }
    int priority = extdeg[vtx];
#ifdef DEBUG_MINPRIORITY_STRATEGIES
    int stamp = getStamp();
    int my_priority = 0;
    visited[ vtx ] = stamp;
    for ( const int *e = g->enode_begin(vtx), *stop_e = g->enode_end(vtx); e != stop_e; ++e) { 
      for ( const int *s = g->snode_begin(*e), *stop_s = g->snode_end(*e); s != stop_s; ++s) { 
	if ( visited[ *s ] < stamp ) { 
	  visited[ *s ] = stamp;
	  my_priority += weight[ *s ];
	}
      }
    }
    if ( priority != my_priority ) { 
      cout << "Warning:  priority = " << priority << " but computed a priority = "
	   << my_priority << " for vertex " << vtx << endl;
      dumpgraph = true;
    }
#endif
    TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " (%d,%d) ", vtx, priority );
    degStruct->insert( priority, vtx );
  } // end for all reachable vtxs
  timer.stop();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " \n\t}" );
  if ( dumpgraph ) g->printTinyGraph( cout );
  return true;
}

bool
MinPriority_ApproximateDegree::prioritize( const VertexList& reachableVtxs, 
					      const QuotientGraph* g, 
					      BucketSorter* degStruct ) {
  timer.start();
#ifdef DEBUG_MINPRIORITY_STRATEGIES
  const int *setDiff = g->getSetDiffsArray();
  const int *weight = g->getWeightArray();
#endif
  const int *extdeg = g->getExternDegreeArray();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, 
	 "\n\tMinPriority_ApproximateDegree::prioritize(){ // (vtx,degree)\n\t\t" );
  for( VertexList::const_iterator it=reachableVtxs.begin(), stop=reachableVtxs.end();
       it != stop; ++it ) {
    int vtx = *it;
    if ( g->isEliminated( vtx ) ) { 
      WARNING("Cannot compute priority of eliminated node %d.  Skipping.",vtx);
      continue;
    }
    int priority = extdeg[vtx];
#ifdef DEBUG_MINPRIORITY_STRATEGIES
    int my_priority = 0;
    int my_weight = weight[vtx];
    for ( const int* e= g->enode_begin( vtx), *stop_e = g->enode_end(vtx); e != stop_e; ++e ) { 
      if ( *e != vtx ) { 
	my_priority += setDiff[ *e ];
      }
    }
    if ( my_priority > 0 ) { 
      my_priority -= my_weight;
    }
    for ( const int *s = g->snode_begin(vtx), *stop_s = g->snode_end(vtx); s != stop_s; ++s ) { 
      my_priority += weight[ *s ];
    }
    if ( priority != my_priority ) { 
      cout << "Warning:  priority = " << priority << " but computed a priority = "
	   << my_priority << " for vertex " << vtx << endl;
      g->printTinyGraph( cout );
    }
#endif
    TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " (%d,%d) ", vtx, priority );
    degStruct->insert( priority, vtx );
  } // end for all reachable vtxs
  timer.stop();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " \n\t}" );
  return true;
}


//
// Rothberg '97
//
// score(i) = (d^2 - d ) /2 - (c^2 - c ) /2 
// where d = ext deg(i) = external degree of node i
//       c = |C_l \ weight(i)| = where C_l is the largest adjacent clique
bool
MinPriority_AMF::prioritize( const VertexList& reachableVtxs, 
			     const QuotientGraph* g, 
			     BucketSorter* degStruct ) {
  // 1. initialization
  timer.start();
  const int *weight = g->getWeightArray();
  const int *extdeg = g->getExternDegreeArray();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, 
	 "\n\tMinPriority_AMF::prioritize(){ // (vtx,degree)\n\t\t" );
  // 2. for each vertex in reachable set.
  for( VertexList::const_iterator it=reachableVtxs.begin(), stop=reachableVtxs.end();
       it != stop; ++it ) { 
    int vtx = *it;
    // 2.a. skip already eliminated vtxs (this is an error)
    if ( g->isEliminated( vtx ) ) { 
      WARNING("Cannot compute priority of eliminated node %d.  Skipping.",vtx);
      continue;
    }
    // 2.b. find the adjacent clq of maximum size 
    int max_clq = 0;
    {for( const int *curClq = g->enode_begin(vtx), *stopClq = g->enode_end(vtx); 
	  curClq < stopClq; ++curClq ) {  // for all adjacent cliques...
      int clq = *curClq;
      if ( clq != vtx ) {                 //  .... excluding self 
	int clqWeight = weight[ clq ];
	max_clq = ( clqWeight > max_clq ) ? clqWeight : max_clq;
      }
    }}
    // 2.c compute score
    double score; 
    { 
      double d = (double) extdeg[ vtx ];  // d = the external degree of the node
      if ( max_clq > 0 ) { 
	// c = the size of the largest adjacent clique
	double c = (double) max_clq - weight[ vtx ]; // c = |C_l \ v |
	score = ( d * d - d ) / 2 - ( c * c - c ) / 2;
      } else { 
	score = ( d * d - d ) / 2;
      }
    }
    // 2.d. insert into degStruct
    int priority = reduceRangeFromNSquaredToN( score );
    TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " (%d,%d) ", vtx, priority );
    degStruct->insert( priority, vtx );
  } // end for all reachable vtxs
  // 3. cleanup
  timer.stop();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " \n\t}" );
  return true;
}

//
// Rothberg and Eisenstat
//
// score(i) = [ (d^2 - d ) - (c^2 - c ) ] / ( 2 *  weight(i) )
// where d = ext deg(i) = external degree of node i
//       c = |C_l \ weight(i)| = where C_l is most recently created clique
bool
MinPriority_AMMF::prioritize( const VertexList& reachableVtxs, 
			      const QuotientGraph* g, 
			      BucketSorter* degStruct ) {
  // 1. initialization
  timer.start();
  const int *weight = g->getWeightArray();
  const int *extdeg = g->getExternDegreeArray();
  
  int C_l = g->queryLastEliminatedNode(); // last eliminated node
  int max_clq = ( C_l != -1 ) ? weight[ C_l ] : 0 ; 
  // weight of last eliminated node, if it exists

  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, 
	 "\n\tMinPriority_AMMF::prioritize(){ // (vtx,degree)\n\t\t" );
  // 2. for each vertex in the reachable set
  for( VertexList::const_iterator it=reachableVtxs.begin(), stop=reachableVtxs.end();
       it != stop; ++it ) {
    int vtx = *it;
    // 2.a. skip already eliminated vtxs (an error if they occur)
    if ( g->isEliminated( vtx ) ) { 
      WARNING("Cannot compute priority of eliminated node %d.  Skipping.",vtx);
      continue;
    }
    // 2.b compute score
    double score; 
    { 
      int my_weight = weight[ vtx ];
      double d = (double) extdeg[ vtx ];  // d = the external degree of the node
      if ( max_clq > 0 ) { 
	// c = the size of the largest adjacent clique
	double c = (double) max_clq - my_weight; // c = |C_l \ v |
	score = ( ( d * d - d ) - ( c * c - c ) ) / ( 2 * my_weight );
      } else { 
	score = ( d * d - d ) / ( 2 * my_weight ) ;
      }
    }
    int priority = reduceRangeFromNSquaredToN( score );
    TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " (%d,%d) ", vtx, priority );
    degStruct->insert( priority, vtx );
  } // end for all reachable vtxs
  // 3. cleanup
  timer.stop();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " \n\t}" );
  return true;
}

//
// score(i) = [ (d^2 - d ) - (c^2 - c ) ] /  2  - ( extdeg(i) * weight(i) )
// where d = ext deg(i) = external degree of node i
//       c = |C_l \ weight(i)| = where C_l is most recently created clique
bool
MinPriority_AMIND::prioritize( const VertexList& reachableVtxs, 
				const QuotientGraph* g, 
				BucketSorter* degStruct ) {
  // 1. initialization
  timer.start();
  const int *weight = g->getWeightArray();
  const int *extdeg = g->getExternDegreeArray();
  int C_l = g->queryLastEliminatedNode(); // last eliminated node
  int max_clq = ( C_l != -1 ) ? weight[ C_l ] : 0 ; /* weight of last eliminated node, 
						     * if it exists
						     */
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, 
	 "\n\tMinPriority_AMIND::prioritize(){ // (vtx,degree)\n\t\t" );

  // 2. for each vertex in the reachable set.
  for( VertexList::const_iterator it=reachableVtxs.begin(), stop=reachableVtxs.end();
       it != stop; ++it ) {
    int vtx = *it;
    // 2.a. skip already eliminated vtxs (an error if they occur)
    if ( g->isEliminated( vtx ) ) { 
      WARNING("Cannot compute priority of eliminated node %d.  Skipping.",vtx);
      continue;
    }
    // 2.b compute score
    double score; 
    { 
      int my_weight = weight[ vtx ];
      double d = (double) extdeg[ vtx ];  // d = the external degree of the node
      if ( max_clq > 0 ) { 
	// c = the size of the largest adjacent clique
	double c = (double) max_clq - my_weight; // c = |C_l \ v |
	score = ( ( d * d - d ) - ( c * c - c ) ) / 2  - (d * my_weight);
      } else { 
	score = ( d * d - d ) / 2  - ( d * my_weight ) ;
      }
    }  
    // 2.c. insert into queue
    int priority = reduceRangeFromNSquaredToN( score );
    TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " (%d,%d) ", vtx, priority );
    degStruct->insert( priority, vtx );
  } // end for all reachable vtxs
  // 3. cleanup
  timer.stop();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " \n\t}" );
  return true;
}

bool
MinPriority_MMMF::prioritize( const VertexList& reachableVtxs, 
			      const QuotientGraph* g, 
			      BucketSorter* degStruct ) {
  // 1. initialization
  timer.start();
  const int *weight = g->getWeightArray();
  const int *extdeg = g->getExternDegreeArray();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, 
	 "\n\tMinPriority_MAMMF::prioritize(){ // (vtx,degree)\n\t\t" );
  // 2. for all vertices in reachable set
  for( VertexList::const_iterator it=reachableVtxs.begin(), stop=reachableVtxs.end();
       it != stop; ++it ) {
    int vtx = *it;
    //  2.a. skip already eliminated vtxs (an error if they occur)
    if ( g->isEliminated( vtx ) ) { 
      WARNING("Cannot compute priority of eliminated node %d.  Skipping.",vtx);
      continue;
    }
    //  2.b. find largest clique
    int max_clq = 0;
    {for( const int *curClq = g->enode_begin(vtx), *stopClq = g->enode_end(vtx); 
	  curClq < stopClq; ++curClq ) { // for all adjacent cliques ...
      int clq = *curClq;
      if ( clq != vtx ) {                //  .... except self 
	int clqWeight = weight[ clq ];
	max_clq = ( clqWeight > max_clq ) ? clqWeight : max_clq;
      }
    }}
    // 2.c compute score
    double score; 
    { 
      int my_weight = weight[ vtx ];
      double d = (double) extdeg[ vtx ];  // d = the external degree of the node
      if ( max_clq > 0 ) { 
	// c = the size of the largest adjacent clique
	double c = (double) max_clq - my_weight; // c = |C_max \ v |
	score = ( ( d * d - d ) - ( c * c - c ) ) / ( 2 * my_weight );
      } else { 
	score = ( d * d - d ) / ( 2 * my_weight ) ;
      }
    }
    // 2.c. insert new priority into queue
    int priority = reduceRangeFromNSquaredToN( score );
    TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " (%d,%d) ", vtx, priority );
    degStruct->insert( priority, vtx );
  } // end for all reachable vtxs
  // 3. cleanup
  timer.stop();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " \n\t}" );
  return true;
}

bool
MinPriority_MMIND::prioritize( const VertexList& reachableVtxs, 
				const QuotientGraph* g, 
				BucketSorter* degStruct ) {
  // 1. initialization
  timer.start();
  const int *weight = g->getWeightArray();
  const int *extdeg = g->getExternDegreeArray();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, 
	 "\n\tMinPriority_MAMIND::prioritize(){ // (vtx,degree)\n\t\t" );

  // 2. for each vertex in the reachable set.
  for( VertexList::const_iterator it=reachableVtxs.begin(), stop=reachableVtxs.end();
       it != stop; ++it ) {
    int vtx = *it;
    // 2.a. skip if already eliminated (which is an error)
    if ( g->isEliminated( vtx ) ) { 
      WARNING("Cannot compute priority of eliminated node %d.  Skipping.",vtx);
      continue;
    }
    // 2.b. find largest clique
    int max_clq = 0;
    {for( const int *curClq = g->enode_begin(vtx), *stopClq = g->enode_end(vtx); 
	  curClq < stopClq; ++curClq ) { // for all adjacent cliques ...
      int clq = *curClq;
      if ( clq != vtx ) {                // ... excluding self 
	int clqWeight = weight[ clq ];
	max_clq = ( clqWeight > max_clq ) ? clqWeight : max_clq;
      }
    }}
    // 2.c compute score
    double score; 
    { 
      int my_weight = weight[ vtx ];
      double d = (double) extdeg[ vtx ];  // d = the external degree of the node
      if ( max_clq > 0 ) { 
	// c = the size of the largest adjacent clique
	double c = (double) max_clq - my_weight; // c = |C_max \ v |
	score = ( ( d * d - d ) - ( c * c - c ) ) / 2  - (d * my_weight);
      } else { 
	score = ( d * d - d ) / 2  - ( d * my_weight ) ;
      }
    }  
    // 2.d. insert into degStruct
    int priority = reduceRangeFromNSquaredToN( score );
    TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " (%d,%d) ", vtx, priority );
    degStruct->insert( priority, vtx );
  } // end for all reachable vtxs
  // 3. cleanup
  timer.stop();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " \n\t}" );
  return true;
}



//
//  Ng Raghavan 97
//
// score(i) = (d^2 - d) - C - ct
// where d = ext deg(i) = external degree of node i
//       C = let K be set of partial cliques.
//           C = \sum_{v \in K} weight(v)^2 - weight(v)
//       ct = 2 * extdeg( i) * weight(i)
bool
MinPriority_MMDF::prioritize( const VertexList& reachableVtxs, 
			      const QuotientGraph* g, 
			      BucketSorter* degStruct ) {
  timer.start();
  const int *weight = g->getWeightArray();
  const int *extdeg = g->getExternDegreeArray();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, 
	 "\n\tMinPriority_MMDF::prioritize(){ // (vtx,degree)\n\t\t" );
  for( VertexList::const_iterator it=reachableVtxs.begin(), stop=reachableVtxs.end();
       it != stop; ++it ) {
    int vtx = *it;
    if ( g->isEliminated( vtx ) ) { 
      WARNING("Cannot compute priority of eliminated node %d.  Skipping.",vtx);
      continue;
    }
    int degree = extdeg[vtx];
    double clq_fill = 0.0;
    int stamp = getStamp();
    visited[vtx] = stamp;  // keep from revisiting self
    {for( const int* curVtx = g->snode_begin(vtx), *stopVtx = g->snode_end(vtx); 
	  curVtx < stopVtx; ++curVtx ) {
      // flag all snodes adj to me
      visited[ *curVtx ] = stamp;
      int adj = *curVtx;
      if ( visited[ adj ] < stamp ) {  // if not yet visited
	visited[ adj ] = stamp;        // visit now
	// degree += weight[ adj ];       // and add weight of adj to current vtx
      }
    }}
    {for( const int *curClq = g->enode_begin(vtx), *stopClq = g->enode_end(vtx); 
	  curClq < stopClq; ++curClq ) {
      // for all adjacent cliques ( including self )
      int clq = *curClq;
      double this_clq_fill = 0.0;
      if ( *curClq != vtx ) {
	for( const int* curVtx = g->snode_begin(clq), *stopVtx = g->snode_end(clq); 
	     curVtx < stopVtx; ++curVtx ) {
	  // for all vertices in each clique
	  int adj = *curVtx;
	  if ( visited[ adj ] < stamp ) {  // if not yet visited
	    visited[ adj ] = stamp;        // visit now
	    // degree += weight[ adj ];       // and add weight of adj to current vtx
	    this_clq_fill += (double) weight[ adj ];
	  }
	}
	if ( this_clq_fill > 0.0 ) {
	  clq_fill += this_clq_fill * ( this_clq_fill - 1 );
	}
      } // end else
    }}
    double score = (double) degree;
    score = ( score * ( score - 1.0 )) - clq_fill - ( 2*degree*weight[vtx]) ;
    int priority = reduceRangeFromNSquaredToN( score );
    TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " (%d,%d) ", vtx, priority );
    degStruct->insert( priority, vtx );
  } // end for all reachable vtxs
  timer.stop();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " \n\t}" );
  return true;
}

//
// priority(v_k) = 2 * extdeg( v_k ) - max clique v_k is a member of
//
bool
MinPriority_MMMD::prioritize( const VertexList& reachableVtxs, 
				 const QuotientGraph* g, 
				 BucketSorter* degStruct ) {
  timer.start();
  const int *weight = g->getWeightArray();
  const int *extdeg = g->getExternDegreeArray();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, 
	 "\n\tMinPriority_MMMD::prioritize(){ // (vtx,degree)\n\t\t" );
  for( VertexList::const_iterator it=reachableVtxs.begin(), stop=reachableVtxs.end();
       it != stop; ++it ) {
    int vtx = *it;
    if ( g->isEliminated( vtx ) ) { 
      WARNING("Cannot compute priority of eliminated node %d.  Skipping.",vtx);
      continue;
    }
    int degree = extdeg[ vtx ];
    //int clq_fill = weight[ g->queryLastEliminatedNode() ];
    int clq_fill = 0;
    int stamp = getStamp();
    visited[vtx] = stamp;  // keep from revisiting self
    {for( const int *curClq = g->enode_begin(vtx), *stopClq = g->enode_end(vtx); 
	  curClq < stopClq; ++curClq ) {
      // for all adjacent cliques ( including self )
      int clq = *curClq;
      if ( clq == vtx ) { continue; } // skip self clique
      int this_clq_fill = weight[ clq ];
      clq_fill = ( this_clq_fill > clq_fill ) ? this_clq_fill : clq_fill ;
    }}
    int score = 2*degree - clq_fill;
    int priority = reduceRangeFromNSquaredToN( score );
    TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " (%d,%d) ", vtx, priority );
    degStruct->insert( priority, vtx );
  } // end for all reachable vtxs
  timer.stop();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " \n\t}" );
  return true;
}


//
// originally mmd until tol% of vertices have been eliminated, then amd
//
MinPriority_MMD_AMD_SimpleHybrid::MinPriority_MMD_AMD_SimpleHybrid() {
  multiple_elimination = true;
  switch_bound = 0.5;
}

MinPriority_MMD_AMD_SimpleHybrid::MinPriority_MMD_AMD_SimpleHybrid( const int Size, float switch_bnd ) 
  : MinPriorityStrategies( Size ) { 
    multiple_elimination = true;
    switch_bound = switch_bnd;
}

MinPriority_MMD_AMD_SimpleHybrid::~MinPriority_MMD_AMD_SimpleHybrid() {}

bool
MinPriority_MMD_AMD_SimpleHybrid::requireSetDiffs() { 
  return ! multiple_elimination;
}

bool
MinPriority_MMD_AMD_SimpleHybrid::prioritize( const VertexList& reachableVtxs, 
					      const QuotientGraph* g, 
					      BucketSorter* degStruct ) {
  timer.start();
  if ( multiple_elimination ) {
    //  const int *weight = g->getWeightArray();
    const int *extdeg = g->getExternDegreeArray();
    TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, 
	   "\n\tMinPriority_MMD_AMD_SimpleHybrid::prioritize(){ // (vtx,degree)\n\t\t" );
    for( VertexList::const_iterator it=reachableVtxs.begin(), stop=reachableVtxs.end();
	 it != stop; ++it ) {
      int vtx = *it;
      if ( g->isEliminated( vtx ) ) { 
	WARNING("Cannot compute priority of eliminated node %d.  Skipping.",vtx);
	continue;
      }
      int priority = extdeg[vtx];
      TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " (%d,%d) ", vtx, priority );
      degStruct->insert( priority, vtx );
    } // end for all reachable vtxs
  } else { 
    //  const int *weight = g->getWeightArray();
    const int *extdeg = g->getExternDegreeArray();
    TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, 
	   "\n\tMinPriority_ApproximateDegree::prioritize(){ // (vtx,degree)\n\t\t" );
    for( VertexList::const_iterator it=reachableVtxs.begin(), stop=reachableVtxs.end();
	 it != stop; ++it ) {
      int vtx = *it;
      if ( g->isEliminated( vtx ) ) { 
	WARNING("Cannot compute priority of eliminated node %d.  Skipping.",vtx);
	continue;
      }
      int priority = extdeg[vtx];
      TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " (%d,%d) ", vtx, priority );
      degStruct->insert( priority, vtx );
    } // end for all reachable vtxs
  }
  if ( multiple_elimination ) {
    if ( g->queryNEliminated() >= ( switch_bound * size ) ) {
      multiple_elimination = false;
    }
  }
  timer.stop();
  TRACE( SPINDLE_TRACE_MINPRIORITY_SCHEME, " \n\t}" );
  return true;
}



//
// 
//
MinPriority_AdvancedHybrid::MinPriority_AdvancedHybrid() {
  useStrategy1 = true;
  strategy1 = 0;
  strategy2 = 0;
  suboptimalMultipleEliminationSize = 5;
  maxSuboptimalMultipleEliminations = 5;
  nSuboptimalMultipleEliminations = 0;
  runtime.reset();
  runtime.start();
}

MinPriority_AdvancedHybrid::MinPriority_AdvancedHybrid( const int Size, const int tol, const int n ) 
  : MinPriorityStrategies( Size ) { 
  useStrategy1 = true;
  strategy1 = 0;
  strategy2 = 0;
  suboptimalMultipleEliminationSize = tol;
  if ( ( maxSuboptimalMultipleEliminations = n ) <= 0 ) { 
    useStrategy1 = false;
  }
  nSuboptimalMultipleEliminations = -1;

  runtime.reset();
  runtime.start();
}

MinPriority_AdvancedHybrid::~MinPriority_AdvancedHybrid() {
  runtime.stop();
  if ( strategy1 != 0 ) { 
    delete strategy1;
  } 
  if ( strategy2 != 0 ) { 
    delete strategy2; 
  }
}

bool
MinPriority_AdvancedHybrid::requireSetDiffs() { 
  return ( useStrategy1 ) ? false : true ;
}

bool
MinPriority_AdvancedHybrid::prioritize( const VertexList& reachableVtxs, 
					const QuotientGraph* g, 
					BucketSorter* degStruct ) {
  if ( nSuboptimalMultipleEliminations == -1 ) { 
    if ( strategy1 == 0 ) { 
      strategy1 = new MinPriority_ExternalDegree( size );
    } else { 
      strategy1->setSize( size );
    }
    if ( strategy2 == 0 ) { 
      strategy2 = new MinPriority_ApproximateDegree( size );
    } else { 
      strategy2->setSize( size );
    }
    nSuboptimalMultipleEliminations = 0;
  }
  
  timer.start();
  bool success;
  
  if ( useStrategy1 ) { 
    success =  strategy1->prioritize( reachableVtxs, g, degStruct);
  } else { 
    success =  strategy2->prioritize( reachableVtxs, g, degStruct);
  }
  
  // Note can only change strategy after the update and 
  // reprioritization.  Otherwise, the quotient graph is not
  // guaranteed to have precomputed some of the values.

  int totEliminated = g->queryTotEliminated(); // total snodes eliminated
  int nCompressed = g->queryNCompressed();     // # compressed, uneliminated
  int nPrincipalSnodes = size - totEliminated - nCompressed;
  int nEliminatedThisStage = g->queryNEliminatedLastStage();
  runtime.stop();
  double secs = ((double) runtime.queryTotalUserTicks() + runtime.queryTotalSystemTicks()) 
    / runtime.queryTicksPerSec();
    runtime.start();
  TRACE( SPINDLE_TRACE_MINPRIORITY_STRATEGY_ADVHYBRID, 
	 "nEliminatedThisStage=%d nPrincipalSnodes=%d  time=%e", 
	 nEliminatedThisStage, nPrincipalSnodes, secs );
  if ( useStrategy1 ) { 
    if ( nEliminatedThisStage < suboptimalMultipleEliminationSize ) { 
      if ( ++nSuboptimalMultipleEliminations > maxSuboptimalMultipleEliminations ) { 
	useStrategy1 = false;
      }
    } else { 
      nSuboptimalMultipleEliminations = 0;
    }
    TRACE( SPINDLE_TRACE_MINPRIORITY_STRATEGY_ADVHYBRID, "   tol=%d   nsubopt=%d\n", 
	   suboptimalMultipleEliminationSize, nSuboptimalMultipleEliminations );
    if ( nSuboptimalMultipleEliminations >= maxSuboptimalMultipleEliminations ) { 
      useStrategy1 = false;
    }
  } else { 
    TRACE( SPINDLE_TRACE_MINPRIORITY_STRATEGY_ADVHYBRID, "\n" );
  }
    
  timer.stop();
  return success;
}
 
bool 
MinPriority_AdvancedHybrid::
setMultipleEliminationStrategy( MinPriority_MultipleElimination * strategy ) { 
  if ( strategy == 0 ) { 
    return false;
  } 
  if ( strategy == strategy1 ) { 
    return true;
  }
  if ( strategy1 != 0 ) { 
    delete strategy1;
  }
  strategy1 = strategy;
  return true;
}

bool 
MinPriority_AdvancedHybrid::
setApproximateEliminationStrategy( MinPriority_ApproximateElimination * strategy ) {
  if ( strategy == 0 ) { 
    return false;
  } 
  if ( strategy == strategy2 ) { 
    return true;
  }
  if ( strategy2 != 0 ) { 
    delete strategy2;
  }
  strategy2 = strategy;
  return true;
}
 


//
// 
//
MinPriority_AdvancedHybrid2::MinPriority_AdvancedHybrid2() {
  useStrategy1 = true;
  strategy1 = 0;
  strategy2 = 0;
  minTolerance = .1;
  slidingAvg = 0;
  curAvg = 1.0;
  curIdx = -1;
  historySize = -1;
}

MinPriority_AdvancedHybrid2::MinPriority_AdvancedHybrid2( const int Size, const float tol, const int n ) 
  : MinPriorityStrategies( Size ) { 
  useStrategy1 = true;
  strategy1 = 0;
  strategy2 = 0;
  minTolerance = tol;
  slidingAvg = new float[n];
  curAvg = 1.0;
  curIdx = -1;
  historySize = n;
}

MinPriority_AdvancedHybrid2::~MinPriority_AdvancedHybrid2() {
  if ( strategy1 != 0 ) { 
    delete strategy1;
  } 
  if ( strategy2 != 0 ) { 
    delete strategy2; 
  }
  if ( slidingAvg != 0 ) { 
    delete[] slidingAvg;
  }
}

bool
MinPriority_AdvancedHybrid2::requireSetDiffs() { 
  return ( useStrategy1 ) ? false : true ;
}

bool
MinPriority_AdvancedHybrid2::prioritize( const VertexList& reachableVtxs, 
					const QuotientGraph* g, 
					BucketSorter* degStruct ) {
  if ( curIdx == -1 ) { 
    if ( strategy1 == 0 ) { 
      strategy1 = new MinPriority_ExternalDegree( size );
    } else { 
      strategy1->setSize( size );
    }
    if ( strategy2 == 0 ) { 
      strategy2 = new MinPriority_ApproximateDegree( size );
    } else { 
      strategy2->setSize( size );
    }
  }
  
  timer.start();
  bool success;
  
  if ( useStrategy1 ) { 
    success =  strategy1->prioritize( reachableVtxs, g, degStruct);
  } else { 
    success =  strategy2->prioritize( reachableVtxs, g, degStruct);
  }
  
  // Note can only change strategy after the update and 
  // reprioritization.  Otherwise, the quotient graph is not
  // guaranteed to have precomputed some of the values.

  int totEliminated = g->queryTotEliminated(); // total snodes eliminated
  int nCompressed = g->queryNCompressed();     // # compressed, uneliminated
  int nPrincipalSnodes = size - totEliminated - nCompressed;
  int nEliminatedThisStage = g->queryNEliminatedLastStage();
  int nReachableVtxs = reachableVtxs.size();
  TRACE( SPINDLE_TRACE_MINPRIORITY_STRATEGY_ADVHYBRID, 
	 "nEliminatedThisStage=%3d nPrincipalSnodes=%3d nReachableVtxs=%3d  ", 
	 nEliminatedThisStage, nPrincipalSnodes, nReachableVtxs );
  if ( useStrategy1 && ( nEliminatedThisStage != 0 ) ) { 
    float currentRatio = ( (float) nEliminatedThisStage ) / nReachableVtxs;
    if ( curIdx < 0 ) { 
      // haven't filled up the sliding window yet.
      int realIdx = -(curIdx + 1);
      slidingAvg[ realIdx ] = currentRatio/historySize;
      --curIdx;
      if ( -(curIdx+1) == historySize ) {  // if we've filled up the sliding average
	TRACE( SPINDLE_TRACE_MINPRIORITY_STRATEGY_ADVHYBRID, " SWITCHED " );
	curIdx = 0;
	curAvg = 0.0;
	for ( int i=0; i<historySize; ++i ) { 
	  curAvg += slidingAvg[i];
	}
      }
    }
    if ( curIdx >= 0 ) {
      curAvg -= slidingAvg[ curIdx ];
      slidingAvg[curIdx] = currentRatio/historySize;
      curAvg += slidingAvg[ curIdx ];
      ++curIdx;
      curIdx = curIdx % historySize;
    }
    if ( curAvg < minTolerance ) { 
      useStrategy1 = false;
      TRACE( SPINDLE_TRACE_MINPRIORITY_STRATEGY_ADVHYBRID, " SWITCHED " );
    } 
    TRACE( SPINDLE_TRACE_MINPRIORITY_STRATEGY_ADVHYBRID, " curRatio=%e curAvg=%e  tol=%e \n", 
	   currentRatio, curAvg, minTolerance );
  } else { 
    TRACE( SPINDLE_TRACE_MINPRIORITY_STRATEGY_ADVHYBRID, "\n" );
  }
    
  timer.stop();
  return success;
}
 
bool 
MinPriority_AdvancedHybrid2::
setMultipleEliminationStrategy( MinPriority_MultipleElimination * strategy ) { 
  if ( strategy == 0 ) { 
    return false;
  } 
  if ( strategy == strategy1 ) { 
    return true;
  }
  if ( strategy1 != 0 ) { 
    delete strategy1;
  }
  strategy1 = strategy;
  return true;
}

bool 
MinPriority_AdvancedHybrid2::
setApproximateEliminationStrategy( MinPriority_ApproximateElimination * strategy ) {
  if ( strategy == 0 ) { 
    return false;
  } 
  if ( strategy == strategy2 ) { 
    return true;
  }
  if ( strategy2 != 0 ) { 
    delete strategy2;
  }
  strategy2 = strategy;
  return true;
}
 


//
// 
//
MinPriority_TimedHybrid::MinPriority_TimedHybrid() {
  useStrategy1 = true;
  strategy1 = 0;
  strategy2 = 0;
  currentSlope = -1.0; // flag, slopes won't be negative
  initialSlope = -1.0; // flag, slopes won't be negative
  slopeChange = 0.01;  // switch when slope reduces by 1/4.
}

MinPriority_TimedHybrid::MinPriority_TimedHybrid( const int Size, const float tol)
  : MinPriorityStrategies( Size ) { 
  useStrategy1 = true;
  strategy1 = 0;
  strategy2 = 0;
  currentSlope = -1.0; // flag, slopes won't be negative
  initialSlope = -1.0; // flag, slopes won't be negative
  if ( tol < 1.0 && tol > 0.0 ) { 
    slopeChange = tol;  // switch when slope reduces by 1/4.
  } else { 
    slopeChange = 0.25;
  }
}

MinPriority_TimedHybrid::~MinPriority_TimedHybrid() {
  if ( strategy1 != 0 ) { 
    delete strategy1;
  } 
  if ( strategy2 != 0 ) { 
    delete strategy2; 
  }
}

bool
MinPriority_TimedHybrid::requireSetDiffs() { 
  return ( useStrategy1 ) ? false : true ;
}

bool
MinPriority_TimedHybrid::prioritize( const VertexList& reachableVtxs, 
					const QuotientGraph* g, 
					BucketSorter* degStruct ) {
  if ( strategy1 == 0 ) { 
    strategy1 = new MinPriority_ExternalDegree( size );
  } else { 
    strategy1->setSize( size );
  }
  if ( strategy2 == 0 ) { 
    strategy2 = new MinPriority_ApproximateDegree( size );
  } else { 
    strategy2->setSize( size );
  }
   
  timer.start();
  bool success;
  
  if ( useStrategy1 ) { 
    success =  strategy1->prioritize( reachableVtxs, g, degStruct);
  } else { 
    success =  strategy2->prioritize( reachableVtxs, g, degStruct);
  }
  
  // Note can only change strategy after the update and 
  // reprioritization.  Otherwise, the quotient graph is not
  // guaranteed to have precomputed some of the values.

  if ( !useStrategy1) { 
    return success;
  } 

  int delta_x = g->queryNEliminatedLastStage();
  stopwatch::ticks  delta_y =  ( interval.queryLapUserTicks() + interval.queryLapSystemTicks() + 1) ;
  if ( !interval.stop() ) { // first time.
    //TRACE( SPINDLE_TRACE_MINPRIORITY_STRATEGY_ADVHYBRID, "1)\n");
  } else if ( useStrategy1 ) { 
    currentSlope = (float) delta_x / delta_y;
    if ( delta_y == 0 ) { // no measureable time elapsed.
      // special case, if last elimination took no measureable time, then don't bother
      /*TRACE( SPINDLE_TRACE_MINPRIORITY_STRATEGY_ADVHYBRID, 
	     "%d) (0 time) nEliminatedThisStage=%3d, elapsed ticks = %3d, "
	     "currentSlope = %g, targetSlope = %g\n",
	     interval.queryNLaps(), g->queryNEliminatedLastStage(), delta_y, 
	     currentSlope, initialSlope*slopeChange )*/;
    } else if ( initialSlope <= 0.0 ) {  // no positive slope yet.
      if ( currentSlope > 0.0 ) { 
	initialSlope = currentSlope;
      }
      /*TRACE( SPINDLE_TRACE_MINPRIORITY_STRATEGY_ADVHYBRID, 
	     "%d) (set initialSlope) nEliminatedThisStage = %3d, elapsed ticks = %3d, "
	     "initialSlope = %g, targetSlope = %g\n",
	     interval.queryNLaps(), g->queryNEliminatedLastStage(), delta_y, 
	     initialSlope, initialSlope*slopeChange );*/
    } else if ( currentSlope < initialSlope * slopeChange ) { 
      useStrategy1 = false;
      /*TRACE( SPINDLE_TRACE_MINPRIORITY_STRATEGY_ADVHYBRID, 
	     "%d) (SWITCHED) nEliminatedThisStage=%3d, elapsed ticks = %3d, "
	     "currentSlope = %g, targetSlope = %g\n",
	     interval.queryNLaps(), g->queryNEliminatedLastStage(), delta_y, 
	     currentSlope, initialSlope*slopeChange );*/
    } /*else { 
      TRACE( SPINDLE_TRACE_MINPRIORITY_STRATEGY_ADVHYBRID, 
	     "%d) nEliminatedThisStage=%3d, elapsed ticks = %3d, "
	     "currentSlope = %g, targetSlope = %g\n",
	     interval.queryNLaps(), g->queryNEliminatedLastStage(), delta_y, 
	     currentSlope, initialSlope*slopeChange );
	     }   */   
  } /*else { 
    TRACE( SPINDLE_TRACE_MINPRIORITY_STRATEGY_ADVHYBRID, 
	   "%d) (SWITCHED) nEliminatedThisStage=%3d, elapsed ticks = %3d,"
	   "currentSlope = %g, targetSlope = %g\n",
	   interval.queryNLaps(), g->queryNEliminatedLastStage(), delta_y, 
	   currentSlope, initialSlope*slopeChange );
	   } */
  timer.stop();
  interval.start();
  return success;
}
 
bool 
MinPriority_TimedHybrid::
setMultipleEliminationStrategy( MinPriority_MultipleElimination * strategy ) { 
  if ( strategy == 0 ) { 
    return false;
  } 
  if ( strategy == strategy1 ) { 
    return true;
  }
  if ( strategy1 != 0 ) { 
    delete strategy1;
  }
  strategy1 = strategy;
  return true;
}

bool 
MinPriority_TimedHybrid::
setApproximateEliminationStrategy( MinPriority_ApproximateElimination * strategy ) {
  if ( strategy == 0 ) { 
    return false;
  } 
  if ( strategy == strategy2 ) { 
    return true;
  }
  if ( strategy2 != 0 ) { 
    delete strategy2;
  }
  strategy2 = strategy;
  return true;
}

