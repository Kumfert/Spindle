//
// FortranEmulator.h -- Emulates Fortran read/write routines. 
//
//  $Id: FortranEmulator.h,v 1.2 2000/02/18 01:31:51 kumfert Exp $
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

#ifndef SPINDLE_FORTRAN_EMULATOR_H_
#define SPINDLE_FORTRAN_EMULATOR_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include <stdio.h>
#else
#include <cstdio>
using namespace std;
#endif


SPINDLE_BEGIN_NAMESPACE

/**
 * @memo Has static function to emulate Fortran I/O
 * @type class
 *
 * Fortran has some peculiar I/O characteristics that
 * are not easily replicated in C/C++.  
 * 
 * This class has a few static functions that attempt
 * to emulate some Fortran formatted I/O conventions for 
 * arrays.  This class can correctly read in blocks
 * where there are no spaces between integers based on
 * the Fortran-style formatting specification.
 *
 * @author Gary Kumfert
 * @version #$Id: FortranEmulator.h,v 1.2 2000/02/18 01:31:51 kumfert Exp $#
 * @see HarwellBoeingFile
 */
class FortranEmulator { 
public:
  /**
   * read a formatted array of integers.
   * @param stream a conventional C/C++ #FILE# pointer
   * @param format a Fortran format specifier.  (e.g. 8I10=8 ints per line, 10 cols per int)
   * @param nLines The number of lines that the array is broken up over.
   * @param array The array where the data is to be read into.
   * @param arraySz The length of the array.
   * @return true iff the operation succeeds.
   */
  static bool readArray( FILE * stream, const char * format, const int nLines, 
			 int * array , const int arraySz);
  /**
   * read a formatted array of doubles.
   * @param stream a conventional C/C++ #FILE# pointer
   * @param format a Fortran format specifier.  (e.g. 8I10.3=8 doubles per line, 10 cols per double, 3 digits precision)
   * @param nLines The number of lines that the array is broken up over.
   * @param array The array where the data is to be read into.
   * @param arraySz The length of the array.
   * @return true iff the operation succeeds.
   */
  static bool readArray( FILE * stream, const char * format, const int nLines,
			 double * array, const int arraySz);
  
  /**
   * write a formatted array of integers.
   * @param stream a conventional C/C++ #FILE# pointer
   * @param format a Fortran format specifier.  (e.g. 8I10=8 ints per line, 10 cols per int)
   * @param nLines The number of lines that the array is broken up over.
   * @param array The array where the data is to be written from.
   * @param arraySz The length of the array.
   * @return true iff the operation succeeds.
   */
  static bool writeArray( FILE * stream, const char * format, const int nLines, 
			  const int * array, const int arraySz);
  /**
   * write a formatted array of doubles
   * @param stream a conventional C/C++ #FILE# pointer
   * @param format a Fortran format specifier.  (e.g. 8I10.3=8 doubles per line, 10 cols per doubles 3 digits precision)
   * @param nLines The number of lines that the array is broken up over.
   * @param array The array where the data is to be written from.
   * @param arraySz The length of the array.
   * @return true iff the operation succeeds.
   */
  static bool writeArray( FILE * stream, const char * format, const int nLines, 
			  const double * array, const int arraySz );
};

SPINDLE_END_NAMESPACE

#endif
