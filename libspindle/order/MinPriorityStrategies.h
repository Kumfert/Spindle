//
// MinPriorityStrategies.h -- 
//
// $Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $
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

#ifndef SPINDLE_MIN_PRIORITY_STRATEGIES_H_
#define SPINDLE_MIN_PRIORITY_STRATEGIES_H_

#ifndef SPINDLE_QUOTIENT_GRAPH_H_
#include "spindle/QuotientGraph.h"
#endif

#ifndef SPINDLE_MIN_PRIORITY_ORDERING_H_
#include "spindle/MinPriorityEngine.h"
#endif

/** useful for double checking ExternDegree and Approxdegree */
#undef DEBUG_MINPRIORITY_STRATEGIES

SPINDLE_BEGIN_NAMESPACE

//
// There is an abstract base class "MinPriorityStrategy".
// There are also two intermediate classes: "MinPriority_MultipleElimination" and 
//                                          "MinPriority_ApproximateElimination"
//
//
//   MinPriorityStrategy
//    |
//    +- MinPriority_MultipleElimination
//    |   |
//    |   +- MinPriority_ExactDegree
//    |   |
//    |   +- MinPriority_ExternalDegree (aka MMD)
//    |   |
//    |   +- MinPriority_AMF
//    |   |
//    |   +- MinPriority_MMMF
//    |   |
//    |   +- MinPriority_MMIND
//    |   |
//    |   +- MinPriority_MMMD
//    |   |
//    |   +- MinPriority_MMDF
//    | 
//    +- MinPriority_ApproximateElimination
//    |   |
//    |   +- MinPriority_ApproximateDegree (aka AMD)
//    |   |
//    |   +- MinPriority_AMMF
//    |   |
//    |   +- MinPriority_AMIND
//    | 
//    +- MinPriority_MMD_AMD_SimpleHybrid
//    | 
//    +- MinPriority_AdvancedHybrid
//    | 
//    +- MinPriority_AdvancedHybrid2
//
/**
 * Hello
 */

/**
 * @memo abstract base class for all Minimum Priority Ordering Strategies
 * @type class 
 *
 * @author Gary Kumfert
 * @version #$Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 * @see MinPriorityEngine
 */
class MinPriorityStrategies { 
public:

  /**
   * Datatype for storing lists of vertices
   */
  typedef QuotientGraph::VertexList            VertexList;

  /**
   * Datatype for interacting with the priority queue
   */
  typedef MinPriorityEngine::BucketSorter    BucketSorter;

protected:

  /**
   * A vector of integers equal to the size of the numbe
   */
  vector<int> vec;

  /**
   * A pointer into vec's storage for fast access.
   * Used to keep track of which vertices have been
   * visited or not.
   */
  int *visited;

  /**
   * Size of the original quotient graph, and hence
   * the maximum index of any supernode.
   */
  int size;

  /**
   * The last timestamp used.
   */
  int lastStamp;

  /**
   * A timer to measure execution time.
   */
  stopwatch timer;

  /**
   * 0.5 X size.  Use as a convenience for some
   * ordering classes.
   */
  double halfN;
  
  /**
   * Used for some strategies that compute a priority 
   * in the range of 0..n^2.  Returns an int between
   * 0..(n-1).
   */
  int reduceRangeFromNSquaredToN( const double priority ) const;

public:
  /** 
   * default constructor, requires setSize() later 
   */
  MinPriorityStrategies();

  /** 
   * convenience constructor 
   */
  MinPriorityStrategies( const int Size );
  
  /** 
   * virtual destructor 
   */
  virtual ~MinPriorityStrategies() {}

  /** 
   * set the size of the quotient graph 
   */
  bool setSize( const int Size );
  
  /**
   * return a new integer larger than #lastStamp# that is 
   * guaranteed to be larger than any integer in #visited#.
   * If there is no such int, then set all values in #visited#
   * to some small number, and return the next largest number.
   */ 
  int getStamp();

  /** 
   * @return true iff using single elimination and approximate degree updates
   *         false if using multiple elimination and exact degree updates.
   */
  virtual bool requireSetDiffs()=0; 

