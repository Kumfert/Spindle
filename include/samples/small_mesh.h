/*
 * small_mesh.h
 * 
 * This is a simple 3X3 grid with a five point stencil
 * 
 */

#ifndef TEST_PROBLEM_H_
#define TEST_PROBLEM_H_

const int nvtxs = 9;
const int Aptr[] = { 0, 3, 7, 10, 14, 19, 23, 26, 30, 33 };
const int Aind[] = { 0, 1, 3,
		     0, 1, 2, 4, 
		     1, 2, 5,
		     0, 3, 4, 6,
		     1, 3, 4, 5, 7, 
		     2, 4, 5, 8, 
		     3, 6, 7, 
		     4, 6, 7, 8,
		     5, 7, 8 };
#endif
