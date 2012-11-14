//
//  GraphMatrixFileFactory -- a Factory class for Graph and Matrix I/O
//
//  Gary Kumfert, Lawrence Livermore National Laboratory
//
///////////////////////////////////////////////////////////////////////
//


#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#include "spindle/GraphMatrixFileFactory.h"

#ifndef SPINDLE_GRAPH_H_
#include "spindle/Graph.h"
#endif

#ifndef SPINDLE_MATRIX_H_
#include "spindle/Matrix.h"
#endif

#include "spindle/ChacoFile.h"
#include "spindle/MatrixMarketFile.h"
#include "spindle/HarwellBoeingFile.h"
#include "spindle/DobrianFile.h"
#include "spindle/SpoolesFile.h"
#include "spindle/ChristensenFile.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

const char * 
GraphMatrixFileFactory::getHelpString() { 
  static const char* msg = 
" Input File types:\n"
"\t Chaco     : Chaco Software package by Bruce Hendrickson and Robert Leland\n"
"\t           	Sandia National Labs, New Mexico.\n"
"\t MeTiS     : MeTiS Partitioning Software by George Karypis and Vipin Kumar\n"
"\t           	University of Minnesota.\n"
"\t           	(note Use Chaco to handle MeTiS files. Chaco is more general.)\n"
"\t MatrixMarket : See http://gams.nist.gov/MatrixMarket\n"
"\t Coordinate : Same as MatrixMarket, but without the header\n"
"\t HB        : Harwell-Boeing Format.\n"
"\t Spooles-F : Spooles Graph.  Text format.\n"
"\t Spooles-B : Spooles Graph.  Binary format.\n"
"\t Dobrian   : Florin Dobrian's indefinite solver, Old Dominion University,\n"
"\t             Norfolk, Va\n"
"\t Christensen : Peter Christensen's matrix, has list of columns that must\n"
"\t             : be delayed.\n";
  return msg;
}


void
GraphMatrixFileFactory::reset() { 
  if ( m_graph != 0 && can_delete_graph ) { 
    delete m_graph;
  }
  if ( m_matrix != 0 && can_delete_matrix ) { 
    delete m_matrix;
  }
  m_matrix = 0;
  m_graph = 0;
  can_delete_matrix = false;
  can_delete_graph = false;
}


bool 
GraphMatrixFileFactory::loadFile( string& filename, string& filetype ) { 
  reset();

  if ( !strcasecmp( filetype.c_str(), "Chaco" ) ) { 
    ChacoFile input;
    if ( !input.open( filename.c_str(), "r" ) ) { 
      return false;
    }
    input.read();
    m_graph = new Graph( &input );
    can_delete_graph = true;
  } else if ( !strcasecmp( filetype.c_str(), "MeTiS" ) ) { 
    ChacoFile input;
    if ( !input.open ( filename.c_str(), "r" ) ) { 
      return false;
    }
    input.read();
    m_graph = new Graph ( &input );
    can_delete_graph = true;
  } else if ( !strcasecmp(filetype.c_str(),"MatrixMarket") ) {
    MatrixMarketFile input;
    if (!input.open( filename.c_str(),"r") ) {
      return false;
    }
    input.read();
    m_graph = new Graph( &input );
    can_delete_graph = true;
  } else if ( !strcasecmp(filetype.c_str(),"Coordinate") ) {
    MatrixMarketFile input;
    if (!input.open( filename.c_str(),"r") ) {
      return false;
    }
    input.read();
    m_graph = new Graph( &input );
    can_delete_graph=true;
  }else if ( !strcasecmp( filetype.c_str(), "HB" ) ) { 
    HarwellBoeingFile input;
    if ( !input.open( filename.c_str(), "r") ) { 
      return false;
    }
    input.read();
    m_matrix = new Matrix( &input );
    can_delete_matrix = true;
  } else if ( !strcasecmp( filetype.c_str(), "Dobrian" ) ) { 
    DobrianFile input;
    if ( !input.open( filename.c_str(), "r") ) { 
      return false;
    }
    input.read();
    m_matrix = new Matrix( &input );
    can_delete_matrix = true;
  } else if ( !strcasecmp( filetype.c_str(), "Spooles-F" ) ) { 
    SpoolesFile input;
    if ( !input.open( filename.c_str(), "r" ) ) { 
      return false;
    }
    input.read();
    m_matrix = new Matrix( &input );
    can_delete_matrix = true;
  } else if ( !strcasecmp( filetype.c_str(), "Spooles-B" ) ) { 
    SpoolesFile input;
    if ( !input.open( filename.c_str(), "rb" ) ) { 
      return false;
    }
    input.read();
    m_matrix = new Matrix( &input );
    can_delete_matrix = true;
  } else if ( !strcasecmp( filetype.c_str(), "Christensen" ) ) { 
    ChristensenFile input;
    if ( !input.open( filename.c_str(), "r" ) ) { 
      return false;
    }
    input.read();
    m_graph = new Graph( &input );
    can_delete_graph = true;
  } else { 
    return false;
  }
  
  // now validate the results
  if ( m_matrix != 0 ) { 
    m_matrix->validate();
    if ( !m_matrix->isValid() ) { 
      return false;
    }
  } else if ( m_graph != 0 ) { 
    m_graph->validate();
    if ( !m_graph->isValid() ) { 
      return false;
    }
  }
  return true;
}


