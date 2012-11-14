//
// symbfact.cc
//
// not finished!!!

#include <fstream.h>

static char help_string1[] = "Usage:\n\t ";
static char help_string2[] = 
      " [ -h | -help ]  [ --help-fintype ] \n"
      "            --fin <GraphInputFile> [ --fintype <GraphFileType>] \n"
      "            [ --pif <PermutationMapInputFile> ] \n"
      "            [ -s | -stats ] [ --ptype old2new | new2old ] \n"    
      "\t -h | --help                  : display this screen\n"
      "\t --help-fintype               : information about file formats\n"
      "\t -s | --stats                 : show quality statistics\n"
      "\t --fin <GraphInputFile>       : Metis format graph \n"
      "\t --fintype <GraphFileType>    : specifies the format\n"
      "\t                              : ( use --help-fintype for details\n"
      "\t --pif <PermutationMapInputFile> : one index per line \n"
      "\t --ptype old2new              : load perm as an old2new perm \n"
      "\t --ptype new2old              : load perm as an old2new perm \n";

#include <iostream.h>
#include "spindle/Graph.h"
#include "spindle/GraphMatrixFileFactory.h"
#include "spindle/SymbolicFactorization.h"
#include "spindle/OptionDatabase.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main(int argc, char *argv[]) {
  ios::sync_with_stdio();

  ///////////////////////////////////////////////////////////////////////////
  //
  //  1.  Parse command line options
  //
  OptionDatabase cmdLine;
  cmdLine.loadCommandLineOptions( argc, argv );
  { 
    bool terminate = false;
    if ( cmdLine.hasOption("h|help") ) {
      cerr << help_string1 << argv[0] << help_string2;
      terminate = true;
    }
    if ( cmdLine.hasOption("help-fintype" ) ) { 
      cerr << GraphMatrixFileFactory::getHelpString() << endl;
      terminate = true;
    }
    if ( terminate ) { 
      exit( 0 );
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  //
  // 2.  Create graph based on file type
  //
  GraphMatrixFileFactory factory;
  string filename;
  string filetype;
  if ( cmdLine.hasOption("fin") ) { 
    filename = cmdLine.getOption("fin");
  } else { 
    cerr << "Error: must specify input file using \"--fin\". "<< endl;
    cerr << "       Try \"--help\" for more information." << endl;
    exit(-1);
  }
  if ( cmdLine.hasOption("fintype") ) { 
    filetype = cmdLine.getOption("fintype");
  } else { 
    cerr << "Warning: no file type specfied, Chaco assumed, else use \"--fintype\". "<< endl;
    filetype = "Chaco";
  }
  Graph* graph=0;
  if ( factory.loadFile( filename, filetype ) ) { 
    graph = factory.spawnGraph();
  }
 
  //////////////////////////////////////////////////////////////////////
  //
  // 3. Validate the graph
  //
  if ( graph == 0 ) { 
    cerr << "ERROR: cannot validate a null graph pointer." << endl;
    exit(-1);
  }
  graph->validate();
  if ( ! graph->isValid() ) {
    cerr << "ERROR: cannot create valid graph from this input." << endl;
    exit(-1);
  }
  const int nVtxs = graph->queryNVtxs();
  
  //////////////////////////////////////////////////////////////////////
  //
  // 4. Load PermutationMap object
  //
  PermutationMap perm(nVtxs);
  if ( cmdLine.hasOption("pif") ) {
    const int n = graph->size();
    int * perm_array = new int[ n ];
    const char* input_file_name = cmdLine.getOption("pif");
    FILE* fp;
    if ( ( fp = fopen( input_file_name, "r" ) ) == NULL ) {
      cerr << "Error, could not open file \'" << input_file_name << "\'" << endl;
      exit(-1);
    }
    int *cur = perm_array;
    for( int i=0; i<n; ++i ) {
      if ( fscanf(fp,"%d",cur++) != 1 ) {
	cerr << "Error, could not read all of file \'" << input_file_name << "\'" << endl;
	fclose(fp);
	exit(-1);
      }
    }
    fclose(fp);
    if ( cmdLine.hasOption("ptype","old2new") ) {
      perm.getOld2New().take( perm_array, n ); // assumes ownership, don't delete perm_array
    } else {
      perm.getNew2Old().take( perm_array, n ); // assumes ownership, don't delete perm_array
    }
    perm.validate();
    if ( ! perm.isValid() ) {
      cerr << "Error validating permtuation" << endl;
      exit(-1);
    } 
  } else {
    cerr << "WARNING: no permutation vector specified.  Using implicit ordering of graph." << endl;
  }

  //////////////////////////////////////////////////////////////////////
  //
  // 5. Do the symbolic factorization
  //

  stopwatch symbFactTotalTimer;
  stopwatch symbFactSetGraphTimer;
  stopwatch symbFactSetETreeTimer;
  stopwatch symbFactExecutionTimer;

  SymbolicFactorization symbfact;  // create the SymbolicFactorization object
  {
    symbFactTotalTimer.start();
    // set the graph
    symbFactSetGraphTimer.start();
    if ( ! symbfact.setGraph( graph ) ) {
      cerr << "Error setting Graph" << endl;
      cerr << symbfact.queryErrMsg() << endl;
      exit(-1);
    }
    symbFactSetGraphTimer.stop();
    
    // create the etree based on the parent array of the quotient graph
    if ( perm.isValid() ) {
      symbFactSetETreeTimer.start();
      if ( ! symbfact.setPermutationMap( & perm ) ) { 
	cerr << "Error setting PermutationMap" << endl;
	cerr << symbfact.queryErrMsg() << endl;
	exit(-1);
      }
      symbFactSetETreeTimer.stop();
    }    

    symbFactExecutionTimer.start();
    if ( ! symbfact.execute() ) {
      cerr << "Error detected during symbolic factorization." << endl;
      cerr << symbfact.queryErrMsg() << endl;
      exit(-1);
    }
    symbFactExecutionTimer.stop();
    symbFactTotalTimer.stop();
  }

  //////////////////////////////////////////////////////////////////////
  //
  // 6. Display timing information, if requested.
  //
  if ( cmdLine.hasOption("t|time") ) {
    float ticksPerSec = stopwatch::queryTicksPerSec();
    cout << "    Total Symbolic Factorization Execution Time                : " 
	 << symbFactTotalTimer.queryTotalUserTicks()/ticksPerSec << " / " 
	 << symbFactTotalTimer.queryTotalSystemTicks()/ticksPerSec << endl;
    cout << "       Total Time Setting Graph             : "
	 << symbFactSetGraphTimer.queryTotalUserTicks()/ticksPerSec << " / "
	 << symbFactSetGraphTimer.queryTotalSystemTicks()/ticksPerSec << endl;
    cout << "       Total Time Setting ETree             : "
	 << symbFactSetETreeTimer.queryTotalUserTicks()/ticksPerSec << " / "
	 << symbFactSetETreeTimer.queryTotalSystemTicks()/ticksPerSec << endl;
    cout << "       Total Time Doing Factorization       : "
	 << symbFactExecutionTimer.queryTotalUserTicks()/ticksPerSec << " / "
	 << symbFactExecutionTimer.queryTotalSystemTicks()/ticksPerSec << endl;
  }
  
  //////////////////////////////////////////////////////////////////////
  //
  // 7. Display quality statistics if requested.
  //
  if ( cmdLine.hasOption("s|stats") ) {
    cout << "Run Statistics : " << endl;
    cout << "    # of vertices in G       : " << graph->queryNVtxs() << endl;
    cout << "    # of edges in G          : " << graph->queryNEdges() << endl;
    cout << "    Fill                     : " << symbfact.queryFill() << endl;
    cout << "    Size of factor           : " << symbfact.querySize() << endl;
    cout << "    Work                     : " << symbfact.queryWork() << endl;
  } 
}
