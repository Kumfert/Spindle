//
// SpindleTraceCodes.h
//

#ifndef SPINDLE_TRACE_CODES_H_
#define SPINDLE_TRACE_CODES_H_

/** @name General Trace Codes 
 *
 * in #SpindleTraceCodes.h#
 */
//@{
/// Trace Everything
#define SPINDLE_TRACE_ALL            0xFFFFL

/// Trace SpindleStack			           
#define SPINDLE_TRACE_STACK          0x0001L
/// Trace general program flow 
#define SPINDLE_TRACE_FLOW           0x0002L
/// Trace previous debug things
#define SPINDLE_TRACE_DEBUG          0x0004L


/// Trace General information about QuotientGraph
#define SPINDLE_TRACE_QGRAPH             0x0010L 
/// Trace Specific information about QuotientGraph Updates
#define SPINDLE_TRACE_QGRAPH_UPD         0x0020L 
/// Trace General information about MinPriority Algorithms
#define SPINDLE_TRACE_MINPRIORITY        0x0040L 
/// Information about vtxs and weights generated
#define SPINDLE_TRACE_MINPRIORITY_SCHEME 0x0080L
/// announce when strategy switches
#define SPINDLE_TRACE_MINPRIORITY_STRATEGY_SWITCH 0x0100L 
/// trace the advanced hybrid MinPriority Strategy 
#define SPINDLE_TRACE_MINPRIORITY_STRATEGY_ADVHYBRID 0x0200L 

//@}

#endif
