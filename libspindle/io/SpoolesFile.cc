//
// SpoolesFile.cc  -- I/O routines for creating a graph from Spooles file
//
//  $Id: SpoolesFile.cc,v 1.2 2000/02/18 01:31:52 kumfert Exp $
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
#include "spindle/SpoolesFile.h"
#endif

#ifndef SPINDLE_PERMUTATION_H_
#include "spindle/PermutationMap.h"
#endif

#ifdef REQUIRE_OLD_CXX_HEADER_SUFFIX
#include "pair.h"
#include "vector.h"
#include "algo.h"
#else
#include <utility> // using std::pair;
#include <vector>
#include <algorithm>
using namespace std;
#endif

#include "spindle/spindle_func.h"  // defines pair_first_less<T1,T2>

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

bool 
SpoolesFile::read( FILE * stream ) {  
  // Quit if file pointer isn't valid 
  if ((stream == NULL) || feof(stream) || ferror(stream)) {
    cerr << "Bad File Pointer" << endl;
    return 0;
  }
  if (m_isBinary) { 
    return read_binary( stream );
  } else if ( m_isFormatted ) { 
    return read_formatted( stream );
  } else { 
    return false;
  }
}

bool 
SpoolesFile::write( FILE * stream ) {
  // Quit if file pointer isn't valid 
  if ((stream == NULL) || feof(stream) || ferror(stream)) {
    cerr << "Bad File Pointer" << endl;
    return 0;
  }
  if (m_isBinary) { 
    return write_binary( stream );
  } else if ( m_isFormatted ) { 
    return write_formatted( stream );
  } else { 
    return false;
  }
}

bool 
SpoolesFile::read_formatted( FILE * stream ) {  
  int totVWgt;
  int totEWgt;
  if ( fscanf( stream, "%d%d%d%d%d%d", &m_type, &nVtxs, &nBoundVtxs, 
	       &nEdges, &totVWgt, &totEWgt )
       != 6 ) { 
    cerr << "Error, couldn't read in all six initial SpoolesFile parameters" << endl;
    return false;
  }
  // now read in the IVL type;
  
  // first read in type, nlists, and nindices
  int IVL_type;
  int IVL_nLists;
  int IVL_nIdxs;

  if ( fscanf( stream, "%d%d%d", &IVL_type, &IVL_nLists, &IVL_nIdxs ) != 3 ) { 
    cerr << "Error, couldn't read in all three initial IVL parameters" << endl;
    return false;
  }
  if ( IVL_nLists != nVtxs ) { 
    cerr << "Warning, IVL_nLists = " << IVL_nLists << " !=  nVtxs = " << nVtxs << endl;
  }
  adjHead.resize( nVtxs + 1 );
  // load in the size of each list.
  int * ptr = adjHead.begin();
  *ptr++ = 0;
  for ( int i=0; i<nVtxs; ++i ) { 
    if ( fscanf( stream,"%d", ptr) ) { 
      break;
    }
    ++ptr;
  }
  // now do a cumulative sum on adjHead.
  ptr = adjHead.begin();
  for ( int i=0; i<nVtxs; ++i ) { 
    ptr[i+1] += ptr[i];
  }

  adjList.resize( IVL_nIdxs );
  ptr = adjList.begin();
  for ( int i=0; i<IVL_nIdxs; ++i ) { 
    if ( fscanf( stream,"%d", ptr ) ) { 
      break;
    }
    ++ptr;
  }
  
  if ( m_type % 2 == 1 ) { 
    // if vertex weights are present
    int nvtot = nVtxs + nBoundVtxs;
    int wgt = 0;
    vtxWeight.resize( nvtot );
    ptr = vtxWeight.begin();
    for ( int i=0; i<nvtot; ++i ) { 
      if ( fscanf( stream,"%d", ptr ) ) { 
	break;
      }
      wgt += *ptr;
      ++ptr;
    }
    if ( wgt != totVWgt ) { 
      cerr << "Error: Declared total vtx weight = " << totVWgt 
	   << " but only read in a total of = " << wgt << endl;
    }
  }
  if ( m_type >= 2 ) { 
    // edge weights are present
    // read in another IVL object, this time for edge weights
    int junk; //skip over the IVL data and the adjHeads.
    for ( int i=-3; i<IVL_nLists; ++i ) { 
      fscanf( stream, "%d", &junk );
    }
    edgeWeight.resize( IVL_nIdxs );
    ptr = edgeWeight.begin();
    junk = 0;
    for ( int i=0; i<IVL_nIdxs; ++i ) { 
      if ( fscanf( stream,"%d", ptr ) ) { 
	break;
      }
      junk += *ptr;
      ++ptr;
    }
    if ( junk != totEWgt ) { 
      cerr << "Error: Declared total edge weight = " << totEWgt 
	   << " but only read in a total of = " << junk << endl;
    }      
  } // end if ewgts
  nNonZeros = IVL_nIdxs;
    
  return true;
}    


