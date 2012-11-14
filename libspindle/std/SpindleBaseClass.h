//
// SpindleBaseClass.h  -- A cosmic base class for the Spindle framework
//
//  $Id: SpindleBaseClass.h,v 1.2 2000/02/18 01:31:59 kumfert Exp $
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
// =========================================================================
//
//


#ifndef SPINDLE_BASE_CLASS_H_
#define SPINDLE_BASE_CLASS_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifndef SPINDLE_CLASS_META_DATA_H_
#include "spindle/ClassMetaData.h"
#endif

SPINDLE_BEGIN_NAMESPACE

class SpindleArchive;

/**
 *  @memo Cosmic Abstract Base Class 
 *  @type class
 *
 * All classes derived from this have an overhead of four virtual
 * functions and a static ClassMetaData.
 *
 *  @author Gary Kumfert
 *  @version #$Id: SpindleBaseClass.h,v 1.2 2000/02/18 01:31:59 kumfert Exp $#
 *  @see ClassMetaData
 * 
 */
class SpindleBaseClass {
private:
  /// default copy constructor disabled, no implementation
  SpindleBaseClass( const SpindleBaseClass& src );      // no implementation
  /// default assignment operator disabled, no implementation
  void operator=( const SpindleBaseClass& src );      // no implementation
  
  /** 
   *  A unique identification number is assigned to each instance that
   *  inherits from SpindleBaseClass
   */
  int idNumber;

protected:
  /** 
   *  @memo the default constructor is protected 
   *  to control instantiation.
   */
  SpindleBaseClass();

  /**
   * This member function should be called by every constructor 
   *      of every class derived from #SpindleBaseClass#.
   * @memo increase the instance count in the #ClassMetaData# instance
   *       associated with the specific descendant of #SpindleBaseClass#
   * @param metaData must be a valid pointer
   */
  void incrementInstanceCount( const ClassMetaData * metaData );
  
  /**
   * This member function should be called by every constructor 
   *      of every class derived from #SpindleBaseClass#.
   * @memo decrease the instance count in the #ClassMetaData# instance
   *       associated with the specific descendant of #SpindleBaseClass#
   * @param metaData must be a valid pointer
   */
  void decrementInstanceCount( const ClassMetaData * metaData );

public:
  /** @memo destructor */
  virtual ~SpindleBaseClass();

  /**
   *  @memo get the unique ID number of this instance
   *  @return positive integer that is unique among all
   *          instances that inherit from SpindleBaseClass
   */
  int queryIDNumber() const { return idNumber; }

  /// get access to the meta data of a class
  virtual const ClassMetaData* getMetaData() const;

  /// dump the instance to a stream (usually for debugging)
  virtual void dump( FILE * stream ) const;

  /// load an instance from an archive (children of SpindlePersistant only!!!)
  virtual void loadObject( SpindleArchive& ar );

  /// write this instance to an archive (children of SpindlePersistant only!!!)
  virtual void storeObject( SpindleArchive& ar ) const;

  /// static pointer to metadata for each instance
  static const ClassMetaData* MetaData;

  /** 
   * used when #dynamic_cast<>()# was not available
   * @return the pointer only if it is safely down-casted from 
   *          #SpindleBaseClass#, NULL otherwise
   */
  static SpindleBaseClass* dynamicCast( SpindleBaseClass* object );

  /// used when #dynamic_cast<>()# was not available
  static const SpindleBaseClass* dynamicCast( const SpindleBaseClass* object );
  
  /// create an empty instance via static function
  static SpindleBaseClass* createObject();
};

inline void
SpindleBaseClass::incrementInstanceCount( const ClassMetaData* metaData ) {
  ClassMetaData* thisData = (ClassMetaData*) metaData; // cast away const-ness
  if ( thisData != 0 ) {
#ifdef SPINDLE__NO_INSTANCE_COUNT
    ;
#else
    thisData->nCreated++;
#endif
  }
}

inline void
SpindleBaseClass::decrementInstanceCount( const ClassMetaData* metaData ) {
  ClassMetaData* thisData = (ClassMetaData*) metaData;
  if ( thisData != 0 ) {
#ifdef SPINDLE__NO_INSTANCE_COUNT
    ;
#else
    thisData->nDestroyed++;
#endif
  }

}

class registry_init {
public:
  registry_init( const ClassMetaData* metaData );
};

#define META_DATA( thisClass ) ( ( ClassMetaData*)( thisClass::MetaData ) )

#define SPINDLE_DECLARE_DYNAMIC( thisClass ) \
public: \
  static const ClassMetaData* MetaData; \
  virtual const ClassMetaData* getMetaData() const; \
  static thisClass* dynamicCast( SpindleBaseClass* object); \
  static const thisClass* dynamicCast( const SpindleBaseClass* object); \

#define SPINDLE_IMPLEMENT_DYNAMIC( thisClass, parentClass ) \
  static const ClassMetaData thisClass##_MetaData = { \
    #thisClass, \
    sizeof( class thisClass ), \
    META_DATA( parentClass ), \
    0, 0, 0 \
    }; \
  const ClassMetaData* thisClass::MetaData = &thisClass##_MetaData; \
  static const registry_init init_##thisClass( &thisClass##_MetaData ); \
  const ClassMetaData* thisClass::getMetaData() const \
    { return  &thisClass##_MetaData ; } \
  thisClass* thisClass::dynamicCast( SpindleBaseClass* object) \
    { if ( object == 0 ) { return 0; } \
      else if (object->getMetaData() != thisClass::MetaData ) { return 0; }\
      else { return (thisClass *) object; } } \
  const thisClass* thisClass::dynamicCast( const SpindleBaseClass* object) \
    { if ( object == 0 ) { return 0; } \
      else if (object->getMetaData() != thisClass::MetaData ) { return 0; } \
      else { return (thisClass *) object; } } 

#define _SPINDLE_IMPLEMENT_DYNAMIC( thisClass, parentClass, pfNew ) \
  static const ClassMetaData thisClass##_MetaData = { \
    #thisClass, \
    sizeof( class thisClass ), \
    META_DATA( parentClass ), \
    pfNew, 0, 0 \
    }; \
  const ClassMetaData* thisClass::MetaData = & thisClass##_MetaData;\
  static const registry_init init_##thisClass( &thisClass##_MetaData ); \
  const ClassMetaData* thisClass::getMetaData() const \
    { return & thisClass##_MetaData ; } \
  thisClass* thisClass::dynamicCast( SpindleBaseClass* object) \
    { if ( object == 0 ) { return 0; } \
      else if (object->getMetaData() != thisClass::MetaData ) { return 0; } \
      else { return (thisClass *) object; } } \
  const thisClass* thisClass::dynamicCast( const SpindleBaseClass* object) \
    { if ( object == 0 ) { return 0; } \
      else if (object->getMetaData() != thisClass::MetaData ) { return 0; }\
      else { return (thisClass *) object; } } 

SPINDLE_END_NAMESPACE

#endif
