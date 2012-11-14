//
// OptionDatabase
//
// $Id: OptionDatabase.h,v 1.2 2000/02/18 01:32:01 kumfert Exp $
//
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
//========================================================================
//
//
//

#ifndef SPINDLE_OPTION_DATABASE_H_
#define SPINDLE_OPTION_DATABASE_H_

#include "spindle/spindle.h"

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include <stdio.h>
#include <iostream.h>
#include <map.h>
#include <vector.h>
#include <function.h>
#else
#include <iostream>
using std::ostream;
#include <cstdio>
#include <map>
using std::map;
#include <vector>
using std::vector;
//#include <functional>
#endif

#include <string>
using std::string;
//using namespace std;

SPINDLE_BEGIN_NAMESPACE

/**
 *  @memo configuration and option database
 *  @type class
 *
 *  This class maintains a list of unique #option#s.  Each #option#
 *  may have zero, one, or many #arguments#.   The #option#'s are stored
 *  in unsorted order, but the order of the #arguments# are sorted with
 *  newer arguments always added in the end.
 *  
 *  #option#s and #argument#s are of type #arg_t#, and the (possibly zero)
 *  list of #argument#s are stored in a list of type #arglist_t#.
 *  Access functions are provided that safely convert instances of
 *  #argumnent# to built in types like #int#, #float#, etc.
 *
 *  @author Gary Kumfert 
 *  @version #$Id: OptionDatabase.h,v 1.2 2000/02/18 01:32:01 kumfert Exp $#
 */
class OptionDatabase {
public:
  /** type used internally as arguments */
  typedef string arg_t;
  /** type used internally as lists of arguments */
  typedef vector<arg_t> arglist_t;
  /** unsigned int */
  typedef arglist_t::size_type size_type;
  /** type used for the database of options and arguments */
  typedef map< string, arglist_t > database_t;
  /** const iterator over the database */
  typedef database_t::const_iterator const_iterator;
  /** non-const iterator over the database */
  typedef database_t::iterator iterator;
  /** const iterator over the argument list of each option */
  typedef arglist_t::const_iterator const_arglist_iter;
  /** non-const iterator over the argument list of each option */
  typedef arglist_t::iterator arglist_iter;
  /** what to do if adding a new list of options to a database that has the option defined */
  typedef enum{ replace, append } collisionPolicy; 

protected:
  /**
   * The database itself
   */
  database_t database;

  collisionPolicy cPolicy;

  /**
   * @param option is a zero terminated character array that contains
   *        or ('|') separated list of indistinguishable arguments
   *        (e.g.   #option = "t|time"# ) where the first match in list is used.
   * @return const iterator to entry that matches or database.end() when no match or fail.
   */
  const_iterator findArgs( const char * option ) const;

  /**
   * @param option is a zero terminated character array that contains
   *        or ('|') separated list of indistinguishable arguments
   *        (e.g.   #option = "t|time"# )  where the first match in list is used.
   * @return iterator to entry that matches or database.end() when no match or fail.
   */
  iterator findArgs( const char * option );

public:
  /**
   * @memo Load command line options into database
   *
   * @param argc taken from #int main( int argc, char* argv[] )#
   * @param argv taken from #int main( int argc, char* argv[] )#
   * @param optionFlag any leading character to denote a new option
   *                 (usually #'-'# in UNIX and #'/'# in Windows)
   *                 all subseqent entries in #argv# that do not start
   *                 with this character are assumed to be arguments for the preceding option.
   * @param collisionPolicy if an option is encountered that already exists in the database, 
   *                      do we #replace# its existing arguments, or #append# them?  Default
   *                      is replace.
   * @return true iff no error detected
   */
  bool loadCommandLineOptions( int argc, char* argv[], char optionFlag = '-', collisionPolicy cp = replace );
  /**
   * load in a list of options.
   * 
   */
  friend istream& operator>>( istream& is, OptionDatabase & db );
  /**
   * used to set collisionPolicy during iostream activity, as in
   * #cin >> optionDatabase.appendOnCollision();#
   * 
   */
  OptionDatabase& appendOnCollision() { cPolicy = append; return *this; }

  /** 
   * bind an environment variable (if it exists to an argument)
   * 
   * @param env_var  zero terminated character array of the environment variable
   * @param option   zero terminated character array of the option name
   * @param arglist_sep zero terminated character array of character that separate
   *                  individual arguments in the argument list (ie #':'# in #PATH#
   *                  for UNIX)
   * @param collisionPolicy if the specified option already exists in the database, 
   *                      are the new arguments appended to previous ones or are
   *                      previous ones replaced by the new ones.
   * @return true iff no error detected
   */
  bool bindEnvironmentVariable( const char * env_var , const char * option, 
				const char * arglist_sep = " :", collisionPolicy cp = replace );

