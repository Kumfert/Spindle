//
// OptionDatabase.cc
//
// $Id: OptionDatabase.cc,v 1.3 2000/03/07 22:22:09 kumfert Exp $
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

#include "spindle/OptionDatabase.h"
#include "spindle/Tokenizer.h"


#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include "algo.h"  // define find()
#else
#include <algorithm>
using std::find;
#endif

extern "C" { 
#include <string.h>
}

#if defined ( NT ) || defined ( WINNT ) || defined ( WIN95 ) || defined ( WIN32 )
#define MICROSOFT
extern "C" {
#include <direct.h> // include directory commands
#include <io.h>   // _finddata_t
#include <stdlib.h>
#include <stddef.h>
}
#else 
#define UNIX
#   include <sys/types.h>
#   include <sys/stat.h>
#   include <dirent.h>
#   include <unistd.h>
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

OptionDatabase::const_iterator
OptionDatabase::findArgs( const char * option ) const {  
  string str_option( option );
  
  string::size_type prev = 0;
  string::size_type next = 0;
  do { 
    next = str_option.find_first_of("|",prev );
    string possible_option = str_option.substr( prev, next );
    const_iterator it = database.find( possible_option );
    if ( it != database.end() ) { 
      return it;
    }
    prev = next + 1;
  } while ( next != string::npos );
  return database.end();
}


OptionDatabase::iterator
OptionDatabase::findArgs( const char * option ) {  
  string str_option( option );
  
  string::size_type prev = 0;
  string::size_type next = 0;
  do { 
    next = str_option.find_first_of("|",prev );
    string possible_option = str_option.substr( prev, next );
    iterator it = database.find( possible_option );
    if ( it != database.end() ) { 
      return it;
    }
    prev = next + 1;
  } while ( next != string::npos );
  return database.end();
}


bool
OptionDatabase::bindEnvironmentVariable( const char * env_var , const char * option, 
					 const char * arglist_sep, collisionPolicy cp ) {
  if ( env_var == 0 || env_var[0] == '\0') { return false; }
  if ( option == 0 || option[0] == '\0' ) { return false; }
  const char * result = getenv( env_var );
  if ( result == 0 ) { return false; }
  if ( cp == replace ) { 
    unsetOption( option );
  }
  
  if ( arglist_sep == 0 || arglist_sep[0] == '\0' ) { // if no arglist separators specified
    // just add the argument
    setOption( option, result );
  } else {  // if there are arglist separators specified
  // parse the argument for list of arguments
    string str_result( result );
    string::size_type prev = 0;
    for( string::size_type next = 0;  // init 'next' = 0;
	 next != string::npos;        // loop while 'next' != end of loop
	 next = str_result.find_first_of( arglist_sep,(prev=next+1) ) 
	   // advance 'prev' to 'next'+1  & 'next' to next sep
	 ) {
      string str_arg = str_result.substr( prev, (next==prev) ? string::npos : next-prev );
      addToOption( option, str_arg.c_str() );
    }
  }
  return true;
}

bool
OptionDatabase::loadCommandLineOptions( int argc, char* argv[], char optionFlag, collisionPolicy cp ) { 
  if ( argc < 0 ) { return false; }
  if ( argv == 0 ) { return false; }
  string option;
  string argument;
  for ( int i = 1; i<argc; ++i ) { // NOTE: 'i' starts at 1 since argv[0] is the executeable
    if ( (argv[i] == 0) || (strlen(argv[i])<1) ) { // if argv[i] is null or empty...
      option = "";                                 // ... skip it
      argument = "";
      continue;                                    
    } else if ( argv[i][0] == optionFlag )  {      
      // else if argv[i] has an optionFlag in the first character...
      // NOTE: if optionFlag == '-', there is a chance that this is really a negative number
      if ( (optionFlag == '-') && (argv[i][1] >= '0') &&  (argv[i][1] <= '9' ) ) { 
	// if argv[i][1] is numeric
	argument = argv[i];
      } else {  // else argv[i] is an option
	int skipPrefix = 1;
	// now check for a "--" prefix
	if ( argv[i][1] == optionFlag ) { 
	  skipPrefix=2;
	}
	argument="";
	// now check for an '=' in the option
	char * c;
	if ( ( c = strchr( argv[i]+skipPrefix , '=' ) ) != NULL ) { 
	  // found an assignment in the option
	  *c = '\0'; //switch '=' to '\0', thereby splitting the arg
	  argument = c+1; // assign argument
	}
	option = argv[i]+skipPrefix;       // set string to option, but omit the optionFlag
	if ( cp == replace ) { 
	  unsetOption( option.c_str() ); // remove the option and its args if it already existed
	}
	setOption( option.c_str() );
      }
    } else { // if argv[i] != optionFlag
      argument = argv[i];
    }
    if ( ( option.compare( "" ) != 0 ) && ( argument.compare( "" ) != 0 ) ) { // if option and arg not empty
      addToOption( option.c_str(), argument.c_str() );                        // add to option
      argument = "";                                                          // reset the argument
    }
  } // end for
  return true;
}

