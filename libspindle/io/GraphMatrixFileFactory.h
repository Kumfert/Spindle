//
//  GraphMatrixFileFactory -- a Factory class for Graph and Matrix I/O
//
//  Gary Kumfert, Lawrence Livermore National Laboratory
//
///////////////////////////////////////////////////////////////////////
//


#ifndef SPINDLE_GRAPH_MATRIX_FILE_FACTORY_H_
#define SPINDLE_GRAPH_MATRIX_FILE_FACTORY_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

SPINDLE_BEGIN_NAMESPACE

class Graph;
class Matrix;

/**
 * @memo Converts files to and from Graphs and Matrices
 * @type class
 * @author Gary Kumfert
 * @version #$Id$#
 */
class GraphMatrixFileFactory {
private:
  Graph * m_graph;
  Matrix * m_matrix;
  bool can_delete_graph;
  bool can_delete_matrix;

protected:
  /** */
  void reset();

public:
  /**@name constructors/destructors */
  //@{
  /// default constructor
  GraphMatrixFileFactory() :  m_graph(0), m_matrix(0) { reset(); }
  /// destructor
  ~GraphMatrixFileFactory() { reset(); }
  //@}

  /// load a file to the appropriate internal graph or matrix
  bool loadFile( string& filename, string& filetype );
  /// load a new graph to the factory's internal state
  bool loadGraph( const Graph* graph );
  /// load a new matrix to the factory's internal state
  bool loadMatrix( const Matrix* matrix );
  
  /// given a valid internal graph or matix, export to a file
  //  bool spawnFile( string& filename, string& filetype );
  /// given a valid internal graph or matrix, spawn a graph
  Graph * spawnGraph();
  /// given a valid internal graph or matrix, spawn a matrix
  Matrix * spawnMatrix();
  
  /// display help info for recognized types
  static const char* getHelpString();
};

SPINDLE_END_NAMESPACE

#endif
