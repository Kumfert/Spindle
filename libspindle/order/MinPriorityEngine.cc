//
// MinPriorityEngine.cc
//
// $Id: MinPriorityEngine.cc,v 1.2 2000/02/18 01:31:53 kumfert Exp $
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

#include "spindle/MinPriorityEngine.h"
#include <algo.h>

#ifndef SPINDLE_MIN_PRIORITY_STRATEGIES_H_
#include "spindle/MinPriorityStrategies.h"
#endif

#ifndef SPINDLE_ARRAY_BUCKET_SORTER_H_
#include "spindle/ArrayBucketSorter.h"
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

#ifdef __FUNC__
#undef __FUNC__
#endif


#undef DEBUG_TRACE
#define DEBUG_TRACE 0
//#endif

bool
MinPriorityEngine::setDelta( const int i ) {
  if ( (i<0) || (i>n) ) { 
    return false; 
  } else { 
    delta = i; 
    return true;
  } 
}

#define __FUNC__ "MinPriorityEngine::MinPriorityEngine()"
MinPriorityEngine::MinPriorityEngine() {
  incrementInstanceCount( MinPriorityEngine::MetaData );
  randomizeGraph = true;
  qgraph = 0;
  degStruct = 0;
  priorityStrategy = 0;
  explicitOrder = 0;
  workVec = 0;
  reset();
}
#undef __FUNC__

#define __FUNC__ "MinPriorityEngine::MinPriorityEngine( const Graph* graph )"
MinPriorityEngine::MinPriorityEngine( const Graph* graph ) {
  FENTER;
  incrementInstanceCount( MinPriorityEngine::MetaData );
  randomizeGraph = true;
  qgraph = 0;
  degStruct = 0;
  priorityStrategy = 0;
  // workVec = 0;
  explicitOrder = 0;
  reset();
  setGraph( graph );
  FEXIT;
}
#undef __FUNC__

bool 
MinPriorityEngine::setHeavyVertexTolerance( float tolerance ) {
  if ( tolerance <= 0.0 ) { return false; }
  if ( tolerance >  1.0 ) { return false; }
  heavyVtxTolerance = tolerance;
  return true;
}

#define __FUNC__ "bool MinPriorityEngine::setGraph( const Graph* graph )"
bool
MinPriorityEngine::setGraph( const Graph* graph ) {
  FENTER;
  FCALL OrderingAlgorithm::setGraph( graph );
  FRETURN( true );
}
#undef __FUNC__

// #define __FUNC__ "MinPriorityEngine::MinPriorityEngine( const int nvtxs, const int* AdjHead, const int* AdjList )"
// MinPriorityEngine::MinPriorityEngine( const int nvtxs, const int* AdjHead, const int* AdjList ) {
//   FENTER;
//   n = nvtxs;
//   FCALL qgraph = new DavisGraph( n, AdjHead , AdjList );
//   FCALL degStruct = new ArrayBucketSorter(n,n);
//   priorityStrategy = new MinPriority_ApproximateDegree();
//   delayWeight = 1;
//   delta = 0; 
//   maxSteps = n;
//   timer.reset();
//   degreeComputationTimer.reset();
//   workVec = new int[n];
//   FEXIT;
// }
// #undef __FUNC__

MinPriorityEngine::~MinPriorityEngine() {
  delete qgraph;
  delete degStruct;
  if ( priorityStrategy != 0 ) {
    delete priorityStrategy;
  }
  if ( explicitOrder != 0 ) { 
    delete explicitOrder;
  }
  explicitOrder = 0;
  //  delete[] workVec;
}

#define __FUNC__ "void MinPriorityEngine::reset()"
bool  
MinPriorityEngine::reset() {
  FENTER;
  //  FCALL qgraph->reset();
  heavyVtxTolerance = 0.8;
  //  delayWeight = 1;
  delta = 0; 
  timer.reset();
  stageTimer.reset();
  stepTimer.reset();
  degreeComputationTimer.reset();
  algorithmicState = EMPTY;
  if (explicitOrder != 0 ) { 
    delete[] explicitOrder;
    explicitOrder = 0 ;
  }
  FRETURN( OrderingAlgorithm::reset() );
}
#undef __FUNC__