bool
OptionDatabase::setOption( const char* option, const char* arg ) { 
  if ( option == 0 || option[0] == '\0' ) { return false; }  // fail if option string is 0 or "" 
  string str_option( option );          // create C++ string representation
  if ( str_option.find_first_of("|") != string::npos ) { return false; } // fail if or's "|" are here
  if ( arg == 0 || arg[0] == '\0' ) { 
    database[ str_option ];
  } else { 
    database[ str_option ].push_back( string( arg ) );
  }
  return true;
}

bool
OptionDatabase::addToOption( const char* option, const char* arg ) { 
  if ( option == 0 || option[0] == '\0' ) { return false; }  // fail if option string is 0 or "" 
  if ( arg == 0 || arg[0] == '\0' ) { return false; }  // fail if arg string is 0 or "" 
  string str_option( option );          // create C++ string representation
  iterator it = database.find( str_option );   // search if the option is already here.
  if ( it == database.end() ) { return false; } // if not, then fail.
  (*it).second.push_back( string( arg ) );
  return true;
}


bool
OptionDatabase::unsetOption( const char* option ) { 
  if ( option == 0 || option[0] == '\0' ) { return false; }  // fail if option string is 0 or ""
  string str_option( option );          // create C++ string representation
  if ( str_option.find_first_of("|") != string::npos ) { return false; } // fail if or's "|" are here
  iterator it = database.find( str_option );
  if ( it == database.end() ) { 
    // no such entry found
    return false;
  } else { 
    // remove the entry.
    database.erase( it );
    return true;
  }
}

bool
OptionDatabase::hasOption( const char* option, const char* arg ) const { 
  if ( option == 0 ) { return false; }               // fail if option == 0;
  const_iterator it = findArgs( option );            // search for requested option
  if ( it == database.end() ) { return false; }      // fail if option not found.
  if ( arg == 0  || arg[0] == '\0' ) { return true; }// succeed if no specific argument is specified.
  string str_arg( arg ); 
  const arglist_t & str_list = (*it).second;
  if ( find( str_list.begin(), str_list.end(), str_arg ) != str_list.end() ) { 
    return true;
  } else { 
    return false;
  }
}

int
OptionDatabase::queryNArgs( const char * option ) const { 
  if ( option == 0 ) { return -1; }
  const_iterator it = findArgs( option );
  if ( it == database.end() ) { return -1; }
  return (*it).second.size();
}

const char * 
OptionDatabase::getOption( const char* option,  int arg_number ) const { 
  if ( option == 0 ) { return 0; }  // fail if option string is 0
  const_iterator it = findArgs( option );
  if ( it == database.end() ) { return 0; }
  if ( (*it).second.size() < (size_type) arg_number ) { return 0; }
  return (*it).second[ arg_number ].c_str();
}

bool OptionDatabase::queryOption( const char * option, bool* bool_arg, int arg_number ) const { 
  const char * arg = getOption( option, arg_number );
  if ( arg == 0 || arg[0] == '\0' ) { return false; }
  string str_arg( arg );
  if ( str_arg.compare( "t" ) == 0 ) { *bool_arg = true; return true; }
  else if ( str_arg.compare( "T" ) == 0 ) { *bool_arg = true; return true; }
  else if ( str_arg.compare( "true" ) == 0 ) { *bool_arg = true; return true; }
  else if ( str_arg.compare( "True" ) == 0 ) { *bool_arg = true; return true; }
  else if ( str_arg.compare( "TRUE" ) == 0 ) { *bool_arg = true; return true; }
  else if ( str_arg.compare( "1" ) == 0 ) { *bool_arg = true; return true; }
  else if ( str_arg.compare( "f" ) == 0 ) { *bool_arg = false; return true; }
  else if ( str_arg.compare( "F" ) == 0 ) { *bool_arg = false; return true; }
  else if ( str_arg.compare( "false" ) == 0 ) { *bool_arg = false; return true; }
  else if ( str_arg.compare( "False" ) == 0 ) { *bool_arg = false; return true; }
  else if ( str_arg.compare( "FALSE" ) == 0 ) { *bool_arg = false; return true; }
  else if ( str_arg.compare( "0" ) == 0 ) { *bool_arg = false; return true; }
  return false; 
}

