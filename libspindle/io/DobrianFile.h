//
// DobrianFile.h -- Florin's format
//
//
//
//  (size)   (highvaluecount) [aka |E|]
//
//  (|V|)  (|E|)   (compressed)   (lower, upper, full square) (values real-symmetric, complex-symmetric)
//
//
//
//

#ifndef DOBRIAN_FILE_H_
#define DOBRIAN_FILE_H_

#ifndef SPINDLE_H_
#include "spindle/spindle.h"
#endif

#ifndef SPINDLE_TEXT_DATA_FILE_H_
#include "spindle/TextDataFile.h"
#endif

#ifndef SPINDLE_MATRIX_BASE_H_
#include "spindle/MatrixBase.h"
#endif

SPINDLE_BEGIN_NAMESPACE

class DobrianFile : public MatrixBase, public TextDataFile {
public:
  bool read( FILE * stream );
  bool write( FILE * stream );

  bool read() { return read( fp ); }
  bool write() { return write( fp ); }
};

SPINDLE_END_NAMESPACE

#endif
