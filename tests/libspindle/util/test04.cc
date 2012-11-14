//
// src/util/drivers/tests/test04.cc
//
//
// tests Graph Compressor

#include <iostream.h>
#include "spindle/Graph.h"
#include "spindle/GraphCompressor.h"

#include "samples/fe_mesh.h"

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

int main() {
  ios::sync_with_stdio();

  Graph graph(nvtxs,Aptr,Aind);
  graph.validate();
  if ( ! graph.isValid() ) {
    cerr << "Cannot create valid graph." << endl;
  }

  graph.dump( stdout );
  graph.prettyPrint( stdout );
  GraphCompressor compressor;
  compressor.setGraph( & graph );
  compressor.execute();
  cout << "Compressed from " << nvtxs << " vertices to " 
       << compressor.queryCompressedNVtxs() << " vertices." << endl;

  Graph* cgraph = compressor.createCompressedGraph();
  if ( cgraph==0 ) {
    cerr << "Could not create compressed Graph" << endl;
  }

  cgraph->dump( stdout );
  cgraph->prettyPrint( stdout );
  delete cgraph;
}