bool OptionDatabase::queryOption( const char * option, char* char_arg, int arg_number ) const { 
  const char * arg = getOption( option, arg_number );
  if ( arg == 0 || arg[0] == '\0' ) { return false; }
  int count;
  int n = sscanf( arg, "%c%n", char_arg, &count );
  if ( n == 0 || n == EOF || (size_t)count != strlen( arg ) ) { return false; }
  return true;
}

bool OptionDatabase::queryOption( const char * option, short* short_arg, int arg_number ) const { 
  const char * arg = getOption( option, arg_number );
  if ( arg == 0 || arg[0] == '\0' ) { return false; }
  int count;
  int n = sscanf( arg, "%hd%n", short_arg , &count); // signed base 10 short
  if ( n == 0 || n == EOF || (size_t)count !=strlen( arg ) ) { return false; }
  return true;
}

bool OptionDatabase::queryOption( const char * option, unsigned short* ushort_arg, int arg_number ) const { 
  const char * arg = getOption( option, arg_number );
  if ( arg == 0 || arg[0] == '\0' ) { return false; }
  int count;
  int n = sscanf( arg, "%hu%n", ushort_arg , &count); // unsigned base 10 short
  if ( n == 0 || n == EOF || (size_t)count !=strlen( arg ) ) { return false; }
  return true;
}

bool OptionDatabase::queryOption( const char * option, int* int_arg, int arg_number ) const { 
  const char * arg = getOption( option, arg_number );
  if ( arg == 0 || arg[0] == '\0' ) { return false; }
  int count;
  int n = sscanf( arg, "%d%n", int_arg , &count); // signed base 10 int
  if ( n == 0 || n == EOF || (size_t)count !=strlen( arg ) ) { return false; }
  return true;
}

bool OptionDatabase::queryOption( const char * option, unsigned int* uint_arg, int arg_number ) const { 
  const char * arg = getOption( option, arg_number );
  if ( arg == 0 || arg[0] == '\0' ) { return false; }
  int count;
  int n = sscanf( arg, "%u%n", uint_arg , &count); // unsigned base 10 int
  if ( n == 0 || n == EOF || (size_t)count !=strlen( arg ) ) { return false; }
  return true;
}

bool OptionDatabase::queryOption( const char * option, long* long_arg, int arg_number ) const {   
  const char * arg = getOption( option, arg_number );
  if ( arg == 0 || arg[0] == '\0' ) { return false; }
  int count;
  int n = sscanf( arg, "%ld%n", long_arg , &count); // signed base 10 long
  if ( n == 0 || n == EOF || (size_t)count !=strlen( arg ) ) { return false; }
  return true;
}

bool OptionDatabase::queryOption( const char * option, unsigned long* ulong_arg, int arg_number ) const { 
  const char * arg = getOption( option, arg_number );
  if ( arg == 0 || arg[0] == '\0' ) { return false; }
  int count;
  int n = sscanf( arg, "%lu%n", ulong_arg , &count); // unsigned base 10 long
  if ( n == 0 || n == EOF || (size_t)count !=strlen( arg ) ) { return false; }
  return true;
}
bool OptionDatabase::queryOption( const char * option, float* float_arg, int arg_number ) const {
 const char * arg = getOption( option, arg_number );
  if ( arg == 0 || arg[0] == '\0' ) { return false; }
  int count;
  int n = sscanf( arg, "%e%n", float_arg , &count); // float
  if ( n == 0 || n == EOF || (size_t)count !=strlen( arg ) ) { return false; }
  return true;
}

bool OptionDatabase::queryOption( const char * option, double* double_arg, int arg_number ) const { 
 const char * arg = getOption( option, arg_number );
  if ( arg == 0 || arg[0] == '\0' ) { return false; }
  int count;
  int n = sscanf( arg, "%le%n", double_arg , &count); // double
  if ( n == 0 || n == EOF || (size_t)count !=strlen( arg ) ) { return false; }
  return true;
}