bool 
SpoolesFile::read_binary( FILE * stream ) {  
  int itemp[6];

  if ( fread((void *) itemp, sizeof(int), 6, stream ) != 6 ) {
    cerr << "Error, couldn't read in all six initial SpoolesFile parameters" << endl;
    return false;
  }
  m_type = itemp[0];
  nVtxs = itemp[1];
  nBoundVtxs = itemp[2];
  nEdges = itemp[3];
  int totVWgt = itemp[4];
  int totEWgt = itemp[5];
  // now read in the IVL type;
  
  // first read in type, nlists, and nindices
  if ( fread((void *) itemp, sizeof(int),3,stream) != 3 ) {
    cerr << "Error, couldn't read in all three initial IVL parameters" << endl;
    return false;
  }
  //  int IVL_type = itemp[0];
  int IVL_nLists = itemp[1];
  int IVL_nIdxs = itemp[2];
  if ( IVL_nLists != nVtxs ) { 
    cerr << "Warning, IVL_nLists = " << IVL_nLists << " !=  nVtxs = " << nVtxs << endl;
  }
  adjHead.resize( nVtxs + 1 );
  int * ptr = adjHead.begin();
  *ptr++ = 0;
  // load in the size of each list.
  if ( fread( (void *) ptr, sizeof(int), nVtxs, stream) != (unsigned ) nVtxs ) { 
    cerr << "Error, couldn't read in all " << nVtxs << " adjHeads" << endl;
  }

  // now do a cumulative sum on adjHead.
  ptr = adjHead.begin();
  for ( int i=0; i<nVtxs; ++i ) { 
    ptr[i+1] += ptr[i];
  }

  adjList.resize( IVL_nIdxs );
  ptr = adjList.begin();
  if ( fread( (void *) ptr, sizeof(int), IVL_nIdxs, stream) != (unsigned) IVL_nIdxs ) { 
    cerr << "Error, couldn't read in all " << IVL_nIdxs << " adjacency indices" << endl;
  }

  if ( m_type % 2 == 1 ) { 
    // if vertex weights are present
    int nvtot = nVtxs + nBoundVtxs;
    int wgt = 0;
    vtxWeight.resize( nvtot );
    ptr = vtxWeight.begin();
    if ( fread( (void *) ptr, sizeof(int), nvtot, stream ) != (unsigned) nvtot ) { 
      cerr << "Error: didn't read in all " << nvtot << "vertex weights;" << endl;
    }
    for ( int i =0; i<nvtot; ++i ) { 
      wgt += ptr[i];
    }
    if ( wgt != totVWgt ) { 
      cerr << "Error: Declared total vtx weight = " << totVWgt 
	   << " but only read in a total of = " << wgt << endl;
    }
  }
  if ( m_type >= 2 ) { 
    // edge weights are present
    // read in another IVL object, this time for edge weights
    if ( fread((void *) itemp, sizeof(int),3,stream) != 3 ) {
      cerr << "Error, couldn't read in all three initial IVL parameters for edgeWeights" << endl;
      return false;
    }
    
    int * junk = new int[ nVtxs ];
    if ( fread( (void *) junk, sizeof(int), nVtxs, stream) != (unsigned) nVtxs ) { 
      cerr << "Error, couldn't read in all " << nVtxs << " edgeWeights" << endl;
    }

    edgeWeight.resize( IVL_nIdxs );
    ptr = edgeWeight.begin();
    if ( fread( (void *) ptr, sizeof(int), IVL_nIdxs, stream) != (unsigned ) IVL_nIdxs ) { 
      cerr << "Error, couldn't read in all " << IVL_nIdxs << " edge weights" << endl;
    }

    int wgt = 0;
    for ( int i=0; i<IVL_nIdxs; ++i ) { 
      wgt += *ptr;
    }
    if ( wgt != totEWgt ) { 
      cerr << "Error: Declared total edge weight = " << totEWgt 
	   << " but only read in a total of = " << wgt << endl;
    }      
  } // end if ewgts
  nNonZeros = IVL_nIdxs;
    
  return true;
}    

bool 
SpoolesFile::write_binary( FILE * stream ) {
  cerr << "ERROR: \'SpoolesFile::write_binary\' not yet implemented" << endl;
  return false;
}

bool 
SpoolesFile::write_formatted( FILE * stream ) {
  cerr << "ERROR: \'SpoolesFile::write_formatted\' not yet implemented" << endl;
  return false;
}