#define __FUNC__ "bool MinPriorityEngine::setPriorityStrategy( MinPriorityStrategies* strategy )"
bool MinPriorityEngine::setPriorityStrategy( MinPriorityStrategies* strategy ) { 
  FENTER;
  if ( algorithmicState == INVALID ) { return false; }
  if ( (strategy != 0) && (strategy != priorityStrategy) ) { //if strategy is not NULL and not current strategy
    if ( priorityStrategy != 0 ) {
      delete priorityStrategy;  // get rid of current strategy
    }
    priorityStrategy = strategy;
    FRETURN( true );
  }
  FRETURN( false );
}
#undef __FUNC__

bool MinPriorityEngine::setExplicitOrder( const PermutationMap& perm ) { 
  if ( notValid()) { return false; }
  if ( notReady()) { return false; }
  if ( !perm.isValid() ) { return false; }
  if ( perm.size() != n ) { return false; }

  // now copy the explicit ordering
  if ( explicitOrder != 0 ) { 
    delete[] explicitOrder;
  }
  explicitOrder = new int[ n ];
  const int * new2old = perm.getNew2Old().lend();
  copy( new2old, new2old + n, explicitOrder );
  return true;
}


#define __FUNC__ "bool MinPriorityEngine::execute()"
bool MinPriorityEngine::execute() {
  FENTER;
  if ( algorithmicState != READY ) { return false; }
  
  ///////////////////////////////////////////////////
  //
  // 1. if this is the first run for this graph, then
  // there are some additional details to take care of.
  //
  if ( qgraph == 0 ) { 
    //
    // 1.a assign the either the compressed or uncompressed
    //     graph.
    //
    TRACE( SPINDLE_TRACE_DEBUG, "Creating Quotient Graph..." );
    if ( useGraphCompression ) { 
      qgraph = new QuotientGraph( coarseGraph );
      n = coarseGraph->queryNVtxs();
    } else { 
      qgraph = new QuotientGraph( fineGraph );
      n = fineGraph->queryNVtxs();
    }
    
    if ( degStruct != 0 ) {
      delete degStruct;
    }
    TRACE( SPINDLE_TRACE_DEBUG, "Creating BucketSorter...");
    degStruct = new BucketSorter(n,n);
    
    if ( explicitOrder != 0 ) { 
      delete[] explicitOrder;
    }
    explicitOrder=0;
    
    reachableVtxs.reserve(n);
    mergedVtxs.reserve(n);
  } // end 2

  timer.start(); 

  ////////////////////////////////
  //
  // 3. per run initialization
  //
  if ( !initialize() ) { 
    algorithmicState = INVALID;
    return false;
  }

  /////////////////////
  //
  // 4. Do the actual elimination
  //
  if ( !run() ) {  // the main loop is here
       algorithmicState = INVALID;
    return false;
  } 

  /////////////////////
  //
  // Do some post-elimination cleanup.
  //
  if( !finalize() ) {
    algorithmicState = INVALID;
    return false;
  }
  timer.stop();
  
  const int * old2New;
  if ( (old2New = qgraph->getOld2New()) != 0 ) { 
    algorithmicState = DONE;
    return OrderingAlgorithm::recomputePermutations(0,old2New);
  }
  FRETURN( true );
}

