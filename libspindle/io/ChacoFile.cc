//
// ChacoFile.cc  -- I/O routines for creating a graph from Chaco file
//
//  $Id: ChacoFile.cc,v 1.2 2000/02/18 01:31:50 kumfert Exp $
//
//  Gary Kumfert, Old Dominion University
//  Copyright(c) 1997, Old Dominion University.  All rights reserved.
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
//
//

#ifndef SPINDLE_METIS_FILE_H_
#include "spindle/ChacoFile.h"
#endif

#ifndef SPINDLE_PERMUTATION_H_
#include "spindle/PermutationMap.h"
#endif

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include "pair.h"
#include "vector.h"
#include "algo.h"
#else
#include <utility> // use std::pair
#include <vector>
#include <algorithm>
#endif

#include "spindle/spindle_func.h"  // defines pair_first_less<T1,T2>

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

bool 
ChacoFile::read( FILE * stream ) {  
  // Quit if file pointer isn't valid 
  if ((stream == NULL) || feof(stream) || ferror(stream)) {
    cerr << "Bad File Pointer" << endl;
    return 0;
  }

  const int bufferSz = 128*1024;
  char buffer[ bufferSz ];
  char * info = buffer;

  // skip over commented lines
  do {
    info = fgets(buffer, bufferSz,stream);
  } while ( (info != NULL) && ( info[0] == '%') );

  char *token;    // pointer to current token
  if ( (info != NULL) && (token = strtok(info," \t")) ) {
    nVtxs = atoi( token );
  }

  if ( (token = strtok(0," \t")) ) {
    nEdges = atoi( token );
    nNonZeros = 2*nEdges;
  }
  nBoundVtxs = 0; // no boundary vertices in Chaco files
  
  bool read_edgeWeights; // read edge weights?
  bool read_vtxWeights; // read vertex weights?
  bool read_vtxNumbers;
  if ( (token = strtok(0," \t")) ) {
    int temp = atoi(token);
    read_vtxWeights = ((temp/10) == 1);
    read_edgeWeights = (((temp%10)/10) == 1);
    read_vtxNumbers = (((temp%100)/10) == 1);
  } else {
    read_vtxWeights = false;
    read_edgeWeights = false;
    read_vtxNumbers = false;
  }

  PermutationMap vtxPermutationMap(nVtxs);
  int *old2new = 0;
  vector< pair<int,int> > pairList( nNonZeros );
  adjHead.resize( nVtxs + 1 );
  adjList.resize( nNonZeros );
  if ( read_vtxWeights ) {
    vtxWeight.resize( nVtxs );
  }
  if ( read_edgeWeights ) {
    edgeWeight.resize( nNonZeros );
    // pairList.reserve( nNonZeros );
  }
  if ( read_vtxNumbers ) {
    old2new = vtxPermutationMap.getOld2New().give();
  }

  int curIdx = 0;
  {for (int i=0; i<nVtxs; i++) { // For every vertex in the graph ...
    
    // ... start a new column in the matrix
    adjHead[i] = curIdx;
    
    // ... get next line of data from the file
    do {
      if ( ( info = fgets(buffer,bufferSz,stream) ) == NULL) {
	cerr << "Input file ended prematurely." << endl;
	return 0;
      }
    } while ( ( info != NULL ) && ( info[0] == '%' ) );
    token = strtok( info," \t" );
    
    // ... get the vertex permutation (if neccessary)
    if ( read_vtxNumbers ) {
      old2new[i]=atoi( token );
      token = strtok( 0, " \t" );
    }

    // ... strip of diagonal value if neccessary.
    if (read_vtxWeights) {
      vtxWeight[i] = atoi( token );
      token = strtok( 0, " \t" );
    } 

    // ... now insert the diagonal
    //// no!  no diagonals in a graph
    //    if ( read_edgeWeights ) {
    //      if ( read_vtxWeights ) {
    //	        pairList[curIdx++] = pair<int,int>(i,vtxWeight[i]);
    //      } else {
    //   	pairList[curIdx++] = pair<int,int>(i,0);
    //      }
    //    } else {
    //      adjList[curIdx++] = i;
    //    }

    // for every element on the line
    while( (curIdx < nNonZeros) && (token != NULL) && (*token != '\n') ) {  
      int edge = atoi(token);
      token = strtok( 0, " \t" );
      int edge_weight = 1;      
      if (read_edgeWeights) {
	edge_weight = atoi(token);
	token = strtok( 0, " \t" );
	pairList[curIdx++] = pair<int,int>(edge-1, edge_weight);
      } else {
	adjList[curIdx++] = edge-1;
      }
    }
  }}
  adjHead[nVtxs] = curIdx;

  // Now do the post-processing...
  
  // ... first sort the edge lists
  if ( read_edgeWeights) {
    for(int i=0;i<nVtxs;i++) {
      sort( pairList.begin()+adjHead[i], pairList.begin() +adjHead[i+1], pair_first_less<int,int>() );
    }
  } else {
    for (int i=0;i<nVtxs;i++) {
      sort( adjList.begin()+adjHead[i], adjList.begin()+adjHead[i+1] );
    }
  }
    
  // ... next, reorder the edge list if the vertices are out of order
  if ( read_vtxNumbers ) {
    vtxPermutationMap.getOld2New().take( old2new, nVtxs );
    vtxPermutationMap.validate();
    if ( ! vtxPermutationMap.isValid() ) {
      cerr << "Vertex PermutationMap problem" << endl;
    }
    const int* new2old = vtxPermutationMap.getNew2Old().lend();
    SharedArray<int> permAdjHead( nVtxs + 1 );
    if ( read_edgeWeights ) {
      vector< pair< int, int > > permPairList( nVtxs );
      int newIdx = 0;
      for( int i=0; i<nVtxs; i++ ) {
	permAdjHead[i] = newIdx;
	int p=new2old[i];
	for (int j=adjHead[p]; j<adjHead[p+1]; j++ ) {
	  permPairList[newIdx++] = pairList[j];
	}
      }
      permAdjHead[nVtxs] = adjHead[nVtxs];
      pairList.swap( permPairList );
    } else {
      SharedArray<int> permAdjList( nVtxs );
      int newIdx = 0;
      for( int i=0; i<nVtxs; i++ ) {
	permAdjHead[i] = newIdx;
	int p=new2old[i];
	for (int j=adjHead[p]; j<adjHead[p+1]; j++ ) {
	  permAdjList[newIdx++] = adjList[j];
	}
      }
      permAdjHead[nVtxs] = adjHead[nVtxs];
      adjList.swap( permAdjList );
    }
    adjHead.swap( permAdjHead );
  }

  // ... finally, extract the pairList into adjList and edgeWeight
  //     if necessary
  if ( read_edgeWeights ) {
    for( int i=0; i<nVtxs; i++ ) {
      for ( int j=adjHead[i]; j<adjHead[i+1]; j++ ) {
	adjList[j] = pairList[i].first;
	edgeWeight[j] = pairList[i].second;
      }
    }
  }
  return true;
}    