Matrix * 
GraphMatrixFileFactory::spawnMatrix() {
  if ( m_graph != 0 && m_matrix == 0) { 
    if ( m_graph->isValid() ) { 
      const GraphBase* inputGraph = m_graph;
      m_matrix = new Matrix( inputGraph );
      m_matrix->validate();
      can_delete_matrix = true;
      // should make m_graph != 0
    } else { 
      return 0;
    }
  }
  if ( m_matrix != 0 ) {
    if ( m_matrix->isValid() ) { 
      can_delete_matrix = false;
      return m_matrix;
    } else { 
      return 0;
    }
  } else {
    return 0;
  }
}




Graph * 
GraphMatrixFileFactory::spawnGraph() {
  if ( m_matrix != 0 && m_graph == 0 ) { 
    if ( m_matrix->isValid() ) { 
      const MatrixBase* inputMatrix = m_matrix;
      m_graph = new Graph( inputMatrix ) ;
      m_graph->validate();
      can_delete_graph = true;
      // should make m_graph != 0
    } else { 
      return 0;
    }
  }
  if ( m_graph != 0 ) { 
    if ( m_graph->isValid() ) { 
      can_delete_graph = false;
      return m_graph;
    } else { 
      return 0;
    }
  } else {
    return 0;
  }
}

bool 
GraphMatrixFileFactory::loadGraph( const Graph* graph ) {
  if ( graph != 0 && graph->isValid() ) { 
    reset();
    m_graph = new Graph( graph );
    can_delete_graph = true;
    return true;
  } else { 
    return false;
  }
}
bool 
GraphMatrixFileFactory::loadMatrix( const Matrix* matrix ) {
  if ( matrix != 0 && matrix->isValid() ) { 
    reset();
    m_matrix = new Matrix( matrix );
    can_delete_matrix = true;
    return true;
  } else { 
    return false;
  }
}

/*
bool 
GraphMatrixFileFactory::spawnFile( string& filename, string& filetype ) { 
  if ( filename.length() < 0  || filetype.length() < 0 ) { 
    return false;
  }
  if ( m_graph == 0 && m_matrix == 0 ) { 
    return false;
  } 
  if ( m_graph != 0 && !m_graph->isValid() ) {
    return false;
  } 
  if ( m_matrix != 0  && !m_matrix->isValid() ) { 
    return false;
  }
  if ( !strcasecmp( filetype.c_str(), "Chaco" ) ) { 
    ChacoFile output;
  } else if ( !strcasecmp( filetype.c_str(), "MeTiS" ) ) { 
    ChacoFile output;
  } else if ( !strcasecmp( filetype.c_str(), "HB" ) ) { 
    HarwellBoeingFile output; 
  } else if ( !strcasecmp( filetype.c_str(), "Dobrian" ) ) { 
    DobrianFile output;
  } else if ( !strcasecmp( filetype.c_str(), "Spooles-F" ) ) { 
    SpoolesFile output;
  } else if ( !strcasecmp( filetype.c_str(), "Spooles-B" ) ) { 
    SpoolesFile output; 
  } else if ( !strcasecmp( filetype.c_str(), "Christensen" ) ) { 
    ChristensenFile output;
  } else { 
    return false;
  }
  return true;
}
*/