  /**
   * reprioritize the reachable vertices based on some
   * characteristic of the quotient graph and reinsert
   * them into the priority queue
   */
  virtual bool prioritize( const VertexList& reachableVtxs, 
			      const QuotientGraph* g, 
			      BucketSorter* degStruct )=0;
  /** 
   * give const access to the basic ordering timer
   */
  const stopwatch& getTimer();
};

/**
 * @memo parent class for most multiple elimination strategies
 * @type class 
 *
 * @author Gary Kumfert
 * @version #$Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 * @see MinPriorityEngine
 */
class MinPriority_MultipleElimination : public MinPriorityStrategies {
public:
  MinPriority_MultipleElimination( const int Size )
    : MinPriorityStrategies( Size ) { }
  virtual ~MinPriority_MultipleElimination() {}
  virtual bool requireSetDiffs() { return false; }
  virtual bool prioritize( const VertexList& reachableVtxs, 
			   const QuotientGraph* g, 
			   BucketSorter* degStruct ) =0 ;
};


/**
 * @memo parent class for most approximate elimination strategies
 * @type class 
 *
 * @author Gary Kumfert
 * @version #$Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 * @see MinPriorityEngine
 */
class MinPriority_ApproximateElimination : public MinPriorityStrategies {
public:
  MinPriority_ApproximateElimination( const int Size )
    : MinPriorityStrategies( Size ) { }
  virtual ~MinPriority_ApproximateElimination() {}
  virtual bool requireSetDiffs() { return true; }
  virtual bool prioritize( const VertexList& reachableVtxs, 
			   const QuotientGraph* g, 
			   BucketSorter* degStruct ) =0 ;
};


/**
 * @memo multiple elimination with exact degree
 * @type class 
 *
 * @author Gary Kumfert
 * @version #$Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 * @see MinPriorityEngine
 */
class MinPriority_ExactDegree : public MinPriority_MultipleElimination {
public:
  MinPriority_ExactDegree(const int Size) 
    : MinPriority_MultipleElimination( Size ) { }
  virtual ~MinPriority_ExactDegree() {}
  
  virtual bool prioritize( const VertexList& reachableVtxs, 
			   const QuotientGraph* g, 
			   BucketSorter* degStruct );
};

/**
 * @memo multiple elimination with external degree a la Liu's GENMMD
 * @type class 
 *
 * @author Gary Kumfert
 * @version #$Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 * @see MinPriorityEngine
 */
class MinPriority_ExternalDegree : public MinPriority_MultipleElimination {
public:
  MinPriority_ExternalDegree( const int Size )
    : MinPriority_MultipleElimination( Size ) { }
  virtual ~MinPriority_ExternalDegree() { }

  virtual bool prioritize( const VertexList& reachableVtxs, 
			   const QuotientGraph* g, 
			   BucketSorter* degStruct );
};


/**
 * @memo approximate (single) elimination a la Amestoy, Davis, and Duff
 * @type class 
 *
 * @author Gary Kumfert
 * @version #$Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 * @see MinPriorityEngine
 */
class MinPriority_ApproximateDegree : public MinPriority_ApproximateElimination {
public:
  MinPriority_ApproximateDegree( const int Size )
    : MinPriority_ApproximateElimination( Size ) { }
  virtual ~MinPriority_ApproximateDegree() { }

  virtual bool prioritize( const VertexList& reachableVtxs, 
			   const QuotientGraph* g, 
			   BucketSorter* degStruct );
};


/**
 * @memo Approximate Minimum Fill (Rothberg)
 * @type class 
 *
 * @author Gary Kumfert
 * @version #$Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 * @see MinPriorityEngine
 */
class MinPriority_AMF : public MinPriority_MultipleElimination { 
public:
  MinPriority_AMF( const int Size ) : MinPriority_MultipleElimination( Size ) { }
  virtual ~MinPriority_AMF() { }
  virtual bool prioritize( const VertexList& reachableVtxs, 
			   const QuotientGraph* g, 
			   BucketSorter* degStruct );
};


/**
 * @memo Approximate Minimum Mean Fill (Rothberg & Eisenstat)
 * @type class 
 *
 * @author Gary Kumfert
 * @version #$Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 * @see MinPriorityEngine
 */