bool 
ChacoFile::write( FILE * stream ) {
  int myCode;
  bool write_edgeWeights;
  bool write_vtxWeights;
  if (CODE == -1) {
    write_edgeWeights = ( edgeWeight.notNull() );
    write_vtxWeights = ( vtxWeight.notNull() );
    myCode = 10 * int( write_edgeWeights ) + int( write_vtxWeights );
  } else {
    myCode = CODE;
    write_edgeWeights = ((myCode / 10) == 1);
    write_vtxWeights = ((myCode % 10) == 1);
  }

  fprintf(stream,"%%%%ChacoFile\n");
  // Now a big case statement to avoid conditional jumps in a loop.
  // This runs faster especially since the condition is invariant
  // on the loop.
  switch(myCode) {
  case 0: // No vertex weights, No edge weights
    fprintf(stream,"%d %d\n", nVtxs, adjHead[nVtxs]/2 );
    {for (int j=0; j<nVtxs; j++) {
      int i=adjHead[j];
      for ( ; (i<adjHead[j+1]) && (adjList[i]<j); i++) {
	fprintf(stream,"%d ",adjList[i]+1);
      }
      if ((i<adjHead[j+1]) && (adjList[i]==j)) { i++; }
      for ( ; i<adjHead[j+1]; i++) {
	fprintf(stream,"%d ",adjList[i]+1);
      }
      fprintf( stream, "\n");
    }}
    break;
  case 1:    // edge weights, no vertex weights 
    fprintf( stream, "%d %d 1\n", nVtxs, (adjHead[nVtxs]-nVtxs)/2 );
    if ( edgeWeight.isNull() ) { // edge weight == 1
      {for (int j=0; j<nVtxs; j++) {
	int i=adjHead[j];
	for ( ; (i<adjHead[j+1]) && (adjList[i]<j); i++) {
	  fprintf(stream,"%d 1 ",adjList[i]+1);
	}
	if ((i<adjHead[j+1]) && (adjList[i]==j)) { i++; }
	for ( ; i<adjHead[j+1]; i++) {
	  fprintf(stream,"%d 1 ",adjList[i]+1);
	}
	fprintf( stream, "\n");
      }}
    } else { // edge weight == edgeWeight[i]
      {for (int j=0; j<nVtxs; j++) {
	int i=adjHead[j];
	for ( ; (i<adjHead[j+1]) && (adjList[i]<j); i++) {
	  fprintf(stream,"%d %d ",adjList[i]+1, edgeWeight[i]);
	}
	if ((i<adjHead[j+1]) && (adjList[i]==j)) { i++; }
	for ( ; i<adjHead[j+1]; i++) {
	  fprintf(stream,"%d %d ",adjList[i]+1, edgeWeight[i]);
	}
	fprintf( stream, "\n");
      }}
    }
    break;
  case 10: // vertex weights, but no edge weights
    fprintf( stream, "%d %d 10\n", nVtxs, (adjHead[nVtxs]-nVtxs)/2 );
    if ( vtxWeight.isNull() && edgeWeight.isNull() ) { // vertex weights are 1
       {for (int j=0; j<nVtxs; j++) {
	fprintf( stream, "1 " );
	int i=adjHead[j];
	for ( ; (i<adjHead[j+1]) && (adjList[i]<j); i++) {
	  fprintf( stream, "%d ", adjList[i]+1 );
	}
	if ((i<adjHead[j+1]) && (adjList[i]==j)) { i++; }
	for ( ; i<adjHead[j+1]; i++) {
	  fprintf( stream, "%d ", adjList[i]+1 );
	}
	fprintf( stream, "\n");
       }}
    } else if ( vtxWeight.isNull() /* && edgeWeight.notNull() */ ) { // vertex weights are edgeWeight[diag]
      {for (int j=0; j<nVtxs; j++) {
	int diag;
	for(diag=adjHead[j]; diag<adjHead[j+1]; diag++) { 
	  if (adjList[diag] == j) {
	    break;
	  }
	}
	fprintf( stream, "%d ", edgeWeight[diag] );
	int i=adjHead[j];
	for( ; i<diag; i++) {
	  fprintf( stream, "%d ", adjList[i]+1 );
	}
	if ((i<adjHead[j+1]) && (adjList[i]==j)) { i++; }
	for ( ; i<adjHead[j+1]; i++) {
	  fprintf( stream, "%d ", adjList[i]+1 );
	}
	fprintf( stream, "\n");
      }}
    } else { // vertex weights are vtxWeight[j]
      {for (int j=0; j<nVtxs; j++) {
	fprintf( stream, "%d ", vtxWeight[j] );
	int i=adjHead[j];
	for ( ; (i<adjHead[j+1]) && (adjList[i]<j); i++) {
	  fprintf( stream, "%d ", adjList[i]+1 );
	}
	if ((i<adjHead[j+1]) && (adjList[i]==j)) { i++; }
	for ( ; i<adjHead[j+1]; i++) {
	  fprintf( stream, "%d ", adjList[i]+1 );
	}
	fprintf( stream, "\n");
      }}
    }
    break;
  case 11: // vertex weights and edge weights
    fprintf( stream, "%d %d 11\n", nVtxs, (adjHead[nVtxs]-nVtxs)/2 );
    if ( edgeWeight.isNull() ) { // vertex weights are 1
      fprintf( stream, "1 ");
      {for (int j=0; j<nVtxs; j++) {
	int i=adjHead[j];
	for ( ; i<adjHead[j+1]; i++) {
	  fprintf( stream, "%d 1 ", adjList[i]+1);
	}
      }}
    } else {  // vertex weights are edgeWeight[diag], edge weights are edgeWeight[i]
      {for (int j=0; j<nVtxs; j++) {
	int diag;
	for(diag=adjHead[j]; diag<adjHead[j+1]; diag++) { 
	  if (adjList[diag] == j) {
	    break;
	  }
	}
	fprintf( stream, "%d ", edgeWeight[diag] );
	int i=adjHead[j];
	for( ; i<diag; i++) {
	  fprintf( stream, "%d %d ", adjList[i]+1, edgeWeight[i]);
	}
	if ((i<adjHead[j+1]) && (adjList[i]==j)) { i++; }
	for ( ; i<adjHead[j+1]; i++) {
	  fprintf( stream, "%d %d ", adjList[i]+1, edgeWeight[i]);
	}
	fprintf( stream, "\n");
      }}
    }
    break;
  default:
    cerr << "Error: Unexpected Case " << myCode << endl;
    return false;
  }
  return true;
}

