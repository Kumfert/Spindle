//
// SpindleBaseClass.cc  -- A cosmic base class for the Spindle framework
//
//  $Id: SpindleBaseClass.cc,v 1.2 2000/02/18 01:31:59 kumfert Exp $
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

#ifndef SPINDLE_BASE_CLASS_H_
#include "spindle/SpindleBaseClass.h"
#endif

#ifndef SPINDLE_ARCHIVE_H_
#include "spindle/SpindleArchive.h"
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

static const ClassMetaData SpindleBaseClass_MetaData = {
  const_cast<char*>("SpindleBaseClass"), 
  sizeof( SpindleBaseClass ), 
  0, 
  SpindleBaseClass::createObject,
  0, 0
  };


static int UniqueIDNumber = 0;

const ClassMetaData* SpindleBaseClass::MetaData = &SpindleBaseClass_MetaData;

static const registry_init registry_init_dummy_class( &SpindleBaseClass_MetaData );

registry_init::registry_init( const ClassMetaData* metaData ) {
  SpindleSystem::registry().registerClass( metaData );
}

#ifdef __FUNC__
#undef __FUNC__
#endif

const ClassMetaData* 
SpindleBaseClass::getMetaData() const { 
  return &SpindleBaseClass_MetaData;
}

SpindleBaseClass::~SpindleBaseClass() {
  decrementInstanceCount( SpindleBaseClass::MetaData );
}

SpindleBaseClass::SpindleBaseClass() : idNumber(UniqueIDNumber++) {  
  incrementInstanceCount( SpindleBaseClass::MetaData );
}

SpindleBaseClass::SpindleBaseClass( const SpindleBaseClass& src ) : idNumber(UniqueIDNumber++) {
  // dummy use of src to keep compiler from complaining
  (void) src;
}

void
SpindleBaseClass::operator=( const SpindleBaseClass& src ) {
  // dummy use of src to keep compiler from complaining
  (void) src;
}

#define __FUNC__ "void SpindleBaseClass::loadObject( SpindleArchive& ar )"
void
SpindleBaseClass::loadObject( SpindleArchive& ar ) {
  FENTER;
  ASSERT( false, "Method not implemented.");
# ifndef _MSC_VER
  (void) ar;
# endif
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "void SpindleBaseClass::loadObject( SpindleArchive& ar )"
void
SpindleBaseClass::storeObject( SpindleArchive& ar ) const {
  FENTER;
  ASSERT( false, "Method not implemented.");
# ifndef _MSC_VER
  (void) ar;
# endif
  FEXIT;
}
#undef __FUNC__

// bool 
// SpindleBaseClass::isKindOf( const ClassMetaData* metaData ) const { 
//   const ClassMetaData* myType = this->getMetaData();
//   for(const ClassMetaData* temp = metaData; temp != 0; temp = temp->parentMetaData) {
//     if ( temp == myType ) {
// 	 return true;
//     } 
//   }
//   return false;
// }

SpindleBaseClass* 
SpindleBaseClass::dynamicCast( SpindleBaseClass * object ) {
  if ( object == 0 ) { return 0; }
  else if ( object->getMetaData() != SpindleBaseClass::MetaData ) { return 0; }
  else { return (SpindleBaseClass *) object; }
}

const SpindleBaseClass* 
SpindleBaseClass::dynamicCast( const SpindleBaseClass * object ) {
  if ( object == 0 ) { return 0; }
  else if ( object->getMetaData() != SpindleBaseClass::MetaData ) { return 0; }
  else { return (const SpindleBaseClass *) object; }
}

SpindleBaseClass* 
SpindleBaseClass::createObject() 
{ return new SpindleBaseClass; }

void 
SpindleBaseClass::dump( FILE * stream ) const {
  fprintf(stream,"SpindleBaseClass:\n");
}