bool
MinPriorityEngine::initialize() {
  // First set up the priority strategy if one is 
  // not specified.
  if( priorityStrategy == 0 ) {
    priorityStrategy = new MinPriority_ExternalDegree( n );
  } 
  if ( priorityStrategy->requireSetDiffs() ) { 
    // set differences are  L_e / L_i , where i is last eliminated vtx.
    // this is required for approximate minimum strategys.
    qgraph->enableSingleElimination();  // enables setDiffs and disables outmatching
    maxSteps = 1;                       // prevents multiple elimination
  } else { 
    maxSteps = n;
  }

  // get the degStruct to insert new nodes in the back
  // Struct->setInsertBack();   //  degStruct->print( cout );

  // Now load up the degStruct.  
  // We must determine which vertices are to be considered.
  // If the set of vertices to number is not explicit, choose
  // all the yet-unnumbered ones.
  
  bool numberExplicitSet = false; // HACK: will be more intelligent test later.
  if ( numberExplicitSet ) {
    // put explicit set in
  } else { 
    // find all vertices whose status is still principalVtx
    VertexList temp;
    int maxDeg = (int)(heavyVtxTolerance * n);
    if ( n < 1000 ) {
      maxDeg =  n+1; // if less than 1000 vertices, forget about heavy vtx removal
    } 
    if ( explicitOrder == 0 ) { 
      for(int i=0; i<n; ++i) { 
	if ( qgraph->isPrincipal(i) && ( !qgraph->isEliminated(i) ) ) {
	  int curDeg = qgraph->snode_end(i) - qgraph->snode_begin(i);
	  if ( curDeg <= maxDeg ) {   // if within tolerance
	    temp.push_back( i );      //    add it to list
	  } else {                    // else... get ready to purge from the qgraph
	    purgedVtxs.push_back( pair<int,int>( curDeg, i ) ); 
	  }
	}
      }
    } else { 
      for(int ii=0; ii<n; ++ii) { 
	int i = explicitOrder[ii];
	if ( qgraph->isPrincipal(i) && ( !qgraph->isEliminated(i) ) ) {
	  int curDeg = qgraph->deg(i); // get current degree of vtx
	  if ( curDeg <= maxDeg ) {   // if within tolerance
	    temp.push_back( i );      //    add it to list
	  } else {                    // else... get ready to purge from the qgraph
	    purgedVtxs.push_back( pair<int,int>( curDeg, i ) ); 
	  }
	}
      }
    }
    //    if ( purgedVtxs.size() > 0 ) { 
    //      qgraph->purgeVtxs( purgedVtxs ); 
    //      // qgraph will eliminate these only after everything else
    //      purgedVtxs.resize( 0 );
    //    }

    // now suffle non-purged vtxs randomly 
    if ( ( explicitOrder == 0 ) && randomizeGraph ) { 
      random_shuffle( temp.begin(), temp.end() );
    }

    // insert them into the degree Structure
    priorityStrategy->prioritize( temp, qgraph, degStruct );
  }

  // If we have delayed vertices, remove these from the degreeStructure
  //  if ( delayedVtxs.size() > 0 ) { 
  //    for(VertexList::const_iterator i=delayedVtxs.begin(), stop=delayedVtxs.end();
  //	i != stop; ++i ) {
  //      degStruct->remove( *i );
  //    }  
  //  }
  
  return true;
}


