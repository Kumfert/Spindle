//
// SpoolesFile.h  -- I/O routines for creating a graph from Spooles file
//
//  $Id: SpoolesFile.h,v 1.2 2000/02/18 01:31:52 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1997, Old Dominion University.  All rights reserved.
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

#ifndef SPINDLE_SPOOLES_FILE_H_
#define SPINDLE_SPOOLES_FILE_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifndef SPINDLE_TEXT_DATA_FILE_H_
#include "spindle/TextDataFile.h"
#endif

#ifndef SPINDLE_GRAPH_BASE_H_
#include "spindle/GraphBase.h"
#endif

SPINDLE_BEGIN_NAMESPACE

class SpoolesFile:  public TextDataFile, public GraphBase { 
private:

  int m_type;  // vertexWeighted  + 2*edgeWeighted

  bool read_binary( FILE * stream );
  bool read_formatted( FILE * stream );
  bool write_binary( FILE * stream );
  bool write_formatted( FILE * stream );

public:  
  SpoolesFile(): m_type(0) {}
  ~SpoolesFile() {}

  bool read( FILE * stream  );
  bool write( FILE * stream );
  bool read() { return read( fp ); }
  bool write() { return write( fp ); }
  bool setType (const int type);
};
  
inline bool
SpoolesFile::setType (const int type) {  
  if ( (type < 0) || (type > 3 ) ) {
    return false;
  } else { 
    m_type = type;
  } 
  return true;
}

SPINDLE_END_NAMESPACE

#endif