bool OptionDatabase::queryOption( const char * option, long double* ldouble_arg, int arg_number ) const { 
 const char * arg = getOption( option, arg_number );
  if ( arg == 0 || arg[0] == '\0' ) { return false; }
  int count;
  int n = sscanf( arg, "%Le%n", ldouble_arg , &count); // long double
  if ( n == 0 || n == EOF || (size_t)count !=strlen( arg ) ) { return false; }
  return true;
}

ostream& 
#ifdef HAVE_NAMESPACES
SPINDLE_NAMESPACE::
#endif 
operator<<( ostream& os,  OptionDatabase const & db ) { 
  for ( OptionDatabase::const_iterator it = db.database.begin(), stop_it = db.database.end(); it != stop_it; ++it ) { 
    if ( (*it).first.find_first_of(" \t") == string::npos ) { 
      os << (*it).first <<  " = ";
    } else { 
      os << "\"" << (*it).first << "\" = " ;
    }
    const OptionDatabase::arglist_t & arg_list = (*it).second;
    for ( OptionDatabase::arglist_t::const_iterator cur = arg_list.begin(), stop_cur = arg_list.end(); 
	  cur != stop_cur; ++cur ) { 
      if ( cur->find_first_of(" \t") == string::npos ) { 
	os << *cur  << " ";
      } else { 
	os << "\"" << *cur << "\" ";
      }
    }
    os << "; " << endl;
  }
  return os;
}

istream& 
#ifdef HAVE_NAMESPACES 
SPINDLE_NAMESPACE::
#endif
operator>>( istream& is,  OptionDatabase & db ) { 
  OptionDatabase::collisionPolicy cp = db.cPolicy;// copy cPolicy
  db.cPolicy = OptionDatabase::replace;           // reset cPolicy to replace
  Tokenizer tokenizer;                        // Create a tokenizer
  tokenizer.setLineBound( "\n#" );            //    set carriage return or # as line bound.
  // tokenizer.setInterpretEscapedChars( true ); //    interpret escaped characters
  tokenizer.setQuoteSensitive( true );        //     be sensitive about quotes
  string option;                            // initialize an option string to empty
  while( ! (is.eof() || is.fail() )  ) {      // while inputstream is not empty.
    string buffer; 
    getline( is, buffer );                    // dump a line into the buffer string.
    tokenizer.setSource( buffer );            // attach the tokenizer to the buffer string
    // for( tokenizer.nextToken(); (!tokenizer.eos()) && (!tokenizer.eoln()); tokenizer.nextToken() ) {
    while ( tokenizer.nextToken() ) { 
      string token = tokenizer.getValue();  // for all tokens in the current line
      if ( token.compare( ";" )  == 0 )  {      // if token == ";", then consider this the end of line
	option = "";                            //    reset option
	break;                                  //    consider this the end of line
      } else if ( token.compare( "=" ) == 0 ) { // else if token == "="
	if ( option.size() == 0 ) {             //    if option is not yet specified
	  break;                                //       error, should have an option before an "="
	} else {                                //    else 
	  if ( db.queryNArgs( option.c_str() ) > 0 ) { 
	    string temp = db.database[option].back();//       error, take last arg off option
	    if ( temp.size() != 0 ) {             //   if this last arg is not empty
	      db.database[option].pop_back();     //      remove that option from arglist of old option
	      option = temp;                      //      it was supposed to be an option
	      db.database[ option ];              //      add the option to the database.
	    }
	  }
	}
      } else if ( option.size() == 0 ) {        // else if no option is yet specified
	if ( token[0] == '\'' || token[0] == '\"' ) {  // " if token has quotes leading quotes
	  token = token.substr(1,token.size()-2);//     remove quotes
	} 
	option = token;                          //     set option to token
	if ( cp == OptionDatabase::replace ) { 
	  db.unsetOption( option.c_str() );       //      remove any previous options
	}
	db.database[ option ];                  //      insert into database
      } else {                                  // else if an option is specified                  
	if ( token[0] == '\'' || token[0] == '\"' ) { // " if token has a leading quote
	  token = token.substr(1,token.size()-2);     // remove front and end quotes
	} 
	db.database[ option ].push_back(token); //      add token as an option
      }
    } // end for all tokens on a line
  }// end while lines exist in is
  return is;
}

