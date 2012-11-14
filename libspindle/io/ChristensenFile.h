//
// ChristensenFile.h -- Peter Christensen's Format
//
//
//
//  *In the first row is n, the size of the K matrix:
//   K=[K11 K12;K12' K22] is n*n.
//  
//  *In the second row is nn, the number of non-zero elements
//  in the upper half of K (including diagonal terms).
//  
//  *In the third row is n2, the number of columns in K22.
//  
//  *In the next nn rows there are 2 values per row, representing
//  the row and column of the non-zero elements.
//  
//  *In the last n2 rows are the columns that are supposed to 
//  belong to K22 (and hence be moved to the right in the K matrix).
//
//
//
//
//

#ifndef CHRISTENSEN_FILE_H_
#define CHRISTENSEN_FILE_H_

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

class ChristensenFile : public MatrixBase, public TextDataFile {

private:
  vector<int> colsInBlock22;

public:
  bool read( FILE * stream );
  bool write( FILE * stream );

  bool read() { return read( fp ); }
  bool write() { return write( fp ); }

  int queryNColsInBlock22() const;
  vector<int>::const_iterator block22_begin() const;
  vector<int>::const_iterator block22_end() const;
  

};

inline int 
ChristensenFile::queryNColsInBlock22() const { 
  return colsInBlock22.size(); 
}

inline	vector<int>::const_iterator 
ChristensenFile::block22_begin() const { 
  return colsInBlock22.begin();
}

inline	vector<int>::const_iterator 
ChristensenFile::block22_end() const {
  return colsInBlock22.end();
}
  
SPINDLE_END_NAMESPACE

#endif