class MinPriority_AMMF : public MinPriority_ApproximateElimination { 
public:
  MinPriority_AMMF( const int Size ) 
    : MinPriority_ApproximateElimination( Size ) { }
  virtual ~MinPriority_AMMF() { }
  virtual bool prioritize( const VertexList& reachableVtxs, 
			   const QuotientGraph* g, 
			   BucketSorter* degStruct );
};

/**
 * @memo Approximate Minimum Increase in Neighbor Degree (Rothberg & Eisenstat)
 * @type class 
 *
 * @author Gary Kumfert
 * @version #$Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 * @see MinPriorityEngine
 */
class MinPriority_AMIND : public MinPriority_ApproximateElimination {
public:
  MinPriority_AMIND( const int Size ) 
    : MinPriority_ApproximateElimination( Size ) { }
  virtual ~MinPriority_AMIND() { }
  virtual bool prioritize( const VertexList& reachableVtxs, 
			   const QuotientGraph* g, 
			   BucketSorter* degStruct );
};

/**
 * @memo Multiple Minimum Mean Fill (Rothberg & Eisenstat)
 * @type class 
 *
 * @author Gary Kumfert
 * @version #$Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 * @see MinPriorityEngine
 */
class MinPriority_MMMF : public MinPriority_MultipleElimination {
public:
  MinPriority_MMMF( const int Size ) 
    : MinPriority_MultipleElimination( Size ) {}
  virtual ~MinPriority_MMMF() { }
  virtual bool prioritize( const VertexList& reachableVtxs, 
			   const QuotientGraph* g, 
			   BucketSorter* degStruct );
};

/**
 * @memo Multiple Minimum Increase in Neighbor Degree (Rothberg & Eisenstat)
 * @type class 
 *
 * @author Gary Kumfert
 * @version #$Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 * @see MinPriorityEngine
 */
class MinPriority_MMIND : public MinPriority_MultipleElimination { 
public:
  MinPriority_MMIND( const int Size ) 
    : MinPriority_MultipleElimination( Size ) { }
  virtual ~MinPriority_MMIND() { }
  virtual bool prioritize( const VertexList& reachableVtxs, 
			   const QuotientGraph* g, 
			   BucketSorter* degStruct );
};

/**
 * @memo Multiple Minimum Deficiency (Ng & Raghavan)
 * @type class 
 *
 * @author Gary Kumfert
 * @version #$Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 * @see MinPriorityEngine
 */
class MinPriority_MMDF : public MinPriority_MultipleElimination { 
public:
  MinPriority_MMDF(const int Size) 
    : MinPriority_MultipleElimination( Size ) { }
  virtual ~MinPriority_MMDF() { }
  virtual bool prioritize( const VertexList& reachableVtxs, 
			   const QuotientGraph* g, 
			   BucketSorter* degStruct );
};


/**
 * @memo Modified Multiple Minimum Degree (Ng & Raghavan)
 * @type class 
 *
 * @author Gary Kumfert
 * @version #$Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 * @see MinPriorityEngine
 */
class MinPriority_MMMD : public MinPriority_MultipleElimination {
public:
  MinPriority_MMMD(const int Size) 
    : MinPriority_MultipleElimination( Size ) { }
  virtual ~MinPriority_MMMD() { }
  virtual bool prioritize( const VertexList& reachableVtxs, 
			   const QuotientGraph* g, 
			   BucketSorter* degStruct );
};



/**
 * @memo A simply MMD/AMD hybrid that switches whenever a specified percentage
 *       of the total ordering is complete.
 * @type class 
 *
 * @author Gary Kumfert
 * @version #$Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 * @see MinPriorityEngine
 */
class MinPriority_MMD_AMD_SimpleHybrid : public MinPriorityStrategies { 
private:
  bool multiple_elimination;
  float switch_bound;
public:
  MinPriority_MMD_AMD_SimpleHybrid();
  MinPriority_MMD_AMD_SimpleHybrid(const int Size, float switch_bnd );
  virtual ~MinPriority_MMD_AMD_SimpleHybrid();
  virtual bool requireSetDiffs();
  virtual bool prioritize( const VertexList& reachableVtxs, 
			   const QuotientGraph* g, 
			   BucketSorter* degStruct );
};


