//
//
//

#include <strstream.h>
#include <string.h>
#include <iostream.h>

class BaseClass { 
protected:
  bool error;           // a flag to know if the class is in an error state
  ostrstream * errMsg;  // a strstream to hold the error message (grows dynamically)
  BaseClass * offendingClass; // pointer to the offending class (if any)
public:
  BaseClass() : errMsg(0), offendingClass(0) { reset(); }
  virtual ~BaseClass() { delete errMsg; }
  virtual void reset() { // reset 
    if ( error == true ) { 
      error = false;
      if ( offendingClass != 0 ) { 
	offendingClass->reset();
	offendingClass = 0; // ? do we want to reset the offending class too?
      }
      if ( errMsg != 0 ) { // can't reset errMsg, so create a new one.
	delete errMsg; 
	errMsg = 0;
      } 
    }
    if ( errMsg == 0 ) { 
      errMsg = new ostrstream;
    }
  }
  BaseClass * getOffendingClass() { return offendingClass; }
  bool isError() const { return error == true; } // check if error is true.
  const char* getErrorMsg() const { return errMsg->str(); } // get error msg.
  void printErrorChain(ostream& out) {  // print the chain of errors
    if ( isError() ) {                  //   if I'm in an error state
      if ( offendingClass != 0 ) {      //      if its because of someone else
	offendingClass->printErrorChain( out ); // have them explain first
      }
      out << errMsg->str();             //   now explain why I'm in error
    }
  }
};

class DerivedOne : public BaseClass { 
public:
  
  virtual void reset() { BaseClass::reset(); }
  void makeError() { 
    error = true;
    (*errMsg) << "DerivedOne::makeError()" << endl 
	      << "Tried to do " << 2 << " many things! " << endl;
  }
  void makeError2() { 
    error = true;
    (*errMsg) << "DerivedOne::makeError2()" << endl 
	      << "This is a completely different kind of error" << endl;
  }
};

class DerivedTwo : public BaseClass { 
  DerivedOne one;
public:
  virtual void reset() { 
    BaseClass::reset(); 
    one.reset(); 
  }
  void makeError() { 
    one.makeError();
    if ( one.isError() ) { 
      offendingClass = &one;
      error = true;
      (*errMsg) << "DerivedTwo::makeError()" << endl 
		<< "Found something wrong with two" << endl;
    }
  }
  void makeError2() { 
    one.makeError2();
    if ( one.isError() ) { 
      offendingClass = &one;
      error = true;
      (*errMsg) << "DerivedTwo::makeError2()" << endl 
		<< "you get the idea" << endl;
    }
  }
};

int main() { 
  DerivedTwo test2;
  cout << "test2.isError() == " << test2.isError() << endl;
  test2.makeError();
  cout << "After makeError(), test2.isError() == " << test2.isError() << endl;
  cout << "---------Error Chain-----------" << endl;
  test2.printErrorChain(cout);
  cout << "-------------------------------" << endl;
  BaseClass * offender = test2.getOffendingClass();
  test2.reset();
  cout << "After reset(), test2.isError() == " << test2.isError() << endl;
  cout << "After reset(), offender->isError() == " << offender->isError() << endl;
  test2.makeError2();
  cout << "After makeError2(), test2.isError() == " << test2.isError() << endl;
  cout << "---------Error Chain-----------" << endl;
  test2.printErrorChain(cout);
  cout << "-------------------------------" << endl;
  test2.reset();
  cout << "After reset(), test2.isError() == " << test2.isError() << endl; 
}
