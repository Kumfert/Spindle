//
// spindle/src/io/drivers/examples/ex02.cc -- convert Chaco or HarwellBoeing to Dobrian Files
//
//

static char help_string1[] = "Usage:\n\t ";
static char help_string2[] = 
      " [ -h | -help ] [ -help-fintype ] [ -fin <inputfile> ] \n"
      "\t\t[ -fintype <formattype> ] [ -fout <outputfile> ] \n"
      "\t -h | -help               : display this screen\n"
      "\t -fin <inputfile>         : specify Chaco filename on commandline\n"
      "\t                            (else done interactively)\n" 
      "\t -fintype <formattype>    : try \'-help-fintype\' for more info\n"
      "\t -fout <outputfile>       : specify DobrianFile output on commandline\n"
      "\t -fout stdout             : display output to standard out\n"
      "\t                            (else done interactively)\n";

static char help_fintype[] = 
" Input File types:\n"
"\t Chaco     : Chaco Software package by Bruce Hendrickson and Robert Leland\n"
"\t           	Sandia National Labs, New Mexico.\n"
"\t MeTiS     : MeTiS Partitioning Software by George Karypis and Vipin Kumar\n"
"\t           	University of Minnesota.\n"
"\t           	Use Chaco to handle MeTiS files. Chaco is more general.\n"
"\t HB        : Harwell-Boeing Format.\n"
"\t Spooles-F : Spooles Graph.  Text format.\n"
"\t Spooles-B : Spooles Graph.  Binary format.\n"
"\t Dobrian   : Florin Dobrian's indefinite solver, Old Dominion University,\n"
"\t             Norfolk, Va\n"
"\t Christensen : Peter Christensen's matrix, has list of columns that must\n"
"\t             : be delayed\n";

#include <fstream.h>
#include <iostream.h>

