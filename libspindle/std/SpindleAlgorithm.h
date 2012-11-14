//
// SpindleAlgorithm.h  -- Everything derived from this class supports object persistance
//
//  $Id: SpindleAlgorithm.h,v 1.2 2000/02/18 01:31:59 kumfert Exp $
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
//=====================================================================
//
//

#ifndef SPINDLE_ALGORITHM_H_
#define SPINDLE_ALGORITHM_H_

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifndef SPINDLE_BASE_CLASS_H_
#include "spindle/SpindleBaseClass.h"
#endif 

SPINDLE_BEGIN_NAMESPACE

/**
 * @memo Parent for all major algorithms.  Defines Algorithm FSM.
 * @type class
 * 
 * All classes that derive from this class adhere to a finite state
 * mechanism of four states: #EMPTY#, #READY#, #DONE#, and #INVALID#
 * as defined by the protected enumeration field #algorithmicState#.
 * 
 * \begin{itemize}
 *  \item[#EMPTY#] The state when there is not enough input to 
 * 		     acutally run the algorithmic engine.
 * 		     This is the normal state after creation
 * 		     by the default constructor and after a
 * 		     successful call to the member function #reset()#.
 * 		     All constructors should call #reset()# as part of
 * 		     the construction.
 *   \item[#READY#] The state when there is enough input to run the
 * 		     algorithmic engine.  This state can be entered
 * 		     by either a successful construction and setting
 * 		     of input.  Often a ``convenience constructor''
 * 		     is available that will set the class to this
 * 		     state, (or in event of a problem #INVALID#).
 * 		     In this state, additional 
 * 		     options can be set, or the algorithm
 * 		     can be run by calling the virtual member function 
 * 		     #execute()#.  In some cases, #execute()# may not
 * 		     exhaust all of its input, and the class would
 * 		     therefore remain in this state, even though it
 * 		     is, in fact, partially done.
 *   \item[#DONE#]  This is the state after all the input has been used
 * 		     in computing a full result.  It is only entered into
 * 		     by invoking #execute()#, though this is not sufficient.
 * 		     In this stage, the user may extract results from the
 * 		     algorithmic class, query timing or statistics.  Often
 * 		     such querys do not cause a change in state.
 *   \item[#INVALID#] This is a catchall error state that can be entered
 * 		     whenever a failure is detected.  Once a class has
 * 		     entered in an INVALID state, it can only provide 
 * 		     some information regarding to why.  Invoking the
 * 		     member function #reset()# is the only way to 
 * 		     exit this state.
 *  \end{itemize}
 * 
 * @author Gary Kumfert
 * @version #$Id: SpindleAlgorithm.h,v 1.2 2000/02/18 01:31:59 kumfert Exp $#
 * @see ClassMetaData
 */
class SpindleAlgorithm : public SpindleBaseClass {

protected:
  /**
   * @type enum { INVALID, EMPTY, READY, DONE } algorithmicState 
   * All descendants of this class use these four states
   * to describe themselves.
   */
  enum { INVALID=-1, EMPTY=0, READY=1, DONE=2 } algorithmicState;

  /**
   *  If an algorithm enters and invalid state, it should
   *  have a useful error message here.  (WARNING, not well supported.)
   */
  char* errMsg;

public:
  /**
   * Should be called only when algorithmicState==READY.
   *   If execution succeeds, set algorithmicState = DONE and return true;
   *   If execution fails, set algorithmicState = INVALID, set errMsg, and return false;
   */
  virtual bool execute()=0; 
  
  /**
   * Always sets algorithmicState = EMPTY, 
   * and restores Algorithm object to state indistinguishable from default constructor.
   * All constructors should call reset for consistency.
   */
  virtual bool reset()=0;   

  /**
   * when algorithmicState==INVALID, this returns the error message
   * otherwise, it returns 0
   */
  const char* queryErrMsg() const;

  /** @name simple tests to determine state */
  //@{
  /** @return true iff algorithmicState != INVALID */
  bool isValid() const  { return algorithmicState != INVALID; }
  /** @return true iff algorithmicState == INVALID */
  bool notValid() const  { return algorithmicState == INVALID; }
  /** @return true iff algorithmicState == EMPTY */
  bool isEmpty() const  { return algorithmicState == EMPTY; }
  /** @return true iff  algorithmicState != EMPTY */
  bool notEmpty() const  { return algorithmicState != EMPTY; }
  /** @return true iff  algorithmicState == READY */
  bool isReady() const  { return algorithmicState == READY; }
  /** @return true iff  algorithmicState != READY */
  bool notReady() const  { return algorithmicState != READY; }
  /** @return true iff algorithmicState == DONE */
  bool isDone()  const  { return algorithmicState == DONE; }
  /** @return true iff  algorithmicState != DONE */
  bool notDone()  const  { return algorithmicState != DONE; }
  //@}

  SPINDLE_DECLARE_DYNAMIC( SpindleAlgorithm )

};


inline const char* 
SpindleAlgorithm::queryErrMsg() const {
  return (errMsg != 0 ) ? errMsg : ""; 
}

SPINDLE_END_NAMESPACE

#endif