  /**
   * Add a single option to the database.
   *
   * If the option already exists and if the argument is not 0 or "", 
   * then the argument is appended to the list of arguments.
   * If the option already exists and the argument is 0 or "", nothing
   * is done.  If option does not exist, it is added to the database
   * and the argument (if it is not 0 or "") is added.
   *
   * @param option is a zero terminated character array
   * @param arg may be a zero terminated character array or 0 pointer for setting an option with no arguments
   * @return true iff no error detected
   */
  bool setOption( const char* option, const char* arg = 0 );

  /**
   * Add a single argument to an existing option.
   *
   * @param option is a zero terminated character array that already exists in the database.
   * @param arg is a zero terminated character array
   * @return true iff no error detected
   */
  bool addToOption( const char* option, const char* arg );

  /**
   * Remove the option and all arguments entirely from the database.
   * @param option is a zero terminated character array that already exists in the database.
   * @return true iff no error detected
   */
  bool unsetOption( const char* option );

  /**
   * Determine if an option, (or option/arg pair) exists in the database
   *
   * @param option a zero terminated character array that already exists in the database.
   *        May be an ('|') separated list of indistinguishable arguments
   *        (e.g.   #option = "t|time"# ) where the first match in list is used.
   * @param arg either a zero terminated character array or 0 pointer.
   * @return true iff match is found in database, false otherwise
   */
  bool hasOption( const char* option, const char* arg = 0 ) const;

  /**
   * Find the number of arguments associated with an option
   *
   * @param option a zero terminated character array that already exists in the database.
   *        May be an ('|') separated list of indistinguishable arguments
   *        (e.g.   #option = "t|time"# ) where the first match in list is used.
   * @return -1 iff the option is not found, 0 if there are no arguments, n=arglist.size() otherwise
   */

  int queryNArgs( const char* option ) const;
  /**
   * Find the number of arguments associated with an option
   *
   * @param option a zero terminated character array that already exists in the database.
   *        May be an ('|') separated list of indistinguishable arguments
   *        (e.g.   #option = "t|time"# ) where the first match in list is used.
   * @param arg_number the index from [0.. queryNArgs( option) ) of the option associated with that argument
   * @return argument if found, 0 if error.
   */
  const char * getOption( const char* option, int arg_number =0 ) const;

  /**
   * 
   * @memo Type-Safe Query
   * @return true iff no error detected.
   *
   * If argument exists, the following are evaluated to true
   * #"t", "T", "true", "True", "TRUE", "1"#.
   * The following are evaluated to false
   * #"f", "F", "false", "False", "FALSE", "0"#.
   * All others are invalid and cause an error, so bool_arg is undefined.
   *
   * @param option a zero terminated character array that already exists in the database.
   *        May be an ('|') separated list of indistinguishable arguments
   *        (e.g.   #option = "t|time"# ) where the first match in list is used.
   * @param bool_arg result, not defined if returns false.
   * @param arg_number the index from [0.. queryNArgs( option) ) of the option associated with that argument
   */
  bool queryOption( const char * option, bool* bool_arg, int arg_number = 0 ) const;

  /**
   * @memo Type-Safe Query
   *
   * @param option a zero terminated character array that already exists in the database.
   *        May be an ('|') separated list of indistinguishable arguments
   *        (e.g.   #option = "t|time"# ) where the first match in list is used.
   * @param char_arg result, not defined if returns false.
   * @param arg_number the index from [0.. queryNArgs( option) ) of the option associated with that argument
   * @return true iff no error detected.
   */
  bool queryOption( const char * option, char* char_arg, int arg_number = 0 ) const;

  /**
   * @memo Type-Safe Query
   *
   * @param option a zero terminated character array that already exists in the database.
   *        May be an ('|') separated list of indistinguishable arguments
   *        (e.g.   #option = "t|time"# ) where the first match in list is used.
   * @param short_arg result, not defined if returns false.
   * @param arg_number the index from [0.. queryNArgs( option) ) of the option associated with that argument
   * @return true iff no error detected.
   */

