//
// src/std/drivers/tests/test02.cc
//
// $Id: test02.cc,v 1.2 2000/02/18 01:32:10 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1998, Old Dominion University.  All rights reserved.
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
//  test Spindle Base class...
//

#include "spindle/SpindleBaseClass.h"
#include <iostream.h>

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

class Test1 : public SpindleBaseClass {
  SPINDLE_DECLARE_DYNAMIC( Test1 )
protected:
  int someInt;
  Test1() { someInt=0; }
public:
  Test1( int newInt ) : someInt(newInt) {}
  int getData() const { return someInt; }
  void setData(int newInt) { someInt = newInt; }
  virtual void dump(FILE* fp) const;
};

SPINDLE_IMPLEMENT_DYNAMIC( Test1, SpindleBaseClass )

void
Test1::dump( FILE* fp ) const {
  //  ::dump(fp);
  fprintf(fp,"Test1::someInt = %d\n", someInt);
}

class Test2 : public Test1 {
  SPINDLE_DECLARE_DYNAMIC( Test2 )
protected:
  int anotherInt;
  Test2() { anotherInt=2; }
public:
  Test2( int newInt ) : anotherInt(newInt) {}
  int getOtherData() const { return anotherInt; }
  void setOtherData(int newInt ) { anotherInt = newInt; }
  virtual void dump( FILE* fp ) const;
};

SPINDLE_IMPLEMENT_DYNAMIC( Test2, Test1 )

void
Test2::dump( FILE* fp ) const {
  // ::dump(fp);
  fprintf(fp,"Test2::someInt = %d\nTest2::anotherInt = %d\n", someInt, anotherInt);
}



class Exam1 : public SpindleBaseClass {
  SPINDLE_DECLARE_DYNAMIC( Exam1 )
private:
  float someFloat;
  Exam1() { someFloat=0.0; }
public:
  Exam1( float newFloat ) : someFloat( newFloat ) {}
  float getData() const { return someFloat; }
  void setData(float newFloat) { someFloat = newFloat; }
  virtual void dump(FILE* fp) const;
};

SPINDLE_IMPLEMENT_DYNAMIC( Exam1, SpindleBaseClass )

void
Exam1::dump( FILE* fp ) const {
  //  ::dump(fp);
  fprintf(fp,"Exam1::someFloat = %f\n", someFloat);
}


int main() {
  Test1 test1(5);
  const SpindleBaseClass* t1 = &test1;

  cout << "Is *t1 a kind of SpindleBaseClass ?   " << flush;
  const SpindleBaseClass* pBC;
  if ( ( pBC = SpindleBaseClass::dynamicCast( t1 ) ) != 0 ) {
    cout << "YES" << endl;
  } else {
    cout << "NO" << endl;
  }

  cout << "Is *t1 a kind of Test1 ?            " << flush;
  const Test1* pTest1;
  if ( ( pTest1 = Test1::dynamicCast( t1 ) ) != 0 ) {
    cout << "YES" << endl;
  } else {
    cout << "NO" << endl;
  }

  cout << "Is *t1 a kind of Test2 ?            " << flush;
  const Test2* pTest2;
  if ( ( pTest2 = Test2::dynamicCast( t1 ) ) != 0 ) {
    cout << "YES" << endl;
  } else {
    cout << "NO" << endl;
  }
  
  cout << "Is *t1 a kind of Exam1 ?            " << flush;
  const Exam1 *pExam1;
  if ( ( pExam1 = Exam1::dynamicCast( t1 ) ) != 0 ) {
    cout << "YES" << endl;
  } else {
    cout << "NO" << endl;
  }

  t1->dump( stdout );

  Test2 test2(5);
  test2.setData(1);
  const SpindleBaseClass* t2 = &test2;

  cout << "Is *t2 a kind of SpindleBaseClass ?   " << flush;
  if ( (pBC = SpindleBaseClass::dynamicCast( t2 ) ) != 0 ) {
    cout << "YES" << endl;
  } else {
    cout << "NO" << endl;
  }

  cout << "Is *t2 a kind of Test1 ?            " << flush;
  if ( (pTest1 = Test1::dynamicCast( t2 ) ) != 0 ) {
    cout << "YES" << endl;
  } else {
    cout << "NO" << endl;
  }

  cout << "Is *t2 a kind of Test2 ?            " << flush;
  if ( (pTest2 = Test2::dynamicCast( t2 ) ) != 0 ) {
    cout << "YES" << endl;
  } else {
    cout << "NO" << endl;
  }

  cout << "Is *t2 a kind of Exam1 ?            " << flush;
  if ( ( pExam1 = Exam1::dynamicCast( t2 ) ) != 0 ) {
    cout << "YES" << endl;
  } else {
    cout << "NO" << endl;
  }

  t2->dump( stdout );


}
