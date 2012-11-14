//
// SpindlePersistant.cc   -- Everything derived from this class supports object persistance
//
//  $Id: SpindlePersistant.cc,v 1.2 2000/02/18 01:31:59 kumfert Exp $
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

#include "spindle/SpindlePersistant.h"

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif



#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

SPINDLE_IMPLEMENT_PERSISTANT( SpindlePersistant, SpindleBaseClass )

#ifdef __FUNC__
#undef __FUNC__
#endif

SpindlePersistant::SpindlePersistant() {
  incrementInstanceCount( SpindlePersistant::MetaData );
}

SpindlePersistant::~SpindlePersistant() {
  decrementInstanceCount( SpindlePersistant::MetaData );
}

void
SpindlePersistant::validate() {
  return; 
}

bool
SpindlePersistant::reset() {
  return true;
}

#define __FUNC__ "void SpindlePersistant::loadObject( SpindleArchive& ar )"
void
SpindlePersistant::loadObject( SpindleArchive& ar) {
  FENTER;
  WARN_IF( true, "Not Implemented");
# ifndef _MSC_VER
  (void) ar;
# endif
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "void SpindlePersistant::loadObject( SpindleArchive& ar )"
void
SpindlePersistant::storeObject( SpindleArchive& ar) const { 
  FENTER;
  WARN_IF( true, "Not Implemented");
# ifndef _MSC_VER
  (void) ar;
# endif
  FEXIT;
}
#undef __FUNC__
