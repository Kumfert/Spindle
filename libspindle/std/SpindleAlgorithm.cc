//
// SpindleAlgorithm.cc  -- Everything derived from this class supports object persistance
//
//  $Id: SpindleAlgorithm.cc,v 1.2 2000/02/18 01:31:59 kumfert Exp $
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

#include "spindle/SpindleAlgorithm.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

bool 
SpindleAlgorithm::reset() { 
  algorithmicState = EMPTY;
  return true;
}

SPINDLE_IMPLEMENT_DYNAMIC( SpindleAlgorithm, SpindleBaseClass )






