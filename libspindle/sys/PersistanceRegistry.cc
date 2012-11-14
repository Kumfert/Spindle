//
// PersistanceRegistry
//
// $Id: PersistanceRegistry.cc,v 1.2 2000/02/18 01:32:01 kumfert Exp $
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
///////////////////////////////////////////////////////////////////////
//

#include "spindle/PersistanceRegistry.h"

#ifndef SPINDLE_CLASS_META_DATA_H_
#include "spindle/ClassMetaData.h"
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

PersistanceRegistry::PersistanceRegistry() {
  dumpOnDestroy = 0;
}

PersistanceRegistry::~PersistanceRegistry() {
  if ( dumpOnDestroy != 0 ) {
    fprintf( dumpOnDestroy,"Destroying Registry... ");
    dump( dumpOnDestroy );
  }
}


PersistanceRegistry& 
PersistanceRegistry::get() {
  static PersistanceRegistry pr;
  return pr;
}

void
PersistanceRegistry::registerClass( const ClassMetaData* metaData ) {
  if ( registry.find( metaData->className ) == registry.end() ) { // if its not yet registered
    registry[ metaData->className ] = metaData;
  }
}
 
const ClassMetaData*
PersistanceRegistry::findClass( const char* className ) {
  RegistryType::iterator it = registry.find( className );
  if ( it != registry.end() ) { // we've found it!
    return (*it).second;
  }
  return 0;
}

void
PersistanceRegistry::dump( FILE * stream ) const {
 
#ifdef SPINDLE__NO_INSTANCE_COUNT
  fprintf(stream ,"WARNING: Instance counting disabled for this compilation platform\n");
#endif

 fprintf(stream,"PersistanceRegistery::dump():  %d items\n", registry.size());
  for( RegistryType::const_iterator it = registry.begin(), stop = registry.end();
       it != stop; ++it ) {
    const ClassMetaData* self = (*it).second;
    if ( self != 0 ) {
      const char* className = ( (self->className == 0) ? "unnamed" : self->className );
      const char* parentName = "";
      const ClassMetaData* parent = self->parentMetaData;
      if ( parent != 0 ) {
	parentName = ( (parent->className == 0) ? parentName : parent->className);
      }
      fprintf(stream, "  %25s (%d) %d/%d---> %s\n", className, self->classSize, self->nCreated, 
	      self->nDestroyed, parentName);
    } // end if (self != 0)
  } // end for-all in registry
}



