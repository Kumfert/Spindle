//
//
//

#ifndef SPINDLE_TRIPLE_H_
#define SPINDLE_TRIPLE_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

SPINDLE_BEGIN_NAMESPACE

template < class T1, class T2, class T3 >
struct triple { 
  typedef T1 first_type;
  typedef T2 second_type;
  typedef T3 third_type;

  T1 first;
  T2 second;
  T3 third;
  triple() : first(T1()), second(T2()), third(T3()) {}
  triple( const T1& a, const T2& b, const T3& c ) : first(a), second(b), third(c) {}

#ifdef HAVE_MEMBER_TEMPLATES
  template< class U1, class U2, class U3 >
  triple( const triple<U1, U2, U3>& p ) : 
    first(p.first), second(p.second), third(p.third) {}
#endif
};

template <class T1, class T2, class T3>
inline bool operator==(const triple<T1, T2, T3>& x, const triple<T1, T2, T3>& y) { 
  return x.first == y.first && x.second == y.second && x.third == y.third; 
}
 
template <class T1, class T2, class T3>
inline bool operator<(const triple<T1, T2, T3>& x, const triple<T1, T2, T3>& y) { 
  return ( x.first < y.first ||  (!(y.first < x.first) && x.second < y.second) ) || 
     ( !(y.first < x.first) && !(y.second < x.second) && ( x.third < y.third) ) ;
}
 
template <class T1, class T2, class T3>
inline triple<T1, T2, T3> make_triple(const T1& x, const T2& y, const T3& z ) {
  return triple<T1, T2, T3>(x, y, z);
}

SPINDLE_END_NAMESPACE

#endif
