//
//  spindle_func.h
//

#ifndef SPINDLE_FUNC_H_
#define SPINDLE_FUNC_H_

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include <pair.h>
#include <string.h>
#include "function.h"
#else
//#include <pair>
#include <string>
#include <functional>
#endif

SPINDLE_BEGIN_NAMESPACE

/** @name template binary predicates */

//@{

/**
 * A binary predicate for comparing pairs of items by its first entry only
 */
template< class T1, class T2 >
struct pair_first_less : public binary_function< pair<T1,T2>* , pair<T1,T2>*, bool > {
  bool operator()( const pair<T1,T2>& a, const pair<T1,T2>& b) const { 
    return a.first < b.first;
  }
};

/**
 * A binary predicate for comparing pairs of items by its first entry only
 */
template< class T1, class T2 >
struct pair_first_equal : public binary_function< pair<T1,T2>* , pair<T1,T2>*, bool > {
  bool operator()( const pair<T1,T2>& a, const pair<T1,T2>& b) const { 
    return a.first == b.first;
  }
};

/**
 * A binary predicate for comparing pairs of items by its second entry only
 */
template< class T1, class T2 >
struct pair_second_less : public binary_function< pair<T1,T2>* , pair<T1,T2>*, bool > {
  bool operator()( const pair<T1,T2>& a, const pair<T1,T2>& b) const { 
    return a.second < b.second;
  }
};

/**
 * A binary predicate for comparing pairs of items by its second entry only
 */
template< class T1, class T2 >
struct pair_second_equal : public binary_function< pair<T1,T2>* , pair<T1,T2>*, bool > {
  bool operator()( const pair<T1,T2>& a, const pair<T1,T2>& b) const { 
    return a.second == b.second;
  }
};

  
/**
 * A binary predicate for comparing pairs of items by its first entry first, and
 * then by its second if necessary
 */
template< class T1, class T2 >
struct pair_first_then_second_less : public binary_function< pair<T1,T2>* , pair<T1,T2>*, bool > {
  bool operator()( const pair<T1,T2>& a, const pair<T1,T2>& b) const { 
    if ( a.first == b.first ) {
      return a.second < b.second ;
    } else {
      return a.first < b.first;
    }
  }
};

/**
 * A binary predicate for testing the equality of pairs by testing the
 * first and second entries of each.
 */
template< class T1, class T2 >
struct pair_both_equal : public binary_function< pair<T1,T2>* , pair<T1,T2>*, bool > {
  bool operator()( const pair<T1,T2>& a, const pair<T1,T2>& b) const { 
    return (a.first == b.first) && (a.second == b.second );
  }
};

/**
 * A binary predicate for a case sensitive comparison of two zero 
 * terminated character strings 
 */
struct str_less : public binary_function<char*, char*, bool> {
  bool operator()(const char* x, const char* y) const { return (strcmp(x,y) < 0); }
};
 

/**
 * A binary predicate for a case insensitive comparison of two zero 
 * terminated character strings 
 */
struct str_case_less : public binary_function<char*, char*, bool> {
#ifndef _MSC_VER
  bool operator()(const char* x, const char* y) const { return (strcasecmp(x,y) < 0); }
#else
  bool operator()(const char* x, const char* y) const { return (_strnicmp(x,y,strlen(x)) < 0); }
#endif
};
//@}

SPINDLE_END_NAMESPACE
  
#endif
