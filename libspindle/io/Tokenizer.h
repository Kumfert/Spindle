//
// Tokenizer.h -- takes a large string and extracts smaller ones
// 
// $Id: Tokenizer.h,v 1.2 2000/02/18 01:31:52 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1999, Old Dominion University.  All rights reserved.
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

#ifndef SPINDLE_TOKENIZER_H_
#define SPINDLE_TOKENIZER_H_

#include <string>
using namespace std;

#include "spindle/spindle.h"

SPINDLE_BEGIN_NAMESPACE
  
class Tokenizer {
protected:
  string::size_type m_iStart;
  string::size_type m_iStop;
  string m_sSource;
  string m_sTokenBoundary;
  string m_sLineBoundary;
  
  bool interpretEscapedChars; // eg \\ \n \t 
  bool quoteSensitive;         // eg " this is one token "
  
  string::size_type scanForTokenEnd();
  
  public:
  Tokenizer();
  bool setSource( const string& source );
  bool setSource( const char * source );
  
  void setTokenBound( string& tokBnd );
  void setTokenBound( const char * tokBnd );
  
  void setLineBound( string&  lnBnd );
  void setLineBound( const char * lnBnd );
  
  bool nextToken(); // advances to next token in the line
  bool nextLine(); // advances to the next line whether or not the current one is exhausted.
  bool eos();  // end of string
  bool eoln(); // end of line
  string getValue(); // returns current token
  string getLine();  // returns current line including token
  
  void setInterpretEscapedChars( bool value );
  void setQuoteSensitive( bool value );
  
  static void trim( string& str ); // removes whitespace from beginning and end of string.
  // including newlines
  
};

SPINDLE_END_NAMESPACE

#endif 
