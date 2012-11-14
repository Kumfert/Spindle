//
// src/io/drivers/tests/test03.cc -- tests persistance of PermutationMap class
//
//  $Id: test03.cc,v 1.2 2000/02/18 01:32:08 kumfert Exp $
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
//
//
//

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifndef SPINDLE_PERMUTATION_H_
#include "spindle/PermutationMap.h"
#endif

#ifndef SPINDLE_ARCHIVE_H_
#include "spindle/SpindleArchive.h"
#endif

#ifndef SPINDLE_FILE_H_
#include "spindle/SpindleFile.h"
#endif

#ifdef __FUNC__
#undef __FUNC__
#endif

#define __FUNC__ "main()"
#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

main() {  
  SpindleSystem::registry().setDumpOnDestroy( stdout );
  SpindleSystem::setErrorFile( stdout );
  SpindleSystem::setWarnFile( stdout );

  int perm_array[] = { 3, 4, 2, 0, 1};
  PermutationMap perm(5);
  perm.getNew2Old().take( perm_array, 5 );
  perm.validate();

  {
    SpindleFile outputFile( "temp.out", "w");
    SpindleArchive archive( &outputFile, SpindleArchive::store | SpindleArchive::noByteSwap );
    archive << &perm;
    archive.close();
  }

  SpindleBaseClass* object=0;
  {
    SpindleFile inputFile( "temp.out", "r");
    SpindleArchive  archive2( &inputFile, SpindleArchive::load | SpindleArchive::noByteSwap );
    archive2 >> object;
  }
    
  if ( object == 0 ) { 
    WARNING("Did not retrieve object.");
  } else {
    PermutationMap* perm2 =  PermutationMap::dynamicCast( object );
    if ( perm2 != 0 ) {
      perm2->validate();
      perm2->dump( stdout );
      fprintf( stdout, "\n");
    } else { 
    WARNING("Dynamic Cast failed!");
    }
    delete perm2;
  }
}
