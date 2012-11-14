//
// Tokenizer.cpp 
//
// $Id: Tokenizer.cc,v 1.3 2000/03/07 22:22:04 kumfert Exp $
//
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
//========================================================================
//
//
//


#include "spindle/Tokenizer.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
using namespace std;
#endif

Tokenizer::Tokenizer() {
  m_iStart = string::npos;
  m_iStop = 0;
  m_sTokenBoundary = " \t";
  m_sLineBoundary = "\n";
  interpretEscapedChars = false;
  quoteSensitive = true;
}

bool Tokenizer::setSource( const string& source ) {   
  m_sSource = source;
  m_iStop = 0 ;
  return true;
}

bool Tokenizer::setSource( const char * source ) {
  if ( source == 0 ) {
    return false;
  } else {
    m_sSource = source;
    m_iStop = 0;
    return true;
  }
}

void Tokenizer::setTokenBound( string& tokBnd ) {
  m_sTokenBoundary = tokBnd;
}

void Tokenizer::setTokenBound( const char * tokBnd ) {
  if ( tokBnd != 0 ) {
    m_sTokenBoundary = tokBnd;
  } else {
    m_sTokenBoundary = "";
  }
}

void Tokenizer::setLineBound(string& lnBnd ) {
  m_sLineBoundary = lnBnd ;
}

void Tokenizer::setLineBound( const char * lnBnd ) {
  if ( lnBnd != 0 ) {
    m_sLineBoundary = lnBnd ;
  } else {
    m_sLineBoundary = "" ;
  }
}

bool Tokenizer::nextToken() {
  if ( m_iStop == string::npos ) { 
    m_iStop = m_sSource.size() ;
  }
  if ( m_iStop == m_sSource.size() ) { // if where I ended last time is the end of the string
    m_iStart = string::npos;          //    flag start so getValue() doesn't work
    return false;                     //    no more tokens
  }
  // find the beginning of the next token.
  m_iStart = m_sSource.find_first_not_of( m_sTokenBoundary , m_iStop );
  if ( m_iStart == string::npos ) {   // if I don't have any nonTokenboundary chars
    m_iStop = m_sSource.size();       //    I have all whitespace to the end.
    m_iStart = string::npos;          //    flag start so getValue() doesn't work
    return false;                     //    no more tokens
  }
  string::size_type nextLine = string::npos;
  if ( m_sLineBoundary != "" ) {      // if I have line boundaries enforced
    // find the next line boundary
    nextLine = m_sSource.find_first_of ( m_sLineBoundary , m_iStop );
    if ( nextLine == m_iStart ) {    //  if my next token is, in fact, a line boundary
      m_iStop = nextLine;            //     Set the end to the line token
      m_iStart = string::npos;       //     flag start so getValue() doesn't work
      return false;                  //     no more tokens on this line
    }
  }
  // now find the end of the current token
  // m_iStop = m_sSource.find_first_of( m_sTokenBoundary, m_iStart );
  m_iStop = scanForTokenEnd();
  if ( nextLine < m_iStop ) {        // if next line begins before end of token
    m_iStop = nextLine;
  }
  if ( m_iStop == string::npos ) {   // if no end found
    m_iStop = m_sSource.size();      //    set end to end of string
  }
  return true;
}

bool Tokenizer::nextLine() {
  if ( m_sLineBoundary == "" ) {
    return false; // didn't define what terminates a line
  }
  if ( m_iStop == m_sSource.size() ) {
    return false; // no more space anyway.
  }
  m_iStart = m_sSource.find_first_of( m_sLineBoundary, m_iStop );
  if ( m_iStart != string::npos ) { // if there are more line boundaries
    m_iStop = m_iStart+1;           //   set Stop to first char after boundary.
    return true;
  } else {                          // else 
    m_iStop = m_sSource.size();
    m_iStart = string::npos;
    return false;                   //   return false, no more lines.
  }
}

string Tokenizer::getValue() {
  if ( m_iStart == string::npos ) {
    return string("");
  } else {
    return string( m_sSource.substr( m_iStart, m_iStop - m_iStart ) ) ;
  }
}

string Tokenizer::getLine() {
  if ( m_iStart == string::npos ) {
    return string("");
  } else if ( m_sLineBoundary == "" ) {
    return string("");
  } else { 
    string::size_type temp = m_sSource.find_first_of( m_sLineBoundary, m_iStart );
    return string( m_sSource.substr( m_iStart, temp - m_iStart ) );
  }
}


bool Tokenizer::eos() { 
  return ( m_iStop == m_sSource.size() || m_iStop == string::npos );
}

bool Tokenizer::eoln() {
  if ( eos() ) { 
    return true;
  }
  if ( m_sLineBoundary == "" ) {
    return false;
  }
  return ( m_iStop == m_sSource.find_first_of( m_sLineBoundary, m_iStop ) );
}


void Tokenizer::setInterpretEscapedChars( bool value ) {
  interpretEscapedChars = value;
}

void Tokenizer::setQuoteSensitive( bool value ) {
  quoteSensitive = value;
}

void Tokenizer::trim( string& str ) {
  string::size_type back = str.size();
  char ch = str[back-1];
  while( ch == ' ' || ch == '\t' || ch == '\n' ) {
    --back;
    ch = str[ back -1 ];
  }
  ++back;
  if ( back < str.size() ) {
    str.erase( back , str.size() - back );
  }
  string::size_type front = str.find_first_not_of( " \t\n" );
  if ( front != 0 ) {
    str.erase( 0, front );
  }
}