bool
MinPriorityEngine::run() {
  //  VertexList eliminatedVtxs(n);
  ////  VertexList eligibleVtxs(n);
  //  int *ip = workVec;
  //  {for (int i=0; i<n; ++i,++ip ) { *ip = -1; }}

  // Now eliminate in stages until we run out of vertices in degStruct
  for(int stage = 0; ((! degStruct->isEmpty()) && (stage<n)); stage++) {
    stageTimer.start();  
    if (DEBUG_TRACE) { cout << "stage " << stage << endl; }

    // do a stage of multiple elimination
    // qgraph->printFullGraph(cout);
    int minDeg = degStruct->queryMinBucket();
    int tolerance = minDeg + delta + 1;

#ifndef NDEBUG
    { 
      int nEligible = degStruct->queryNInserted();
      int totEliminated = qgraph->queryTotEliminated();
      int nCompressed = qgraph->queryNCompressed();
      int nOutmatched = qgraph->queryNOutmatched();
      if ( nEligible + totEliminated + nCompressed + nOutmatched != n ) {
	ERROR(0,"nEligible(%d) + totEliminated(%d) + nCompressed(%d) + nOutmatched(%d) = %d != n(%d)",
	      nEligible, totEliminated, nCompressed, nOutmatched, 
	      nEligible+ totEliminated+ nCompressed+ nOutmatched, 
	      n );
	int countEligible = 0;
	int countEliminated = 0;
	int countCompressed = 0;
	int countOutmatched = 0;
	for ( int i=0; i<n; ++i ) { 
	  int isEligible = ( degStruct->queryKey( i ) != -1 ) ? 1 : 0 ;
	  int isEliminated = ( qgraph->eliminatedAt( i ) != -1 ) ? 1 : 0 ;
	  int isOutmatched = ( qgraph->queryParent( i ) < 0 ) ? 1 : 0 ;
	  int isCompressed = ( ( qgraph->queryParent( i ) != i ) && (!isOutmatched) && (!isEliminated) )
	    ? 1 : 0;
	  int state = isEligible + isEliminated + isOutmatched + isCompressed;
	  if ( state != 1 ) { 
	    ERROR(0,"   Node %d has state (isEligible,isEliminated,isCompressed,isOutmatched) (%d,%d,%d,%d)",
		  i,isEligible,isEliminated,isCompressed,isOutmatched);
	  }
	  countEligible += isEligible;
	  countEliminated += isEliminated;
	  countCompressed += isCompressed;
	  countOutmatched += isOutmatched;
	}
	if ( nEligible != countEligible ) { 
	  ERROR(0,"nEligible(%d) != countEligible(%d)", nEligible, countEligible );
	}
	if ( totEliminated != countEliminated ) { 
	  ERROR(0,"totEliminated(%d) != countEliminated(%d)", totEliminated, countEliminated );
	}
	if ( nCompressed != countCompressed ) { 
	  ERROR(0,"nCompressed(%d) != countCompressed(%d)", nCompressed, countCompressed );
	}
	if ( nOutmatched != countOutmatched ) { 
	  ERROR(0,"nOutmatched(%d) != countOutmatched(%d)", nOutmatched, countOutmatched );
	}
      }
    }
#endif

    // While there exists a vertex within tolerance
    ////      for (int step=0; ( eligibleVtxs.size() > 0 ) && ( step < maxSteps ); ++step ) {
    for (int step=0; ( ( ! degStruct->isEmpty() )
		       && ( degStruct->queryMinBucket() < tolerance )
		       && (  step < maxSteps ) 
		     ); 
	 ++ step ){
      stepTimer.start();
      // remove the vertex of minimum degree
      // degStruct->print(cout);
      int current = degStruct->queryFirstItem(minDeg);
      // this next block handles possible errors in state info for degStruct
      if (current == -1 ) {                     // if current bucket is, in fact, empty
	minDeg = degStruct->queryMinBucket();   // get first non-empty bucket
	if ( minDeg == -1 ) {                   //     if no non-empty buckets, update
	  break;        
	} else {                                // else
	  --step;                               //     reset the step counter
	  continue;                             //     try again.
	}
      }
      degStruct->remove(current);
      // eliminate it from the graph
      if (DEBUG_TRACE) cout << "   step  "  << step << " principal node=" 
			    << current << "   reachable nodes = " << flush;
      if ( qgraph->eliminateSupernode(current) ) { // vertex elimination succeeds.
	// remove its reachable set from consideration
	for( const int* cur=qgraph->snode_begin(current), *stop=qgraph->snode_end(current);
	     cur < stop; ++cur ) {
	  if (DEBUG_TRACE) cout << *cur << ", ";
	  degStruct->remove(*cur);
	}
      }
      // qgraph->printFullGraph(cout);
      if (DEBUG_TRACE) cout << endl;
    stepTimer.stop();
    } // end steps

    qgraph->update( reachableVtxs, mergedVtxs );
    if ( DEBUG_TRACE )  qgraph->printShortGraph( cout );
    // qgraph->printSupernodes( cout );

    if( reachableVtxs.size() > 0) {
      // Add the reachable vertices back
      degreeComputationTimer.start();
      // random_shuffle( reachableVtxs.begin(), reachableVtxs.end() ); // not necessary
      priorityStrategy->prioritize( reachableVtxs, qgraph, degStruct );
      degreeComputationTimer.stop();
      // reachableVtxs.resize(0);
    }

    //if ( delayedVtxs.size() > 0 ) {
    //      delayedVtxsTimer.start();
    //      processDelayedVtxs();
    //      delayedVtxsTimer.stop();
    //    }

    stageTimer.stop();
    if ( ( maxSteps != 1 ) && priorityStrategy->requireSetDiffs() ) {
      // set differences are  L_e / L_i , where i is last eliminated vtx.
      // this is required for approximate minimum strategys.
      qgraph->enableSingleElimination();  // enables setDiffs and disables outmatching
      maxSteps = 1;                    // prevents multiple elimination
      int totEliminated = qgraph->queryTotEliminated(); // total snodes eliminated
      int nCompressed = qgraph->queryNCompressed();     // # compressed, uneliminated
      double secs = (double(stageTimer.queryTotalUserTicks())
		     +stageTimer.queryTotalSystemTicks())/
	stageTimer.queryTicksPerSec();
      TRACE(SPINDLE_TRACE_MINPRIORITY_STRATEGY_SWITCH, 
	    "Switched stage=%d at time=%g, nsteps=%d, totUneliminated=%d, "
	    "nPrincipalSnode=%d\n",
	    stage, secs, stepTimer.queryNLaps(), 
	    n-totEliminated, n-totEliminated-nCompressed);
    } else if ( ( maxSteps == 1 ) && !(priorityStrategy->requireSetDiffs() ) ){ 
      qgraph->enableMultipleElimination();
      maxSteps = n;
      int totEliminated = qgraph->queryTotEliminated(); // total snodes eliminated
      int nCompressed = qgraph->queryNCompressed();     // # compressed, uneliminated 
      double secs = (double(stageTimer.queryTotalUserTicks())
		     +stageTimer.queryTotalSystemTicks())/
	stageTimer.queryTicksPerSec();
      TRACE(SPINDLE_TRACE_MINPRIORITY_STRATEGY_SWITCH, 
	    "Switched stage=%d at time=%g, nsteps=%d, totUneliminated=%d,"
	    "nPrincipalSnode=%d\n",
	    stage, secs, stepTimer.queryNLaps(), 
	    n-totEliminated, n-totEliminated-nCompressed);
    }
    
    if (DEBUG_TRACE) cout << endl;
  } // end stage-loop
  stageTimer -= delayedVtxsTimer;
  stageTimer -= degreeComputationTimer;
  stageTimer -= stepTimer;
  return true;
}