#include "spindle/spindle.h"
#include "spindle/ChacoFile.h"
#include "spindle/HarwellBoeingFile.h"
#include "spindle/DobrianFile.h"
#include "spindle/SpoolesFile.h"
#include "spindle/ChristensenFile.h"
#include "spindle/Graph.h"
#include "spindle/Matrix.h"
#include "spindle/CommandLineOptions.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main( int argc, char *argv[] ) {

  ///////////////////////////////////////////////////////////////////////////
  //
  //  1.  Parse command line options
  //
  CommandLineOptions cmdLine( argc, argv );
  bool terminate = false; 
  if ( cmdLine.hasFlag("-h|-help") ) {
    cerr << help_string1 << argv[0] << help_string2 << endl;
    terminate = true;
  }
  if ( cmdLine.hasFlag("-help-fintype") ) { 
    cerr << help_fintype << endl;
    terminate = true;
  }
  if ( terminate ) { 
    exit ( 0 );
  }
   
  ////////////////////////////////////////////////////////////////////////////
  //
  // 2.  Open input stream
  //
  const char* input_file_name;

  if ( cmdLine.hasFlag("-fin") ) {
    input_file_name = cmdLine.getOption("-fin");
  } else {  
    cerr << "Error: must specify input file using \"-fin\". "<< endl;
    cerr << "       Try \"-help\" for more information." << endl;
    exit(-1);
  }
  
  // one of these will be set.
  Matrix * mat = 0;
  Graph * graph = 0;

  if ( cmdLine.hasFlag("-fintype") ) { 
    const char * filetype = cmdLine.getOption("-fintype");
    if ( !strcasecmp(filetype,"Chaco") ) { 
      ChacoFile input;
      
      if (!input.open( input_file_name,"r") ) {
	cerr << "Error:  could not read \'" << input_file_name << "\'" << endl;
	exit(-1);
      }
      input.read();
      graph = new Graph( &input );
    } else if ( !strcasecmp(filetype,"MeTiS") ) {
      ChacoFile input;
      if (!input.open( input_file_name,"r") ) {
	cerr << "Error:  could not read \'" << input_file_name << "\'" << endl;
	exit(-1);
      }
      input.read();
      graph = new Graph( &input );
    } else if ( !strcasecmp(filetype,"HB") ) {
      HarwellBoeingFile input;
      if (!input.open( input_file_name,"r") ) {
	cerr << "Error:  could not read \'" << input_file_name << "\'" << endl;
	exit(-1);
      }
      input.read();
      mat = new Matrix( &input );
    } else if ( !strcasecmp(filetype,"Dobrian") ) {
      DobrianFile input;
      if (!input.open( input_file_name,"r") ) {
	cerr << "Error:  could not read \'" << input_file_name << "\'" << endl;
	exit(-1);
      }
      input.read();
      mat = new Matrix( &input );
    } else if ( !strcasecmp(filetype,"Spooles-F") ) {
      SpoolesFile input;
      if (!input.open( input_file_name,"r") ) {
	cerr << "Error:  could not read \'" << input_file_name << "\'" << endl;
	exit(-1);
      }
      input.read();
      mat = new Matrix( &input );
    } else if ( !strcasecmp(filetype,"Spooles-B") ) {
      SpoolesFile input;
      if (!input.open( input_file_name,"rb") ) {
	cerr << "Error:  could not read \'" << input_file_name << "\'" << endl;
	exit(-1);
      }
      input.read();
      mat = new Matrix( &input );
    } else if ( !strcasecmp(filetype,"Christensen") ) {
      ChristensenFile input;
      if (!input.open( input_file_name,"r") ) {
	cerr << "Error:  could not read \'" << input_file_name << "\'" << endl;
	exit(-1);
      }
      input.read();
      graph = new Graph( &input );
    } else { 
      cerr << "Error: do not understand \'" << filetype << "\' files." << endl;
      exit(-1);
    }
  } else { 
    // assume chaco file
    ChacoFile input;
    if (!input.open( input_file_name,"rb") ) {
      cerr << "Error:  could not read \'" << input_file_name << "\'" << endl;
      exit(-1);
    }    
    input.read();
    graph = new Graph( &input );
  }

  //////////////////////////////////////////////////////////////////////
  //
  // 3. Validate
  //
  if ( mat != 0 ) { 
    mat->validate();
    if ( ! mat->isValid() ) { 
      cerr << "ERROR: read matrix input file, but cannot validate matrix object" << endl;
      exit(-1);
    }
  } else if ( graph != 0 ) { 
    graph->validate();
    if ( ! graph->isValid() ) { 
      cerr << "ERROR: read graph input file, but cannot validate input graph" << endl;
      exit(-1);
    }
  }
  
  ////////////////////////////////////////////////////////////////////////////
  //
  // 4.  Set the output file
  //
  DobrianFile outputFile;
  if ( mat != 0 ) { 
    outputFile.resize( mat->queryNCols(), mat->queryNRows(), mat->queryNNonZeros() );
    outputFile.getColPtr().take( mat->getColPtr() );
    outputFile.getRowIdx().take( mat->getRowIdx() );
    outputFile.getDiagPtr().take( mat->getDiagPtr() );
    outputFile.getRealValues().take( mat->getRealValues() );
    outputFile.getImgValues().take( mat->getImgValues() );
 } else if ( graph != 0 ) { 
    //outputFile = new DobrianFile ( graph );
    outputFile.resize( graph->queryNVtxs(), graph->queryNVtxs(), graph->queryNNonZeros() );
    outputFile.getColPtr().take( graph->getAdjHead() );
    outputFile.getRowIdx().take( graph->getAdjList() );
  }

  ////////////////////////////////////////////////////////////////////////////
  //
  // 5.  Set the output file
  //
  if ( cmdLine.hasFlag("-fout") ) {
    const char* output_file_name = cmdLine.getOption("-fout");
    if ( ! outputFile.open( output_file_name , "w") ) {
      cerr << "Error:  could not open \'" << output_file_name << "\'" << endl;
      exit(-1);
    }
  } else {                       // else prompt user for output file
    cerr << "Error: must specify an outputfile with \'-fout\' option." << endl;
    exit(-1);
  }
  
  if ( !outputFile.write() ) {
    cerr << "Error: could not complete writing outputfile." << endl;
  }
  outputFile.close();
}
