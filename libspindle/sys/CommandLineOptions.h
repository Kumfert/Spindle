//
// CommandLineOptions.h
//
// $Id: CommandLineOptions.h,v 1.2 2000/02/18 01:32:00 kumfert Exp $
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

#ifndef SPINDLE_COMMAND_LINE_OPTIONS_H_
#define SPINDLE_COMMAND_LINE_OPTIONS_H_
#include "spindle/spindle.h"

SPINDLE_BEGIN_NAMESPACE

/** 
 *  @memo (Deprecated) Simple Command Line Database see #OptionDatabase#
 *  @type class
 *
 *
 *
 *  This is a little class that handles searching through
 *  command line options.  Typically, only a single instance
 *  is created in #main(int argc, char* argv[])# and is created
 *  using #argc# and #argv#.  Then options are queried on demand
 *  rather than looping through the command line arguments at the
 *  beginning and remembering all kinds of configuration variables.
 *
 *  @see OptionDatabase
 *  @author Gary Kumfert 
 *  @version #$Id: CommandLineOptions.h,v 1.2 2000/02/18 01:32:00 kumfert Exp $#
 */

class CommandLineOptions {
private:
  /// same as #argc# from #main(argc,argv)#
  int nOptions;
  /// same as #argv# from #main(argc,argv)#
  char** Option;
  
  enum{ maxStrings=64 };
  char *String[maxStrings];

  enum{ buffer_size=256 };
  char buffer[256];
  
  int nStrings;

  void parseFlag( const char* flag );
  int findFlag( const char* flag );

public:

  /** @name constructor/destructors */
  //@{
  /**
   * constructor
   * @param argc use directly from main(argc,argv)
   * @param argv use directly from main(argc,argv)
   */
  CommandLineOptions(int argc,  char** argv): nOptions(argc), Option(argv) { buffer[0]='\0';}
  //@}

  /** @name accessors */
  //@{
  /**
   * @memo check for flag in database
   * @param flag should be a non-null pointer to a zero 
   *        terminated character string and can have logical
   *        #or# in the form of a '|' character in the string.
   * @return true iff a matching flag is found in argv
   */
  bool hasFlag( const char* flag );  

  /**
   * @memo return the value associated with a flag
   * @param flag as above
   * @return if #hasFlag( flag )#, it returns the next entry in #argv# 
   *         provided one exists.  
   *         0 otherwise;
   */     
  const char* getOption( const char* flag ); 

  /**
   * @memo return the #i#th value associated with a flag
   * @param flag as above
   * @param i non-negative integer
   * @return if #hasFlag( flag )#, it returns the #i#th successive entry in #argv# 
   *         provided one exists.  
   *         0 otherwise;
   */     
  const char* getOption( const char* flag, int i ); 

  /**
   * @memo check if a flag-string pair exist.
   * @param flag as above
   * @param option is a non-null pointer to a zero terminated character string
   * @return true if #strcmp( getOption(flag), option )==0#. false otherwise.
   */
  bool hasOption( const char* flag, const char* option );

  /**
   * @memo check if a flag-character pair exists.
   * @param flag as above
   * @param option is any character
   * @return true if #getOption(flag)[0] == option#. false otherwise.
   */
  bool hasOption( const char* flag, const char option ); 

  /**
   * @memo check if a flag-integer pair exists.
   * @param flag as above
   * @param option is any integer
   * @return true if #atoi(getOption(flag)) == option#. false otherwise.
   */
  bool hasOption( const char* flag, const int option );

  /**
   * @memo check if a flag-double pair exists.
   * @param flag as above
   * @param option is any double
   * @return true if #atof(getOption(flag)) == option# false otherwise
   */
  bool hasOption( const char* flag, const double option );
  //@}
};

SPINDLE_END_NAMESPACE

#endif