bool
MinPriorityEngine::finalize() {
  algorithmicState = DONE;
  return true;
}

/*
bool 
MinPriorityEngine::enableDelayedVertices( const VertexList& delayedVertices ) {
  if ( algorithmicState == INVALID ) { return false; }
  if ( delayedVtxs.size() != 0 ) { return false; }
  delayedVtxs.reserve( delayedVertices.size() );
  for( VertexList::const_iterator i = delayedVertices.begin(), 
	 stop = delayedVertices.end(); i != stop; ++i ){
   delayedVtxs.push_back( (*i) - 1 );
  }
  if ( delayedVtxs.size() > 0 ) {
    sort( delayedVtxs.begin(), delayedVtxs.end() );
  }
 return true;
}

bool 
MinPriorityEngine::disableDelayedVertices() {
  if ( algorithmicState == INVALID ) { return false; }
  if ( delayedVtxs.size() == 0 ) { return false; }
  delayedVtxs.erase( delayedVtxs.begin(), delayedVtxs.end() );
  return true;
}
*/

/*
SharedPtr<PermutationMap> 
MinPriorityEngine::getPermutationMap() const {
  // At this point, we're all done.  Just fill in old2new and new2old.
  //  if ( n == nVertices) {
  PermutationMap *perm = new PermutationMap( n );
  int* old2new  =  perm->getOld2New().give();
  for (int i=0; i<n; i++) {
    int p = qgraph->eliminatedAt(i);
    //    if (DEBUG_TRACE) cout << "old2new[" << i << "] = " << p << endl;
    old2new[i] = p;
  }
  perm->getOld2New().take( old2new , n );
  perm->validate();
  return SharedPtr<PermutationMap>( perm );
}
*/

