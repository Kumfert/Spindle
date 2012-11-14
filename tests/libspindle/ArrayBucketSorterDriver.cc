//
// ArrayBucketSorterDriver.cc
//


#include <iostream>
using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::flush;

#include <string>
using std::string;

#include "spindle/ArrayBucketSorter.h"

const char prompt[] = "==> ";

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

void parsecommand( string& , string&, string&, string& );

int main( int argc, char * argv[] ) {

  const char main_menu[] =  
  "MAIN MENU\n"
  "BucketSorter *sorter;\n"
  " 1. sorter = new ArrayBucketSorter(int nBucketsMax, int nItemsMax);\n"
  " 2. void sorter->validate();\n"
  " 3. bool sorter->isValid() const; \n"
  " 4. void sorter->dump( stdout ) const; \n"
  " 5. bool sorter->isEmpty() const; \n"
  " 6. int  sorter->size() const; \n"
  " 7. int  sorter->queryMinBucket() const; \n" 
  " 8. int  sorter->queryNextBucket( const int Bucket ) const; \n"
  " 9. int  sorter->queryFirstItem( const int Bucket ) const; \n"
  "10. int  sorter->queryNextItem( const int Item ) const; \n"
  "11. int  sorter->queryPrevItem( const int Item ) const; \n"
  "12. int  sorter->queryKey( const int Item ) const; \n"
  "13. int  sorter->queryBucket( const int Item ) const ; \n"
  "14. int  sorter->queryNInserted() const; \n"
  "15. bool  sorter->insert( const int Key, const int Item );\n"
  "16. void sorter->setInsertFront();\n"
  "17. void sorter->setInsertBack();\n"
  "18. bool sorter->remove( const int Item );\n"
  "19. bool sorter->reset();\n"
  "20. void sorter->printDebug( stdout );\n"
  " 0. quit\n";

  ArrayBucketSorter * sorter =0;

  cout << main_menu << endl;
  for(;;) {
    string command;
    cout << prompt << flush; 
    getline( cin, command );
    string word1("");
    string word2("");
    string word3("");
    parsecommand( command, word1, word2, word3 );
    if ( word1 == "" ) { 
      ; // do nothing
    } else if ( word1 == "0" || word1 == "quit" || word1 == "q" 
		|| word1 == "exit" || word1 == "done" ) { 
      if ( sorter != 0 ) { 
	delete sorter;
      }
      break;
    } else if ( word1 == "h" || word1 == "?" || word1 == "help" ) { 
      cout << main_menu << endl;
    } else if ( word1 == "1" ) { 
      int nBucketsMax = 0;
      int nItemsMax = 0;
      if ( word2 != "" ) { 
	nBucketsMax = atoi( word2.c_str());
      } 
      if ( nBucketsMax == 0 ) {
	cout << "nBucketsMax " << prompt << flush;
	cin >> nBucketsMax;
      }
      if ( word3 != "" ) { 
	nItemsMax = atoi( word3.c_str() );
      } 
      if ( nItemsMax == 0 ) { 
	cout << "nItemsMax " << prompt << flush;
	cin >> nItemsMax;
      }
      cout << "sorter = new ArrayBucketSorter(" 
	   << nBucketsMax << ", " << nItemsMax << " );" << endl;
      if ( sorter != 0 ) { 
	delete sorter;
      }
      sorter = new ArrayBucketSorter( nBucketsMax, nItemsMax );
    } else if ( sorter == 0 ) { 
      cout << "ERROR: attempted to dereference null pointer" << endl;
    } else if ( word1 == "2" ) { 
      sorter->validate();
      cout << "void sorter->validate();" << endl;
    } else if ( word1 == "3" ) { 
      cout << ( sorter->isValid()  ? "true" : "false" )
	   << " = sorter->isValid();" << endl;
    } else if ( word1 == "4" ) { 
      cout << "sorter->dump( stdout )" << endl;
      sorter->dump( stdout );
    } else if ( word1 == "5" ) { 
      cout << ( sorter->isEmpty()  ? "true" : "false" ) 
	   << " = sorter->isEmpty();" << endl;
    } else if ( word1 == "6" ) { 
      cout << sorter->size() << " = sorter->size();" << endl;
    } else if ( word1 == "7" ) { 
      cout << sorter->queryMinBucket() << " = sorter->queryMinBucket();" 
	   << endl;
    } else if ( word1 == "8" ) { 
      int Bucket = 0;
      if ( word2 != "" ) { 
	Bucket = atoi( word2.c_str() );
      }
      if ( Bucket == 0 ) { 
	cout << "Bucket " << prompt << flush;
	cin >> Bucket;
      }
      cout << sorter->queryNextBucket( Bucket )
	   << " = sorter->queryNextBucket( " << Bucket << " );" << endl;
    } else if ( word1 == "9" ) { 
      int Bucket = 0;
      if ( word2 != "" ) { 
	Bucket = atoi( word2.c_str() );
      }
      if ( Bucket == 0 ) { 
	cout << "Bucket " << prompt << flush;
	cin >> Bucket;
      }
      cout << sorter->queryFirstItem(Bucket )
	   << " = sorter->queryFirstItem( " << Bucket << " );" << endl;
    } else if ( word1 == "10" ) { 
      int Item = 0;
      if ( word2 != "" ) { 
	Item = atoi( word2.c_str() );
      }
      if ( Item == 0 ) { 
	cout << "Item " << prompt << flush;
	cin >> Item;
      }
      cout << sorter->queryNextItem( Item )
	   << " = sorter->queryNextItem( " << Item << " );" << endl;
    } else if ( word1 == "11" ) { 
      int Item = 0;
      if ( word2 != "" ) { 
	Item = atoi( word2.c_str() );
      }
      if ( Item == 0 ) { 
	cout << "Item " << prompt << flush;
	cin >> Item;
      }
      cout << sorter->queryPrevItem( Item )
	   << " = sorter->queryPrevItem( " << Item << " );" << endl;
    } else if ( word1 == "12" ) { 
      int Item = 0;
      if ( word2 != "" ) { 
	Item = atoi( word2.c_str() );
      }
      if ( Item == 0 ) { 
	cout << "Item " << prompt << flush;
	cin >> Item;
      }
      cout << sorter->queryKey( Item )
	   << " = sorter->queryKey( " << Item << " );" << endl;
    } else if ( word1 == "13" ) { 
      int Item = 0;
      if ( word2 != "" ) { 
	Item = atoi( word2.c_str() );
      }
      if ( Item == 0 ) { 
	cout << "Item " << prompt << flush;
	cin >> Item;
      }
      cout << sorter->queryBucket( Item )
	   << " = sorter->queryBucket( " << Item << " );" << endl;
    } else if ( word1 == "14" ) { 
      cout << sorter->queryNInserted() 
	   << " = sorter->queryNInserted();" << endl;
    } else if ( word1 == "15" ) { 
      int Key = 0;
      int Item = 0;
      if ( word2 != "" ) { 
	Key = atoi( word2.c_str() );
      }
      if ( Key == 0 ) { 
	cout << "Key " << prompt << flush;
	cin >> Key;
      }
      if ( word3 != "" ) { 
	Item = atoi( word3.c_str() );
      }
      if ( Item == 0 ) { 
	cout << "Item " << prompt << flush;
	cin >> Item;
      }
      cout << ( sorter->insert( Key, Item ) ? "true" : "false" ) 
	   << " = sorter->insert( " << Key << ", " << Item << " );" << endl;
    } else if ( word1 == "16" ) { 
      sorter->setInsertFront();
      cout << "sorter->setInsertFront();" << endl;
    } else if ( word1 == "17" ) { 
      sorter->setInsertBack();
      cout << "sorter->setInsertBack();" << endl;
    } else if ( word1 == "18" ) { 
      int Item = 0;
      if ( word2 != "" ) { 
	Item = atoi( word2.c_str() );
      } 
      if ( Item == 0 ) { 
	cout << "Item " << prompt << flush;
	cin >> Item;
      }
      cout << ( sorter->remove( Item ) ? "true" : "false" )
	   << " = sorter->remove( " << Item << " );" << endl;
    } else if ( word1 == "19" ) { 
      cout << ( sorter->reset() ? "true" : "false" )
	   << " = sorter->reset();" << endl;
    } else if ( word1 == "20" ) { 
      cout << "sorter->printDebug( stdout );" << endl;
      sorter->printDebug( stdout );
    }
  } // end for(;;)     
}