/**
 * @memo An advance Multiple Elimination/Approximate Elimination Hybrid
 *       that switches whenever a specified number of suboptimal multiple
 *       eliminations occur
 * @type class 
 *
 * @author Gary Kumfert
 * @version #$Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 * @see MinPriorityEngine
 */
class MinPriority_AdvancedHybrid : public MinPriorityStrategies {
private:
  bool useStrategy1;
  MinPriority_MultipleElimination * strategy1;
  MinPriority_ApproximateElimination * strategy2;
  int suboptimalMultipleEliminationSize; 
  int maxSuboptimalMultipleEliminations;
  int nSuboptimalMultipleEliminations;
  stopwatch runtime;
  
public:
  MinPriority_AdvancedHybrid();
  MinPriority_AdvancedHybrid( const int Size, const int tol, const int n );
  virtual ~MinPriority_AdvancedHybrid();
  virtual bool requireSetDiffs();
  virtual bool prioritize( const VertexList& reachableVtxs, 
			   const QuotientGraph* g, 
			   BucketSorter* degStruct );
  
  bool setMultipleEliminationStrategy(  MinPriority_MultipleElimination * strategy );
  bool setApproximateEliminationStrategy(  MinPriority_ApproximateElimination * strategy );
};

/**
 * @memo An advanced Multiple Elimination/Approximate Elimination Hybrid
 *       that switches whenever a sliding average of suboptimal multiple
 *       eliminations occur
 * @type class 
 *
 * @author Gary Kumfert
 * @version #$Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 * @see MinPriorityEngine
 */
class MinPriority_AdvancedHybrid2 : public MinPriorityStrategies {
private:
  bool useStrategy1;
  MinPriority_MultipleElimination * strategy1;
  MinPriority_ApproximateElimination * strategy2;
  float minTolerance; // minimum tolerated
  float *slidingAvg;  // array for the sliding average
  float curAvg;       // currentAverage
  int curIdx;         // current index into sliding avg.
  int historySize;    // size of the sliding average
  
public:
  MinPriority_AdvancedHybrid2();
  MinPriority_AdvancedHybrid2( const int Size, const float tol, const int n );
  virtual ~MinPriority_AdvancedHybrid2();
  virtual bool requireSetDiffs();
  virtual bool prioritize( const VertexList& reachableVtxs, 
			   const QuotientGraph* g, 
			   BucketSorter* degStruct );
  
  bool setMultipleEliminationStrategy(  MinPriority_MultipleElimination * strategy );
  bool setApproximateEliminationStrategy(  MinPriority_ApproximateElimination * strategy );
};


/**
 * @memo A Multiple/Approximate Elimination Hybrid that switches
 *       whenever the multiple elimination scheme is stalling.
 * @type class
 * 
 * @author Gary Kumfert
 * @version #$Id: MinPriorityStrategies.h,v 1.2 2000/02/18 01:31:53 kumfert Exp $#
 * @see MinPriorityEngine
 */
class MinPriority_TimedHybrid : public MinPriorityStrategies {
  stopwatch interval;
  bool useStrategy1;
  MinPriority_MultipleElimination * strategy1;
  MinPriority_ApproximateElimination * strategy2;
  float currentSlope; // slope of x,y line where x=seconds, y=number supernodes eliminated.
  float initialSlope; // slope established by first three mass eliminations
  float slopeChange;  // switch when currentSlope < initialSlope * slopeChange

public:
  MinPriority_TimedHybrid();
  MinPriority_TimedHybrid( const int Size, const float slopeChange );
  ~MinPriority_TimedHybrid();
  virtual bool requireSetDiffs();
  virtual bool prioritize( const VertexList& reachableVtxs, 
			   const QuotientGraph* g, 
			   BucketSorter* degStruct );
  
  bool setMultipleEliminationStrategy(  MinPriority_MultipleElimination * strategy );
  bool setApproximateEliminationStrategy(  MinPriority_ApproximateElimination * strategy );
};

SPINDLE_END_NAMESPACE

#endif
