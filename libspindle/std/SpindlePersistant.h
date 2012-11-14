//
// SpindlePersistant.h   -- Everything derived from this class supports object persistance
//
//  $Id: SpindlePersistant.h,v 1.2 2000/02/18 01:31:59 kumfert Exp $
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
//============================================================================
//
//

#ifndef SPINDLE_PERSISTANT_H_
#define SPINDLE_PERSISTANT_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifndef SPINDLE_BASE_CLASS_H_
#include "spindle/SpindleBaseClass.h"
#endif

#ifndef SPINDLE_ARCHIVE_H_
#include "spindle/SpindleArchive.h"
#endif

SPINDLE_BEGIN_NAMESPACE

class SpindleArchive;

/**
 * @memo Parent for all Persistant Data Types
 * @type class
 *
 * @author Gary Kumfert
 * @version #$Id: SpindlePersistant.h,v 1.2 2000/02/18 01:31:59 kumfert Exp $#
 * @see ClassMetaData
 */
class SpindlePersistant : public SpindleBaseClass {
protected:
  /**
   * @type enum { INVALID, EMPTY, UNKNOWN, VALID } currentState;
   * All descendants of this class use these four states
   * to describe themselves.
   */
  enum{ INVALID=-1, EMPTY=0, UNKNOWN=1, VALID=2} currentState;

  /**
   * constructor is protected so that derived classes cannot
   * be instantiated with a compiler provided default constructor.
   *
   * There is a static member function that is available for creating
   * empty instances.  A pointer to this static member function is
   * made available to ClassMetaData.
   */
  SpindlePersistant();

public:
  /**
   * destructor
   */
  virtual ~SpindlePersistant();
  
  /**
   * @return an instance whose #currentState==EMPTY#
   */
  static SpindleBaseClass* createObject();
  
  /**
   * @return true iff #currentState==VALID#
   */
  bool isValid() const;

  /**
   * the only function allowed to run when #currentState==INVALID#
   */
  virtual bool reset();
  /**
   * May change currentState from #UNKNOWN# to #VALID# 
   */
  virtual void validate();
  /**
   * Used for object persistance.
   */
  virtual void loadObject( SpindleArchive& ar );
  /**
   * Used for object persistance
   */
  virtual void storeObject( SpindleArchive& ar ) const;

  SPINDLE_DECLARE_DYNAMIC( SpindlePersistant )
};


inline bool
SpindlePersistant::isValid() const {
   return ( currentState == VALID );
}

#define SPINDLE_DECLARE_PERSISTANT( thisClass ) \
  SPINDLE_DECLARE_DYNAMIC( thisClass ) \
  static SpindleBaseClass* createObject(); \
  virtual void loadObject( SpindleArchive& ar );\
  virtual void storeObject( SpindleArchive& ar ) const ; \
 
#define SPINDLE_IMPLEMENT_PERSISTANT( thisClass, BaseClass )\
  _SPINDLE_IMPLEMENT_DYNAMIC( thisClass, BaseClass, thisClass::createObject ) \
  SpindleBaseClass* thisClass::createObject() \
    { return new thisClass; } \


SPINDLE_END_NAMESPACE

#endif
