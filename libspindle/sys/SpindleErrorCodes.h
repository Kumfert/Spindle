//
// SpindleErrorCodes.h
//

#ifndef SPINDLE_ERROR_CODES_H_
#define SPINDLE_ERROR_CODES_H_

/** @name General Error Codes 
 *
 * in #SpindleErrorCodes.h#
 */
//@{
/// undiagnosed error
#define SPINDLE_ERROR_MISC            -10   
/// unable to allocate requested memory (using malloc)
#define SPINDLE_ERROR_MEM             -11
/// no support for requested operation 
#define SPINDLE_ERROR_SUP             -12
/// signal received 
#define SPINDLE_ERROR_SIG             -13   
/// floating point exception 
#define SPINDLE_ERROR_FP              -14
/// object itself is in wrong state
#define SPINDLE_ERROR_WRONGSTATE      -15
 				      
/// input argument, out of range 
#define SPINDLE_ERROR_ARG_OUTOFRANGE  -21
/// invalid pointer argument 
#define SPINDLE_ERROR_ARG_BADPTR      -22
/// two arguments not allowed to be the same 
#define SPINDLE_ERROR_ARG_IDN         -23
/// object in argument is in wrong state, e.g. unassembled mat 
#define SPINDLE_ERROR_ARG_WRONGSTATE  -24
/// some other aspect of the argument renders it invalid
#define SPINDLE_ERROR_ARG_INVALID     -25
 				      
/// unable to open file 
#define SPINDLE_ERROR_FILE_OPEN       -30
/// unable to read from file 
#define SPINDLE_ERROR_FILE_READ       -31
/// unable to write to file 
#define SPINDLE_ERROR_FILE_WRITE      -32
/// file ended prematurely
#define SPINDLE_ERROR_FILE_TOOSHORT   -33
/// other miscellaneous file I/O error
#define SPINDLE_ERROR_FILE_MISCERR    -34
				      
/// unexpected case
#define SPINDLE_ERROR_UNEXP_CASE      -40
/// unexpected data/data corruption
#define SPINDLE_ERROR_UNEXP_DATA      -41

/// unimplemented function / member function
#define SPINDLE_ERROR_UNIMP_FUNC      -50

/// error propagation
#define SPINDLE_ERROR                -100

//@}
#endif