void parsecommand( string& command, string& word1, 
		   string& word2, string& word3) { 
  
  // 1. trim leading and trailling whitespace from command.
 {
   string::size_type back = command.size();
   char ch = command[ back - 1 ];
   while( ch == ' ' || ch == '\t' || ch == '\n' ) {
    --back;
    ch = command[ back -1 ];
   }
   ++back;
   if ( back < command.size() ) {
     command.erase( back , command.size() - back );
   }
   string::size_type front = command.find_first_not_of( " \t\n" );
   if ( front != 0 ) {
     command.erase( 0, front );
   }
 }

  // 2. find first whitespace 
 {
   string::size_type nextend = command.find_first_of(" \t\n");
   // if there is no whitespace left, then we have one word.
   if ( nextend == string::npos ) { 
     word1 = command;
     return; 
   } else { 
     word1 = command.substr(0,nextend);
   }
   
   // now try for the second word.
   // advance 'next' to point to first non-whitespace after whitespace
   string::size_type next = command.find_first_not_of(" \t\n", nextend);
   // 'nextend' points to first whitespace after
   nextend = command.find_first_of(" \t\n", next);
   // if 'nextend' == npos...
   if ( nextend == string::npos ) { 
     // ... then we have exactly two words...
     word2 = command.substr( next, command.length() - next );
     return; 
   } else { 
     // ...else we probably have a third...
     word2 = command.substr( next, nextend - next );
   }
   

   // now try for the third word.
   // advance 'next' to point to first non-whitespace after whitespace
   next = command.find_first_not_of(" \t\n", nextend);
   // 'nextend' points to first whitespace after
   nextend = command.find_first_of(" \t\n", next);
   // if 'nextend' == npos...
   if ( nextend == string::npos ) { 
     // ... then we have exactly three words...
     word3 = command.substr( next, command.length() - next );
     return; 
   } else { 
     // ...else we probably have a fourth...
     word3 = command.substr( next, nextend - next );
   }
 }
}
