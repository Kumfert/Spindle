//
// SPINDLE_ALGO.H -- Extensions to STL Generic Algorithms
//
// 
//
#ifndef SPINDLE_ALGO_H_
#define SPINDLE_ALGO_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

//
// bool nonempty_set_intersection()  returns true iff the intersection 
// between two sorted sequences is non-empty
//

SPINDLE_BEGIN_NAMESPACE

/** @name template algorithms */
//@{
/**
 * return true iff the intersection between two sorted sequences is nonempty
 * @type function
 * @arg first1 InputIterator pointing to the beginning of the first sequence
 * @arg last2 InputIterator pointing to one past the end of the first sequence
 * @arg first2 InputIterator pointing to the beginning of the second sequence
 * @arg last2 InputIterator pointing to one past the end of the second sequence
 */
template< class InputIterator1, class InputIterator2>
bool
nonempty_set_intersection( InputIterator1 first1, InputIterator2 last1,
			   InputIterator2 first2, InputIterator2 last2 ) {
  while( (first1 != last1) && (first2 != last2) ) {
    if ( *first1 < *first2 ) { ++first1;  }
    else if ( *first2 < *first1 ) { ++first2;}
    else  { return true; }
  }
  return false;
}

/**
 * return true iff the intersection between two sorted sequences is nonempty
 * @type function
 * @arg first1 InputIterator pointing to the beginning of the first sequence
 * @arg last2 InputIterator pointing to one past the end of the first sequence
 * @arg first2 InputIterator pointing to the beginning of the second sequence
 * @arg last2 InputIterator pointing to one past the end of the second sequence
 * @arg comp is a binary predicate like less<T> 
 */
template< class InputIterator1, class InputIterator2, class Compare>
bool
nonempty_set_intersection( InputIterator1 first1, InputIterator2 last1,
			   InputIterator2 first2, InputIterator2 last2,
			   Compare comp ) {
  while( (first1 != last1) && (first2 != last2) ) {
    if( comp(*first1,*first2) ) { ++first1; }
    else if ( comp(*first2,*first1) ) { ++first2;  }
    else { return true; }
  }
  return false;
}

/**
 * return true iff the the second sorted sequence is completely contained in the first
 * @type function
 * @arg first1 InputIterator pointing to the beginning of the first sequence
 * @arg last2 InputIterator pointing to one past the end of the first sequence
 * @arg first2 InputIterator pointing to the beginning of the second sequence
 * @arg last2 InputIterator pointing to one past the end of the second sequence
 */
template< class InputIterator1, class InputIterator2>
bool
contains( InputIterator1 first1, InputIterator2 last1,
	  InputIterator2 first2, InputIterator2 last2 ) {
  while( (first1 != last1) && (first2 != last2) ) {
    if ( *first1 < *first2 ) { ++first1; }
    else if ( *first2 < *first1 ) { return false; }
    else  { ++first1; ++first2; }
  }
  return (first2 == last2 );
}

/**
 * return true iff the the second sorted sequence is completely contained in the first
 * @type function
 * @arg first1 InputIterator pointing to the beginning of the first sequence
 * @arg last2 InputIterator pointing to one past the end of the first sequence
 * @arg first2 InputIterator pointing to the beginning of the second sequence
 * @arg last2 InputIterator pointing to one past the end of the second sequence
 * @arg comp is a binary predicate like less<T> 
 */
template< class InputIterator1, class InputIterator2, class Compare>
bool
contains( InputIterator1 first1, InputIterator2 last1,
	  InputIterator2 first2, InputIterator2 last2,
	  Compare comp ) {
  while( (first1 != last1) && (first2 != last2) ) {
    if( comp(*first1,*first2) ) { ++first1; }
    else if ( comp(*first2,*first1) ) { return false; }
    else { ++first1; ++first2; }
  }
  return (first2 == last 2);
}
//@}

SPINDLE_END_NAMESPACE

#endif