string::size_type Tokenizer::scanForTokenEnd() {
  if ( (!interpretEscapedChars) && ( !quoteSensitive ) ) {
    return m_sSource.find_first_of( m_sTokenBoundary, m_iStart );
  } else if ( (!interpretEscapedChars) && ( quoteSensitive ) ) {
    string::size_type word_bound = m_iStart;
    string::size_type quote_bound = m_iStart;
    while( true ) {
      word_bound = m_sSource.find_first_of( m_sTokenBoundary, word_bound );
      quote_bound = m_sSource.find_first_of( "\"\'", quote_bound );
      if ( quote_bound == string::npos ) { 
	return word_bound;
      } else if ( quote_bound < word_bound ) { // if we found a quote
	char ch = m_sSource[ quote_bound ];
	quote_bound = m_sSource.find( ch, quote_bound+1 ); // advance to end of quote
	if ( quote_bound == string::npos ) { // if there is no end quote
	  m_sSource += ch;                   //   add one
	  return m_sSource.size();           //   return size
	}
	word_bound = quote_bound;
	++quote_bound;
      } else if ( word_bound < quote_bound)  {
	return word_bound;
      }
    }
    return word_bound;
  } else if ( interpretEscapedChars && (!quoteSensitive) ) {
    string::size_type size = m_sSource.size();
    string::size_type src = m_iStart;
    string::size_type dest = m_iStart;
    
    bool done = false;
    while( !done  && ( src < size ) ) {
      char c = m_sSource[ src ];
      if ( c == '\\' ) {
	char c2 = m_sSource[ ++src ];
	switch ( c2 ) {
	case 'n' :  // newline
	  m_sSource[ dest++ ] = '\n';
	  break;
	case 't' : // horizontal tab
	  m_sSource[ dest++ ] = '\t';
	  break;
	case 'v' : // vertical tab
	  m_sSource[ dest++ ] = '\v';
	  break;
	case 'b' : // backspace
	  m_sSource[ dest++ ] = '\b';
	  break;
	case 'r' : // carriage return
	  m_sSource[ dest++ ] = '\r';
	  break;
	case 'f' : // form feed
	  m_sSource[ dest++ ] = '\f';
	  break;
	case 'a' : // alert
	  m_sSource[ dest++ ] = '\a';
	  break;
	default : 
	  m_sSource[ dest++ ] = m_sSource[ src ];
	  break;
	} // end switch
	++src;
      } else if ( ( m_sTokenBoundary.find(c) != string::npos) || 
		  ( m_sLineBoundary.find(c) != string::npos ) ) {
	done = true;
      } else { // just copy the string
	m_sSource[ dest++ ] = c;
	++src;
      }
    } // end while( !done );
    for ( string::size_type k = dest; k < src ; ++k ) {
      m_sSource[k] = m_sTokenBoundary[0];
    }
    return dest;
  } else if ( interpretEscapedChars && quoteSensitive ) {
    string::size_type size = m_sSource.size();
    string::size_type src = m_iStart;
    string::size_type dest = m_iStart;
    bool quoteExact = false;
    bool quoteApprox = false;

    bool done = false;
    while( !done && ( src < size ) ) {
      char c = m_sSource[ src ];
      if ( c == '\'' ) { // copy exactly until end single quote
	quoteExact = !quoteExact;
	++src;
      } else if ( c == '\"' ) { // copy with interpretation until end double quote
	if ( !quoteExact ) {    // if not in exact quote mode
	  quoteApprox = !quoteApprox; // set to approx quote mode
	  ++src;
	} else {                // else if in exact quote mode
	  m_sSource[ dest++ ] = c;   // just copy the character
	  ++src;
	}
      } else if ( c == '\\' ) { 
	if ( quoteExact ) {
	  char c2 = m_sSource[ ++src ];
	  m_sSource[ dest++ ] = c;
	  m_sSource[ dest++ ] = c2;
	  ++src;
	} else {      //	if  (!quoteExact) ) { // else if not in exact quote mode
	  char c2 = m_sSource[ ++src ];
	  switch ( c2 ) {
	  case 'n' :  // newline
	    m_sSource[ dest++ ] = '\n';
	    break;
	  case 't' : // horizontal tab
	    m_sSource[ dest++ ] = '\t';
	    break;
	  case 'v' : // vertical tab
	    m_sSource[ dest++ ] = '\v';
	    break;
	  case 'b' : // backspace
	    m_sSource[ dest++ ] = '\b';
	    break;
	  case 'r' : // carriage return
	    m_sSource[ dest++ ] = '\r';
	    break;
	  case 'f' : // form feed
	    m_sSource[ dest++ ] = '\f';
	    break;
	  case 'a' : // alert
	    m_sSource[ dest++ ] = '\a';
	    break;
	  default : 
	    m_sSource[ dest++ ] = m_sSource[ src ];
	    break;
	  } // end switch
	  ++src;
	}
      } else if ( ( ( m_sTokenBoundary.find(c) != string::npos) || 
		    ( m_sLineBoundary.find(c) != string::npos ) ) && 
		  ( ! quoteExact ) && (! quoteApprox ) ) {
	done = true;
      } else { // just copy the string
	m_sSource[ dest++ ] = c;
	++src;
      }
    } // end while( !done );
    for ( string::size_type k = dest; k < src ; ++k ) {
      m_sSource[k] = m_sTokenBoundary[0];
    }
    return dest;
  }
  return m_iStart;
}






