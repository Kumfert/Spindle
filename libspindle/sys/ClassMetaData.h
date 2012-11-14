//
// ClassMetaData.h
//
// $Id: ClassMetaData.h,v 1.2 2000/02/18 01:32:00 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1997, Old Dominion University.  All rights reserved.
// 
//  Permission to use, copy, modify , distribute and sell this software and
//  its documentation for any purpose is hereby granted without fee, 
//  provided that the above copyright notice appear in all copies and
//  that both that copyright notice and this permission notice appear
//  in supporting documentation.  Old Dominion University makes no
//  representations about the suitability of this software for any 
//  purpose.  It is provided "as is" without express or implied warranty.
//
//=====================================================================
//

#ifndef SPINDLE_CLASS_META_DATA_H_
#define SPINDLE_CLASS_META_DATA_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

SPINDLE_BEGIN_NAMESPACE

class SpindleBaseClass;

/** 
 *  @memo Meta Data on descendants of SpindleBaseClass
 *  @type class
 * 
 *  @author Gary Kumfert
 *  @version #$Id: ClassMetaData.h,v 1.2 2000/02/18 01:32:00 kumfert Exp $#
 *  @see SpindleBaseClass
 */

class ClassMetaData {
public:

  /// A zero terminated string of the class's name
  char* className;

  /// The size of the class as returned by #sizeof()#
  int  classSize;

  /// A pointer to the meta data of the class's parent
  ClassMetaData *parentMetaData;

  /** a pointer to the default constructor of the class
   * Can be NULL if the class is abstract
   */
  SpindleBaseClass* (* pfnCreateObject )(); 

  /// The count of number of instances created.
  int nCreated;

  /// The count of number of instances destroyed.
  int nDestroyed;
 
  /**
   * @memo creates a new instance of the class the metadata represents.
   * @return a new instance if it can create one, else NULL.
   */
  SpindleBaseClass* createObject();

  /**
   * @memo Used before #dynamic_cast<>()# was available
   * @return true if this meta data represents a descendent of the
   *         class represented by p.
   * @param p non-null pointer to a class's metadata.
   */
  bool isDerivedFrom( const ClassMetaData* p) const;
};

SPINDLE_END_NAMESPACE
 
#endif
