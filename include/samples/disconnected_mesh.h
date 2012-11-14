/*
 * disconnected_mesh.h
 *
 * This is a permuted reducable mesh
 *    It contains two 3X3 grids with a five point stencil
 *    four vertices with a self edge
 *    and four vertices with no edge.
 *
 *
 *
 *  no edges:   0,  7,  9, 18
 *  self edges: 1, 10, 11, 19
 *
 *  connected components:
 *
 *           24 - 12 - 25          5 - 23 -  2
 *            |    |    |          |    |    |
 *           22 -  3 -  4         17 - 13 - 21
 *            |    |    |          |    |    |
 *            8 - 14 - 15         20 -  6 - 16
 *
 *
 */


#ifndef TEST_PROBLEM_H_
#define TEST_PROBLEM_H_

const int nvtxs = 26;
const int Aptr[] = {  0,  0,  1,  4,  9, 13, 16, 20, 20, 23, 23, 24, 25, 29, 34, 
		     38, 41, 44, 48, 48, 49, 52, 56, 60, 64, 67, 70 };
const int Aind[] = { //
                      1, 
		      2, 21, 23,
		      3,  4, 12, 14, 22,
		      3,  4, 15, 25,
		      5, 17, 23,
		      6, 13, 16, 20,
		      // 
		      8, 14, 22,
		      //
		     10,
		     11,
		      3, 12, 24, 25,
		      6, 13, 17, 21, 23,
		      3,  8, 14, 15,
		      4, 14, 15, 
		      6, 16, 21,
		      5, 13, 17, 20,
		      //
		     19, 
		      6, 17, 20,
		      2, 13, 16, 21,
		      3,  8, 22, 24,
		      2,  5, 13, 23,
		     12, 22, 24,
		      4, 12, 25 };
		   

#endif


