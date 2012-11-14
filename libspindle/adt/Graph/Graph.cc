// 
//  Graph.cc
//
//  $Id: Graph.cc,v 1.2 2000/02/18 01:31:45 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1998, Old Dominion University.  All rights reserved.
// 
//  Permission to use, copy, modify, distribute and sell this software and
//  its documentation for any purpose is hereby granted without fee, 
//  provided that the above copyright notice appear in all copies and
//  that both that copyright notice and this permission notice appear
//  in supporting documentation.  Old Dominion University makes no
//  representations about the suitability of this software for any 
//  purpose.  It is provided "as is" without express or implied warranty.
//
///////////////////////////////////////////////////////////////////////
//
// 
//

#include "spindle/Graph.h"

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include "algo.h"
#else
#include <algorithm>
using namespace std;
#endif

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#ifndef SPINDLE_ARCHIVE_H_
#include "spindle/SpindleArchive.h"
#endif

#ifndef SPINDLE_MATRIX_BASE_H_
#include "spindle/MatrixBase.h"
#endif

#ifndef SPINDLE_MATRIX_UTILS_H_
#include "spindle/MatrixUtils.h"
#endif

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

#ifndef __FUNC__
#undef __FUNC__
#endif

#define __FUNC__ "Graph::Graph()"
Graph::Graph(): GraphBase(), SpindlePersistant() {
  FENTER;
  incrementInstanceCount( Graph::MetaData );
  FCALL reset();
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "Graph::Graph( const int NVtxs, const int *AdjHead, const int *AdjList )"
Graph::Graph( const int NVtxs, const int *AdjHead, const int *AdjList ) {
  FENTER;
  incrementInstanceCount( Graph::MetaData );
  FCALL reset();
  if ( (NVtxs <= 0 ) || ( AdjHead == 0 ) || ( AdjList == 0 ) ) {
    // invalid parameters;
    return;
  }
  nVtxs = NVtxs;
  nNonZeros = AdjHead[ nVtxs ];
  nEdges = nNonZeros;
  adjHead.import( AdjHead, nVtxs+1 );
  adjList.import( AdjList, nNonZeros );
  currentState = UNKNOWN;
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "Graph::Graph( const int NVtxs, int *AdjHead, int *AdjList )"
Graph::Graph( const int NVtxs, int *AdjHead, int *AdjList ) {
  FENTER;
  incrementInstanceCount( Graph::MetaData );
  reset();
  if ( (NVtxs <= 0 ) || ( AdjHead == 0 ) || ( AdjList == 0 ) ) {
    // invalid parameters;
    ERROR(SPINDLE_ERROR_ARG_BADPTR,"Invalid constructor arguments.");
    currentState = INVALID;
    FEXIT;
  }
  nVtxs = NVtxs;
  nNonZeros = AdjHead[ nVtxs ];
  nEdges = nNonZeros;
  adjHead.take( AdjHead, nVtxs+1  );
  adjList.take( AdjList, nNonZeros );
  currentState = UNKNOWN;
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "Graph::Graph( const int NVtxs, int *AdjHead, int *AdjList, int *VtxWeights, int *EdgeWeights)"
Graph::Graph( const int NVtxs, int *AdjHead, int *AdjList,  int *VtxWeights, int *EdgeWeights ) {
  FENTER;
  incrementInstanceCount( Graph::MetaData );
  FCALL reset();
  if ( (NVtxs <= 0 ) || ( AdjHead == 0 ) || ( AdjList == 0 ) ) {
    ERROR(SPINDLE_ERROR_ARG_BADPTR,"Invalid constructor arguments.");
    currentState = INVALID;
    FEXIT;
  }
  nVtxs = NVtxs;
  nNonZeros = AdjHead[ nVtxs ];
  nEdges = nNonZeros;
  adjHead.take( AdjHead, nVtxs+1 );
  adjList.take( AdjList, nNonZeros );
  currentState = UNKNOWN;
  if ( VtxWeights != 0 ) {
    vtxWeight.take( VtxWeights, nVtxs );
  }
  if (EdgeWeights != 0 ) {
    edgeWeight.take( EdgeWeights, nNonZeros );
  }
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "Graph::Graph( const int NVtxs, const int *AdjHead, const int *AdjList," \
"const int *VtxWeights, const int* EdgeWeights )"
Graph::Graph( const int NVtxs, const int *AdjHead, const int *AdjList, 
	      const int *VtxWeights, const int* EdgeWeights ) {
  FENTER;
  incrementInstanceCount( Graph::MetaData );
  FCALL reset();
  if ( (NVtxs <= 0 ) || ( AdjHead == 0 ) || ( AdjList == 0 ) ) {
    ERROR(SPINDLE_ERROR_ARG_BADPTR,"Invalid constructor arguments.");
    currentState = INVALID;
    FEXIT;
  }
  nVtxs = NVtxs;
  nNonZeros = AdjHead[ nVtxs ];
  nEdges = nNonZeros;
  adjHead.import( AdjHead, nVtxs+1 );
  adjList.import( AdjList, nNonZeros );
  currentState = UNKNOWN;
  if ( VtxWeights != 0 ) {
    vtxWeight.import( VtxWeights, nVtxs );
  }
  if (EdgeWeights != 0 ) {
    edgeWeight.import( EdgeWeights, nNonZeros );
  }
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "Graph::Graph( GraphBase* inputGraph )"
Graph::Graph( GraphBase* inputGraph ){
  FENTER;
  incrementInstanceCount( Graph::MetaData );
  FCALL reset();
  if ( inputGraph == 0 ) {
    ERROR(SPINDLE_ERROR_ARG_BADPTR,"Invalid constructor arguments.");
    currentState = INVALID;
    FEXIT;
  }
  nVtxs = inputGraph->queryNVtxs();
  nEdges = inputGraph->queryNEdges();
  nBoundVtxs = inputGraph->queryNBoundVtxs();
  nNonZeros = inputGraph->queryNNonZeros();
  // Would like to be able to access protected
  // members of a base class.
  //  adjHead.take( inputGraph->adjHead );
  //  adjSelf.take( inputGraph->adjSelf );
  //  adjList.take( inputGraph->adjList );
  //  vtxWeight.take( inputGraph->vtxWeight );
  //  edgeWeight.take( inputGraph->edgeWeight );
  adjHead.import( inputGraph->getAdjHead() );
  adjSelf.import( inputGraph->getAdjSelf() );
  adjList.import( inputGraph->getAdjList() );
  vtxWeight.import( inputGraph->getVtxWeight() );
  edgeWeight.import( inputGraph->getEdgeWeight() );

  currentState = UNKNOWN;
  FEXIT;
}
#undef __FUNC__ 

#define __FUNC__ "Graph::Graph( const GraphBase* inputGraph )"
Graph::Graph( const GraphBase* inputGraph ){
  FENTER;
  incrementInstanceCount( Graph::MetaData );
  FCALL reset();
  if ( inputGraph == 0 ) {
    ERROR(SPINDLE_ERROR_ARG_BADPTR,"Invalid constructor arguments.");
    currentState = INVALID;
    FEXIT;
  }
  nVtxs = inputGraph->queryNVtxs();
  nEdges = inputGraph->queryNEdges();
  nBoundVtxs = inputGraph->queryNBoundVtxs();
  nNonZeros = inputGraph->queryNNonZeros();
  adjHead.import( inputGraph->getAdjHead() );
  adjSelf.import( inputGraph->getAdjSelf() );
  adjList.import( inputGraph->getAdjList() );
  vtxWeight.import( inputGraph->getVtxWeight() );
  edgeWeight.import( inputGraph->getEdgeWeight() );
  currentState = UNKNOWN;
  FEXIT;
}
#undef __FUNC__
#define __FUNC__ "Graph::Graph( MatrixBase* inputMatrix )"
Graph::Graph( MatrixBase* inputMatrix ){
  FENTER;
  incrementInstanceCount( Graph::MetaData );
  FCALL reset();
  if ( inputMatrix == 0 ) {
    ERROR(SPINDLE_ERROR_ARG_BADPTR,"Invalid constructor arguments.");
    currentState = INVALID;
    FEXIT;
  }
  currentState = UNKNOWN;
  nEdges = -1; // won't know here
  nBoundVtxs = -1; // won't know here
  int nCols = inputMatrix->queryNCols();
  int nRows = inputMatrix->queryNRows();
  if ( nCols != nRows ) { // rectangular matrix.... assume graph with boundary nodes
    if ( nCols > nRows ) { // if wide and short.. just take it
      nVtxs = nRows;
      nBoundVtxs = nCols - nRows;
      adjHead.take( inputMatrix->getColPtr() );
      adjList.take( inputMatrix->getRowIdx() );
    } else {               // if tall and skinny... transpose first
      nVtxs = nCols;
      nBoundVtxs = nRows - nCols;
      MatrixUtils::makeTranspose( inputMatrix );
      adjHead.take( inputMatrix->getColPtr() );
      adjList.take( inputMatrix->getRowIdx() );
    }       
  } else {
    nVtxs = nCols;
    nBoundVtxs = 0;
    adjHead.take( inputMatrix->getColPtr() );
    adjList.take( inputMatrix->getRowIdx() );
    nNonZeros = adjList.size();
  }
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "Graph::Graph( const MatrixBase* inputMatrix )"
Graph::Graph( const MatrixBase* inputMatrix ){
  FENTER;
  incrementInstanceCount( Graph::MetaData );
  FCALL reset();
  FCALL reset();
  if ( inputMatrix == 0 ) {
    ERROR(SPINDLE_ERROR_ARG_BADPTR,"Invalid constructor arguments.");
    currentState = INVALID;
    FEXIT;
  }
  currentState = UNKNOWN;
  nEdges = -1; // won't know here
  nBoundVtxs = -1; // won't know here
  int nCols = inputMatrix->queryNCols();
  int nRows = inputMatrix->queryNRows();
  if ( nCols != nRows ) { // rectangular matrix.... assume graph with boundary nodes
    if ( nCols > nRows ) { // if wide and short.. just take it
      nVtxs = nRows;
      nBoundVtxs = nCols - nRows;
      adjHead.import( inputMatrix->getColPtr() );
      adjList.import( inputMatrix->getRowIdx() );
    } else {               // if tall and skinny... transpose first
      nVtxs = nCols;
      nBoundVtxs = nRows - nCols;
      MatrixBase temp;
      MatrixUtils::deepCopy( inputMatrix, &temp ); // need our own copy
      MatrixUtils::makeTranspose( &temp );
      adjHead.take( temp.getColPtr() );
      adjList.take( temp.getRowIdx() );
    }       
  } else {
    nVtxs = nCols;
    nBoundVtxs = 0;
    adjHead.import( inputMatrix->getColPtr() );
    adjList.import( inputMatrix->getRowIdx() );
    nNonZeros = adjList.size();
  }
  FEXIT;
}
#undef __FUNC__

#ifdef MATLAB_MEX_FILE
#define __FUNC__ "Graph::Graph( const mxArray * mat )"
Graph::Graph( const mxArray* mat ) { 
  FENTER;
  incrementInstanceCount( Graph::MetaData );
  FCALL reset();
  if ( mat == 0 ) {
    ERROR(SPINDLE_ERROR_ARG_BADPTR,"Invalid constructor arguments.");
    currentState = INVALID;
    FEXIT;
  }
  nVtxs = mxGetN( mat );
  if ( nVtxs != mxGetM( mat ) ) { 
    ERROR(SPINDLE_ERROR_ARG_INVALID,"Input Matlab Matrix is not square.");
    currentState = INVALID;
    FEXIT;
  }
  if ( ! mxIsSparse( mat ) ) { 
    ERROR(SPINDLE_ERROR_ARG_INVALID,"Input Matlab Matrix is not sparse.");
    currentState = INVALID;
    FEXIT;
  }
  adjHead.import( mxGetJc( mat ), nVtxs + 1 );
  nNonZeros = adjHead[ nVtxs ];
  adjList.import( mxGetIr( mat ), nNonZeros );
  nEdges = -1;
  nBoundVtxs = 0;
  //  adjSelf.import( inputGraph->getAdjSelf() );
  // vtxWeight.import( inputGraph->getVtxWeight() );
  // edgeWeight.import( inputGraph->getEdgeWeight() );
  currentState = UNKNOWN;
  FEXIT;
}
#undef __FUNC__
#endif


Graph::~Graph(){
  decrementInstanceCount( Graph::MetaData );
}

#define __FUNC__ "Graph::validate()"
void
Graph::validate() {
  FENTER;
  if ( currentState == VALID ) { FEXIT; }   // if known valid, don't bother.
  if ( currentState == INVALID ) { 
    WARNING("Cannot validate a known invalid object, must \"reset()\" it first.\n");
    FEXIT;
  }

  // 1. Make sure that adjHead and adjList are not null.
  if ( (adjHead.size()==0) || (adjList.size()==0) ) { 
    ERROR(SPINDLE_ERROR_ARG_BADPTR,"Cannot validate Graph with empty \'adjHead\' or \'adjList\'\n");
    currentState=INVALID;
    FEXIT;
  }
  // 2. now determine nVtxs 
  int possibility1 = adjHead.size() - 1;
  int possibility2 = 0;
  {for(const int * cur = adjList.begin(), * stop = adjList.end(); cur != stop; ++cur ) {
    possibility2 = ( possibility2 < *cur ) ? *cur : possibility2;
  }}
  possibility2++;  // add one
  if (( nVtxs >= possibility2 ) && ( nVtxs <= possibility1 )) {
    ;    // nVtxs = nVtxs;
  } else if ( vtxWeight.size() != 0 ) {
    int possibility3 = vtxWeight.size();
    if (( possibility3 >= possibility2 ) && ( possibility3 <= possibility1 )) {
      nVtxs = possibility3;
    } else { 
      nVtxs = possibility2;
    }
  } else { 
    nVtxs = possibility2;
  }
  nEdges = adjHead[nVtxs]/2;
  nBoundVtxs = possibility1 - nVtxs;
  nNonZeros = adjList.size();

  // 2. Now we're ready to verify that each adjList segment lists
  //    the indices in strict increasing order.  We can sort if neccessary, 
  //    but abort if duplicate entries are detected.
  int haveErrors = 0;
  {for (int i=0; i<nVtxs; ++i) {
    haveErrors = 0;
    int lastIdx = -1;  // something less than anything else
    {for (int j=adjHead[i]; j<adjHead[i+1]; ++j) {
      if (lastIdx >= adjList[j]) {
        haveErrors = 1;
	break;
      }
      lastIdx = adjList[j];
    }}
    if ( haveErrors > 0 ) {
      if ( edgeWeight.size() == 0 ) {
	WARNING("Index list for vertex %d not in sorted order.  Trying to compensate", i );
	// sort the adjList for this region again
	sort( adjList.begin() + adjHead[i], adjList.begin() + adjHead[i+1] );
	for (int j=adjHead[i]; j<adjHead[i+1]; j++) {
	  if (lastIdx == adjList[j]) {
	    ERROR(SPINDLE_ERROR_UNEXP_DATA,"Duplicate edges in vertex %d detected.  Validation failed.", i);
	    currentState=INVALID;
	    FEXIT;
	  }
	}
      } else { 
	ERROR(SPINDLE_ERROR_UNEXP_DATA,"Index list for vertex %d not in sorted order.  Cannot compensate.", i );
	currentState=INVALID;
	FEXIT;
      }
    } // end if(haveErrors);
  }}

  // 3. Now that we know all the edges are sorted and unique. 
  //    Check for self edges and set adjSelf
  adjSelf.resize( nVtxs );
  int extractSelfEdges = 0;
  {for (int i=0; i<nVtxs; ++i ) {
    adjSelf[i] = adjHead[i+1];
    for ( int j=adjHead[i]; j<adjHead[i+1]; ++j ) {
      if ( adjList[j]== i ) { // if index
	adjSelf[i] = -j;
	++extractSelfEdges;
	break; // breaks j-loop to next iteration of i-loop
      } 
      if ( adjList[j] > i ) {
	adjSelf[i] = j;
	break; // breaks j-loop to next iteration of i-loop
      }
    }
  }}

  //
  // If we have self edges to extract do so
  //
  if ( extractSelfEdges > 0 ) {
    if ( edgeWeight.size() == 0 ) {
      int *p = adjList.begin();
      int offset = 0;          // offsets adjHead[i] as we are inserting diags
      int j=0;
      for (int i=0; i<nVtxs; i++) { // for each vertex
	adjHead[i] -= offset;
	for ( ; ( (j<adjHead[i+1]) && (adjList[j]<i) ); ++j) {
	  *p++ = adjList[j];
	}
	if ( adjList[j] == i ) { 
	  adjSelf[i] = (-adjSelf[i]) - offset; // recall adjSelf[i]<0 for self edges
	  ++offset;
	  ++j;
	}
	for( ; j < adjHead[i+1]; ++j) {
	  *p++ = adjList[j];
	}
      }
      adjHead[nVtxs] -= offset;
      // we also have to shift cut edges
      while( j < adjList.size() ) {
	*p++ = adjList[j++];
      }
      adjList.resize( p - adjList.begin() );
    } else { 
      int *p = adjList.begin();
      int *p2 = edgeWeight.begin();
      int offset = 0;          // offsets adjHead[i] as we are inserting diags
      int j=0;
      for (int i=0; i<nVtxs; i++) { // for each vertex
	adjHead[i] -= offset;
	for ( ; ( (j<adjHead[i+1]) && (adjList[j]<i) ); ++j) {
	  *p++ = adjList[j];
	  *p2++ = edgeWeight[j];
	}
	if ( adjList[j] == i ) { 
	  adjSelf[i] = (-adjSelf[i]) - offset; // recall adjSelf[i]<0 for self edges
	  ++offset;
	  ++j;
	}
	for( ; j < adjHead[i+1]; ++j) {
	  *p++ = adjList[j];
	  *p2++ = edgeWeight[j];
	}
      }
      adjHead[nVtxs] -= offset;
      // we also have to shift cut edges
      while( j < adjList.size() ) {
	*p++ = adjList[j++];
	*p2++ = edgeWeight[j];
      }
      adjList.resize( p - adjList.begin() );
      edgeWeight.resize( p2 - edgeWeight.begin() );
    }
  }
  nNonZeros = adjList.size();
  nEdges = adjHead[ nVtxs ] / 2;
  nBoundVtxs = adjHead.size() - ( nVtxs + 1 );
  currentState = VALID;
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "bool Graph::setGhostNodes( int nBoundVtxs_, int nCutEdges_ )"
bool
Graph::setGhostNodes( const int nBoundVtxs, const int nCutEdges ) { 
  FENTER;
  bool success = true;
  if ( currentState == INVALID ) { 
    ERROR( SPINDLE_ERROR_WRONGSTATE,"Cannot setGhostNodes() on an invalid Graph");
    success = false;
  } else if ( nBoundVtxs >= nVtxs ) { 
    ERROR( SPINDLE_ERROR_ARG_OUTOFRANGE,"nBoundVtxs larger than nVtxs");
    success = false;
  } else if ( nCutEdges >= nEdges ) { 
    ERROR( SPINDLE_ERROR_ARG_OUTOFRANGE,"nCutEdges larger than nEdges");
    success = false;
  } else if ( adjHead[ nVtxs - nBoundVtxs ] != nEdges - nCutEdges ) {
    ERROR( SPINDLE_ERROR_ARG_OUTOFRANGE,"nCutEdges, nBoundVtxs mismatch");
    success = false;
  } else {
    nVtxs -= nBoundVtxs;
  }    
  currentState = UNKNOWN;
  FRETURN( success );
}
#undef __FUNC__
    
#define __FUNC__ "bool Graph::reset()"
bool
Graph::reset() {
  FENTER;
  currentState = EMPTY;
  nVtxs = 0;
  nEdges = 0;
  nBoundVtxs = 0;
  nNonZeros = 0;

  adjHead.reset();
  adjSelf.reset();
  adjList.reset();
  vtxWeight.reset();
  edgeWeight.reset();
  FRETURN( true );
}
#undef __FUNC__

#define __FUNC__ "void Graph::dump( FILE * fp ) const"
void 
Graph::dump( FILE * fp ) const {
  FENTER;
  fprintf(fp,"\nGraph::dump() {\n");
  fprintf(fp,"   className    = %s\n", MetaData->className );
  fprintf(fp,"   classSize    = %d\n", MetaData->classSize );
  fprintf(fp,"   idNumber     = %d\n", queryIDNumber() );
  switch( currentState ) {
  case INVALID:
    fprintf(fp,"   currentState = INVALID\n");
    break;
  case EMPTY:
    fprintf(fp,"   currentState = EMPTY\n");
    break;
  case UNKNOWN:
    fprintf(fp,"   currentState = UNKNOWN\n");
    break;
  case VALID:
    fprintf(fp,"   currentState = VALID\n");
    break;
  default:
    WARNING("Unrecognized state detected... seting to INVALID");
    Graph* cast_away_const = ( Graph* ) this;
    cast_away_const->currentState = INVALID;
    fprintf(fp,"   currentState = Unrecognized... set to INVALID\n");
    break;
  }
  if ( currentState == VALID ) {
    fprintf( fp,"   nVtxs        = %d\n", nVtxs );
    fprintf( fp,"   nEdges       = %d\n", nEdges );
    fprintf( fp,"   nBoundVtxs   = %d\n", nBoundVtxs );
    fprintf( fp,"   nCutEdges    = %d\n", nNonZeros/2 - nEdges );
    fprintf( fp,"   nNonZeros    = %d\n", nNonZeros );
    fprintf( fp,"   vtxWeights?  = %s\n", (vtxWeight.size()==0)? "No" : "Yes" );
    fprintf( fp,"   edgeWeights? = %s\n", (edgeWeight.size()==0)? "No" : "Yes" );
  }
  fprintf(fp,"}\n");
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "void Graph::prettyPrint( FILE * fp ) const"
void 
Graph::prettyPrint( FILE * fp ) const {
  FENTER;
  const int * adjhead = adjHead.lend();
  const int * adjlist = adjList.lend();
  const int * vweight = ( vtxWeight.size() == nVtxs ) ? vtxWeight.lend() : 0 ;
  const int * eweight = ( edgeWeight.size() == 2*nEdges ) ? edgeWeight.lend() : 0 ;

  if ( eweight == 0 ) {
    if ( vweight == 0 ) {
      for( int i=0; i<nVtxs; ++i ) {
	fprintf(fp, "\n%5d | ",i+1);
	for( int jj=adjhead[i]; jj<adjhead[i+1]; ++jj ) {
	  fprintf(fp," %5d ",adjlist[jj]+1);
	}
      }
    } else { 
      for( int i=0; i<nVtxs; ++i ) {
	fprintf(fp, "\n%5d [%5d] | ",i+1, vweight[i]);
	for( int jj=adjhead[i]; jj<adjhead[i+1]; ++jj ) {
	  fprintf(fp," %5d ",adjlist[jj]+1);
	}
      }
    }
  } else { // if eweight != 0
 if ( vweight == 0 ) {
      for( int i=0; i<nVtxs; ++i ) {
	fprintf(fp, "\n%5d | ",i+1);
	for( int jj=adjhead[i]; jj<adjhead[i+1]; ++jj ) {
	  fprintf(fp," (%5d, %5d) ",adjlist[jj]+1,eweight[jj]);
	}
      }
    } else { 
      for( int i=0; i<nVtxs; ++i ) {
	fprintf(fp, "\n%5d [%5d] | ",i+1, vweight[i]);
	for( int jj=adjhead[i]; jj<adjhead[i+1]; ++jj ) {
	  fprintf(fp," (%5d, %5d) ",adjlist[jj]+1,eweight[jj]);
	}
      }
    }
  }
  fprintf(fp,"\n");
  FEXIT;
}
#undef __FUNC__


#define __FUNC__ "void Graph::loadObject( SpindleArchive& ar )"
void 
Graph::loadObject( SpindleArchive& ar ) {
  FENTER;
  if ( &ar ) ; // keeps compiler from complaining about ar not being used
  ERROR( SPINDLE_ERROR_UNIMP_FUNC,"Unimplemented function" );
  FEXIT;
}
#undef __FUNC__


#define __FUNC__ "void Graph::storeObject( SpindleArchive& ar ) const"
void 
Graph::storeObject( SpindleArchive& ar ) const {
  FENTER;
  if ( &ar ) ; // keeps compiler from complaining about ar not being used
  ERROR( SPINDLE_ERROR_UNIMP_FUNC,"Unimplemented function" );
  FEXIT;
}
#undef __FUNC__


SPINDLE_IMPLEMENT_DYNAMIC( Graph, SpindlePersistant )

