//
// CommandLineOptions.cc
//
// $Id: CommandLineOptions.cc,v 1.2 2000/02/18 01:32:00 kumfert Exp $
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
//

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include <string.h>
#include <assert.h>
#else
#include <string>
#include <cassert>
using namespace std;
#endif

#include "spindle/CommandLineOptions.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

void CommandLineOptions::parseFlag( const char* flag ) {
  // first copy the char string to the buffer
  strcpy(buffer,flag );

  // then parse the string for ``|'' characters.
  // convert them to nils and record their location.
  String[0] = buffer;
  nStrings = 1;
  char* ch = buffer;
  while ( *ch != '\0' ) {
    if (*ch == '|') {
      *ch = '\0';
      ch++;
      String[nStrings++] = ch;
      assert( nStrings < maxStrings);
    } else {
      ch++;
    }
  }
}

int CommandLineOptions::findFlag( const char* flag ) {
  parseFlag( flag );

  // Now go through the list of commandline arguments
  for ( int i=0; i<nOptions; i++ ) {
    for ( int j=0; j<nStrings; j++ ) {
      if (! strcmp( Option[i],String[j] ) ) {
	return i;
      }
    }
  }
  return nOptions;
}

bool CommandLineOptions::hasFlag( const char* flag ) {
  return findFlag( flag ) != nOptions;
}

const char* CommandLineOptions::getOption( const char* flag ) {
  int location = findFlag( flag );
  if (++location >= nOptions ) {
    return (const char*) 0;
  } else {
    return Option[location];
  }
}
const char* CommandLineOptions::getOption( const char* flag, int offset ) {
  int location = findFlag( flag );
  if (location+offset+1 >= nOptions ) {
    return (const char*) 0;
  } else {
    return Option[location+offset+1];
  }
}

bool CommandLineOptions::hasOption(const char* flag, const char* option ) {
  int location = findFlag( flag );
  if ( ++location >= nOptions ) {
    return false;
  } else {
    return (!strcmp( option, Option[location] ));
  }
}

bool CommandLineOptions::hasOption(const char* flag, const char option ) {
  int location = findFlag( flag );
  if ( ++location >= nOptions ) {
    return false;
  } else {
    const char* ch = Option[location];
    return ((ch[0]==option) && (ch[1]=='\0'));
  }
}

bool CommandLineOptions::hasOption(const char* flag, const int option ) {
  int location = findFlag( flag );
  if ( ++location >= nOptions ) {
    return false;
  } else {
    return (atoi( Option[location] ) == option);
  }
}

bool CommandLineOptions::hasOption(const char* flag, const double option ) {
  int location = findFlag( flag );
  if ( ++location >= nOptions ) {
    return false;
  } else {
    return (atof( Option[location] ) == option);
  }
}

