//
//
//

#include "spindle/BinaryHeap.h"
#include <iostream.h>
#include <stdio.h>

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main() {
  ios::sync_with_stdio();
  int maxKey = 200;
  int maxItem = 100;
  BinaryHeap heap(maxKey, maxItem);

  cout << endl << "insert <50,5>" << endl;;
  heap.insert(50, 5);
  heap.print( stdout );

  cout << endl << "insert <10,1>" << endl;;
  heap.insert(10, 1);
  heap.print( stdout );

  cout << endl << "insert <40,4>" << endl;;
  heap.insert(40, 4);
  heap.print( stdout );

  cout << endl << "insert <70,7>" << endl;;
  heap.insert(70, 7);
  heap.print( stdout );

  cout << endl << "insert <30,3>" << endl;;
  heap.insert(30, 3);
  heap.print( stdout );

  cout << endl << "insert <60,6>" << endl;;
  heap.insert(60, 6);
  heap.print( stdout );
 
  cout << endl << "insert <20,2>" << endl;;
 heap.insert(20, 2);
  heap.print( stdout );

  cout << endl << "insert <80,8>" << endl;;
  heap.insert(80, 8);
  heap.print( stdout );

  cout << endl << "increment 1 by 80" << endl;;
  heap.incrementPriority(80,1);
  heap.print( stdout );
  
  cout << endl << "increment 2 by 80" << endl;;
  heap.incrementPriority(80,2);
  heap.print( stdout );
  
  cout << endl << "increment 3 by 80" << endl;;
  heap.incrementPriority(80,3);
  heap.print( stdout );

  cout << endl << "increment 4 by 5" << endl;;
  heap.incrementPriority(5,4);
  heap.print( stdout );

  cout << endl << "increment 4 by 5" << endl;;
  heap.incrementPriority(5,4);
  heap.print( stdout );

  cout << endl << "increment 4 by 5" << endl;;
  heap.incrementPriority(5,4);
  heap.print( stdout );

  cout << endl << "increment 4 by 5" << endl;;
  heap.incrementPriority(5,4);
  heap.print( stdout );

  cout << endl << "increment 4 by 5" << endl;;
  heap.incrementPriority(5,4);
  heap.print( stdout );

  cout << endl << "remove max " << heap.extractMax() << endl;
  heap.print( stdout );

  cout << endl << "remove max " << heap.extractMax() << endl;
  heap.print( stdout );

  cout << endl << "remove max " << heap.extractMax() << endl;
  heap.print( stdout );

  cout << endl << "remove max " << heap.extractMax() << endl;
  heap.print( stdout );
  
  cout << endl << "decrement 7 by 10" << endl;
  heap.decrementPriority(10,7);
  heap.print();

  cout << endl << "decrement 4 by 5" << endl;
  heap.decrementPriority(5,4);
  heap.print();

  cout << endl << "decrement 4 by 5" << endl;
  heap.decrementPriority(5,4);
  heap.print();
}
