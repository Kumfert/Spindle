//
// MapUtils.h
//
//  $Id: MapUtils.h,v 1.2 2000/02/18 01:31:48 kumfert Exp $
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

#ifndef SPINDLE_MAP_UTILS_H_
#define SPINDLE_MAP_UTILS_H_

#ifndef SPINDLE_PERMUTATION_MAP_H_
#include "spindle/PermutationMap.h"
#endif

#ifndef SPINDLE_COMPRESSION_MAP_H_
#include "spindle/CompressionMap.h"
#endif

SPINDLE_BEGIN_NAMESPACE

class MapUtils { 
public:
  static PermutationMap * createUncompressedPermutationMap( const PermutationMap* perm, const CompressionMap* cmap ) ;
};

SPINDLE_END_NAMESPACE

#endif