  bool queryOption( const char * option, short* short_arg, int arg_number = 0 ) const;
  /**
   * @memo Type-Safe Query
   *
   * @param option a zero terminated character array that already exists in the database.
   *        May be an ('|') separated list of indistinguishable arguments
   *        (e.g.   #option = "t|time"# ) where the first match in list is used.
   * @param ushort_arg result, not defined if returns false.
   * @param arg_number the index from [0.. queryNArgs( option) ) of the option associated with that argument
   * @return true iff no error detected.
   */
  bool queryOption( const char * option, unsigned short* ushort_arg, int arg_number = 0 ) const;

  /**
   * @memo Type-Safe Query
   *
   * @param option a zero terminated character array that already exists in the database.
   *        May be an ('|') separated list of indistinguishable arguments
   *        (e.g.   #option = "t|time"# ) where the first match in list is used.
   * @param int_arg result, not defined if returns false.
   * @param arg_number the index from [0.. queryNArgs( option) ) of the option associated with that argument
   * @return true iff no error detected.
   */
  bool queryOption( const char * option, int* int_arg, int arg_number = 0 ) const;

  /**
   * @memo Type-Safe Query
   *
   * @param option a zero terminated character array that already exists in the database.
   *        May be an ('|') separated list of indistinguishable arguments
   *        (e.g.   #option = "t|time"# ) where the first match in list is used.
   * @param uint_arg result, not defined if returns false.
   * @param arg_number the index from [0.. queryNArgs( option) ) of the option associated with that argument
   * @return true iff no error detected.
   */
  bool queryOption( const char * option, unsigned int* uint_arg, int arg_number = 0 ) const;

  /**
   * @memo Type-Safe Query
   *
   * @param option a zero terminated character array that already exists in the database.
   *        May be an ('|') separated list of indistinguishable arguments
   *        (e.g.   #option = "t|time"# ) where the first match in list is used.
   * @param long_arg result, not defined if returns false.
   * @param arg_number the index from [0.. queryNArgs( option) ) of the option associated with that argument
   * @return true iff no error detected.
   */
  bool queryOption( const char * option, long* long_arg, int arg_number = 0 ) const;

  /**
   * @memo Type-Safe Query
   *
   * @param option a zero terminated character array that already exists in the database.
   *        May be an ('|') separated list of indistinguishable arguments
   *        (e.g.   #option = "t|time"# ) where the first match in list is used.
   * @param ulong_arg result, not defined if returns false.
   * @param arg_number the index from [0.. queryNArgs( option) ) of the option associated with that argument
   * @return true iff no error detected.
   */
  bool queryOption( const char * option, unsigned long* ulong_arg, int arg_number = 0 ) const;

  /**
   * @memo Type-Safe Query
   *
   * @param option a zero terminated character array that already exists in the database.
   *        May be an ('|') separated list of indistinguishable arguments
   *        (e.g.   #option = "t|time"# ) where the first match in list is used.
   * @param float_arg result, not defined if returns false.
   * @param arg_number the index from [0.. queryNArgs( option) ) of the option associated with that argument
   * @return true iff no error detected.
   */
  bool queryOption( const char * option, float* float_arg, int arg_number = 0 ) const;

  /**
   * @memo Type-Safe Query
   *
   * @param option a zero terminated character array that already exists in the database.
   *        May be an ('|') separated list of indistinguishable arguments
   *        (e.g.   #option = "t|time"# ) where the first match in list is used.
   * @param double_arg result, not defined if returns false.
   * @param arg_number the index from [0.. queryNArgs( option) ) of the option associated with that argument
   * @return true iff no error detected.
   */
  bool queryOption( const char * option, double* double_arg, int arg_number = 0 ) const;

  /**
   * @memo Type-Safe Query
   *
   * @param option a zero terminated character array that already exists in the database.
   *        May be an ('|') separated list of indistinguishable arguments
   *        (e.g.   #option = "t|time"# ) where the first match in list is used.
   * @param ldouble_arg result, not defined if returns false.
   * @param arg_number the index from [0.. queryNArgs( option) ) of the option associated with that argument
   * @return true iff no error detected.
   */
  bool queryOption( const char * option, long double* ldouble_arg, int arg_number = 0 ) const;

  /** 
   */
  friend ostream& operator<<( ostream& os, OptionDatabase const & db );

  /**
   * Iterate over the list of options.
   */
  const_iterator options_begin() const { return database.begin(); }
  /**
   * Iterate over the list of options.
   */
  const_iterator options_end() const { return database.end(); }

};

  
SPINDLE_END_NAMESPACE

#endif