/*
void 
MinPriorityEngine::processDelayedVtxs() {
  if ( mergedVtxs.size() > 0 ) {
    // If a delayed vertex is merged, its parent must be delayed
    sort( mergedVtxs.begin(), mergedVtxs.end() );
    if ( workVtxList.size() > 0 ) { 
      workVtxList.erase( workVtxList.begin(), workVtxList.end() );
    }
    set_intersection( delayedVtxs.begin(), delayedVtxs.end(), 
		      mergedVtxs.begin(), mergedVtxs.end(), 
		      back_inserter(workVtxList) );
    // workVtxList now has all delayed vtxs which have been merged
    { 
      VertexList::const_iterator i=workVtxList.begin();
      VertexList::const_iterator stop_i=workVtxList.end();
      VertexList::iterator j=delayedVtxs.begin();
      VertexList::iterator stop_j=delayedVtxs.end();
      // now update the delayedVtxs to their principal snodes
      bool dirty=false;
      while ( (i!=stop_i) && (j!=stop_j) ) {
	if (*i < *j) {
	  ++i;
	} else if ( *i > *j ) {
	  ++j;
	} else {
	  *j = qgraph->queryParent(*j);
	  ++j;
	  dirty=true;
	}
      }
      if (dirty) {
	sort( delayedVtxs.begin(),delayedVtxs.end() );
	unique( delayedVtxs.begin(),delayedVtxs.end() );
      }
    }
  } // end if mergedVtxs.size()
  
  // Now all vertices in delayedVtxs are current ( i.e. not compressed out)
  if (workVtxList.size() > 0) { 
    workVtxList.erase( workVtxList.begin(), workVtxList.end() ); 
  }
  set_intersection( delayedVtxs.begin(), delayedVtxs.end(), 
		    reachableVtxs.begin(), reachableVtxs.end(),
		    back_inserter(workVtxList) );
  // workVtxList now contains all delayed vertices that were reintroduced
  
  if (( (int)workVtxList.size() >= degStruct->size() ) && (workVtxList.size() > 0) ) {
    // stop delaying vertices, they're all we have left
    cout << "Warning: " << workVtxList.size() 
	 << " Delayed vertices remained when there were only " << degStruct->size() 
	 << " vertices left to number!!!" << endl;
    delayedVtxs.erase( delayedVtxs.begin(), delayedVtxs.end() );
    return;
  }
  
  // for each vertex that is delayed...
  for( VertexList::iterator i=workVtxList.begin(), stop_i=workVtxList.end(); i!=stop_i; ++i ) {
    // ... compute its weight;
    int weight=qgraph->queryNEliminatedNbrs(*i);
    
    // ... if the weight is too small delay it longer
    if (( weight < delayWeight ) && (weight>=0) ) {
      degStruct->remove( *i );
    } else {
      //  otherwise remove it from the list of delayedVtxs.
      //  ( It should already be in the degStruct. )
      VertexList::iterator new_end = remove( delayedVtxs.begin(), delayedVtxs.end(), *i );
      if ( new_end != delayedVtxs.end() ) {
	delayedVtxs.erase( new_end, delayedVtxs.end() );
      }
    }
  } // end for each vertex that is delayed
} 
*/ 
 
/*
int
MinPriorityEngine::queryNPurgedVtxs() const { 
  return purgedVtxs.size();
 }
*/
SPINDLE_IMPLEMENT_DYNAMIC( MinPriorityEngine, SpindleAlgorithm )
