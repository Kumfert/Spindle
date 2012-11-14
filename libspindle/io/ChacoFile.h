//
// ChacoFile.h  -- I/O routines for creating a graph from Chaco file
//
//  $Id: ChacoFile.h,v 1.3 2000/04/01 19:55:01 kumfert Exp $
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

#ifndef SPINDLE_CHACO_FILE_H_
#define SPINDLE_CHACO_FILE_H_

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
/**
 * @memo Reads and writes symmetric (possibly weighted) Graphs in Chaco format
 * @type class
 *
 *
 * @author Gary Kumfert
 * @version #$Id: ChacoFile.h,v 1.3 2000/04/01 19:55:01 kumfert Exp $#
 */

class ChacoFile:  public TextDataFile, public GraphBase { 
private:

  int CODE;

public:  

  static const int HAVE_VERTEX_WEIGHTS = 1;
  static const int HAVE_EDGE_WEIGHTS = 10;
  static const int HAVE_EXPLICIT_VERTEX_ORDER = 100;

  /**@name constructors/destructors */
  //@{
  /// default constructor
  ChacoFile() { CODE = -1; }
  /// destructor
  ~ChacoFile() {}
  //@}

  /**@name basic I/O operations */
  //@{
  bool read( FILE * stream  );
  bool write( FILE * stream );
  /// read in the formatted file
  bool read() { return read( fp ); }
  /// write out the formatted file
  bool write() { return write( fp ); }
  //@}
  
  /**@name Chaco Format Specifics */
  //@{
  /**
   * @memo set which (if any) weights to include in Chaco file.
   * @param code 0 for unweighted, +1 if edge weighted, +10 if vertex weighted
   */
  bool setCode (const int code);
  //@}
};
  
inline bool
ChacoFile::setCode (const int code) {  
  if ( (code == 0) || (code == 1) || (code == 10) || (code == 11) || (code == -1) ||
       (code == 100) || (code == 101) || (code == 110 ) || (code == 111) ) {
    CODE = code;
    return true;
  }
  return false;
}

SPINDLE_END_NAMESPACE

#endif
