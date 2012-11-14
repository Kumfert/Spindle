//
//
//

#include <iostream.h>
#include <fstream.h>
#include "spindle/OptionDatabase.h"


char * argv[] = {
  "test05.exe",
  "-fin", "/home/sparse/matrices/testmat.chac", 
  "-fintype", "Chaco", 
  "-time", "5",
  "-stats", 
  "-help", "priority", "options", "other stuff",
  "-version"
};

int argc = 13;


#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main() { 

  OptionDatabase options;
  
  cout << "Adding option \"author\"  = \"Gary Kumfert\"      ? " 
       << (options.setOption( "author", "Gary Kumfert" ) ? "passed." : "failed." ) 
       << endl;
  cout << "Adding option \"coauthor\" = \"\"                 ? " 
       << ( options.setOption( "coauthor") ? "passed." : "failed." )
       << endl;
  cout << "Adding option \"parents\"  = \"Dad\"              ? "
       << (options.setOption( "parents", "Dad" ) ? "passed." : "failed." )
       << endl;
  cout << "Adding option \"parents\"  = \"Mom\"              ? "
       << (options.setOption( "parents", "Mom" ) ? "passed." : "failed." )
       << endl << endl;

  cout << "Dumping... {" << endl << options << '}' << endl << endl;

  cout << "Has option \"author\" == T  ?         "  
       << (options.hasOption("author") ? "passed." : "failed." ) 
       << endl;
  cout << "Has option \"coauthor\" == T ?        "  
       << (options.hasOption("coauthor") ? "passed." : "failed." ) 
       << endl;
  cout << "Has option \"parents\" == T ?         "  
       << (options.hasOption("parents") ? "passed." : "failed." ) 
       << endl;
  cout << "Has option \"grandparents\" == F ?    "  
       << (options.hasOption("grandparents") ? "failed." : "passed." ) 
       << endl << endl;

  cout << "Remove option \"author\" == T ?       "  
       << (options.unsetOption("author") ? "passed." : "failed." )  
       << endl;
  cout << "Remove option \"coauthor\" == T ?     "  
       << (options.unsetOption("coauthor") ? "passed." : "failed.")   
       << endl;
  cout << "Remove option \"parents\" == T ?      "  
       << (options.unsetOption("parents") ? "passed." : "failed." ) 
       << endl;
  cout << "Remove option \"grandparents\" == F ? "  
       << (options.unsetOption("grandparents") ? "failed." : "passed." ) 
       << endl << endl;
 
  cout << "Dumping... {" << endl << options << '}' << endl << endl;

  cout << "Loading Commandline Options...    " 
       << (options.loadCommandLineOptions( argc,  argv, '-') ? "passed." : "failed." )
       << endl;


  cout << "Dumping... {" << endl << options << '}' << endl << endl;
  { 
    cout << "Writing to file \'junk.txt\'..." << endl;
    ofstream outputfile("junk.txt");
    outputfile << options << endl;
    outputfile.close();
    
    cout << "Creating db2 from file \'junk.txt\'..." << endl;
    ifstream inputfile("junk.txt");
    OptionDatabase db2;
    inputfile >> db2;
    inputfile.close();
  
    cout << "Dumping db2... {" << endl << options << '}' << endl << endl;
  }

  cout << "Adding version information ...    "
       << (options.addToOption("version","0187341098741093487" )  ? "passed." : "failed." )
       << endl;
  cout << "Adding time information ...       "
       << (options.addToOption("time","hours" )  ? "passed." : "failed." )
       << endl;
  cout << "Removing help option ...          "
       << (options.unsetOption("help")  ? "passed." : "failed." )
       << endl;

  cout << "Dumping... {" << endl << options << '}' << endl << endl;

  {
    cout << "Opening db with \'test05.in\' file..." << endl;
    OptionDatabase db;
    ifstream inputfile("test05.in");
    inputfile >> db; 
    inputfile.close();
    
    cout << "Dumping db... {" << endl << db << '}' << endl << endl;

    { 
      bool temp;
      cout << "Test (list2[0] == true)...            " ;
      if (  db.queryOption( "list2", &temp, 0 ) ) { 
	cout << ( (temp) ? "passed." : "failed." ) << endl;
      } else { 
	cout << "failed, cannot determine answer" << endl;
      }
      
      cout << "Test (list2[0] != false)...           " ;
      if (  db.queryOption( "list2", &temp, 0 ) ) { 
	cout << ( (!temp) ? "failed." : "passed." ) << endl;
      } else { 
	cout << "failed, cannot determine answer" << endl;
      }
      
      cout << "Test list2[1] cannot evaluate to true of false since it is 12... " ;
      if (  db.queryOption( "list2", &temp, 1 ) ) { 
	cout << ( (!temp) ? "failed." : "failed." ) << endl;
      } else { 
	cout << "passed." << endl;
      }
    }  
    
    { 
      int temp;
      cout << "Test (list2[0] == 1)...               " ;
      if (  db.queryOption( "list2", &temp, 0 ) ) { 
	cout << ( (temp==1) ? "passed." : "failed." ) << endl;
      } else { 
	cout << "failed, cannot determine answer" << endl;
      }
      
      cout << "Test (list2[1] == 12)...              " ;
      if (  db.queryOption( "list2", &temp, 1 ) ) { 
	cout << ( (temp==12) ? "passed." : "failed." ) << endl;
      } else { 
	cout << "failed, cannot determine answer" << endl;
      }
    }  
  }
  
  { 
    int temp;
    cout << "Error on read a very large number ... (pass is good) ";
      if ( options.queryOption("verion", &temp ) ) { 
	cout << "failed." << endl;
      } else {
	cout << "passed." << endl;
      }
  }

  return 0;
}


