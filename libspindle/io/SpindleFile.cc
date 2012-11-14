//
// SpindleFile.cc -- raw unbuffered binary I/O file
//
// $Id: SpindleFile.cc,v 1.2 2000/02/18 01:31:51 kumfert Exp $
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

#include "spindle/spindle.h"

#include "spindle/SpindleFile.h"

#ifndef SPINDLE_SYSTEM_H_
#include "spindle/SpindleSystem.h"
#endif

#include <string.h>

#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

 
#ifdef COMPRESS
#define DO_COMPRESS    COMPRESS " - -c > %s"
#endif
#ifdef UNCOMPRESS
#define DO_UNCOMPRESS  UNCOMPRESS " -c %s"
#endif

#ifdef GZIP
#define DO_GZIP        GZIP   " -9 - -c > %s"
#endif
#ifdef GUNZIP
#define DO_GUNZIP      GUNZIP " -c %s"
#endif GUNZIP 


#ifdef PKZIP
#define DO_PKZIP       PKZIP   " %s -q -"
#endif 
#ifdef PKUNZIP
#define DO_PKUNZIP     PKUNZIP " -p %s"
#endif

#ifdef _MSC_VER

#define popen _popen
#define pclose _pclose

#endif

SPINDLE_IMPLEMENT_DYNAMIC( SpindleFile, SpindleBaseClass )

#ifdef __FUNC__
#undef __FUNC__
#endif

#define __FUNC__ "SpindleFile::SpindleFile()"
SpindleFile::SpindleFile() { 
  FENTER;
  incrementInstanceCount( SpindleFile::MetaData );
  closeOnDelete_ = dont;
  filename_ = 0;
  fp_ = 0;
  FEXIT;
}
#undef __FUNC__


#define __FUNC__ "SpindleFile::SpindleFile( FILE * fp )"
SpindleFile::SpindleFile( FILE * fp ) {
  FENTER;
  ASSERT(fp != 0, "FILE pointer cannot be null.");

  incrementInstanceCount( SpindleFile::MetaData );
  closeOnDelete_ = dont;
  filename_ = 0;
  fp_ = fp;
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "SpindleFile::SpindleFile( int filedes, const char * openFlags )"
SpindleFile::SpindleFile( int filedes, const char * openFlags ) {
  FENTER;
  ASSERT( filedes > 0, "Negative file descriptor.");

  incrementInstanceCount( SpindleFile::MetaData );
  if (! open( filedes, openFlags ) ) { 
    filename_ = 0;
    //    currentState_ = INVALID;
    FEXIT;
  }
  filename_ = "unknown";
  //  currentState_ = VALID;
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "SpindleFile::SpindleFile( const char* filename, const char* openFlags )"
SpindleFile::SpindleFile( const char* filename, const char* openFlags ) {
  FENTER;
  ASSERT( filename!=0, "`filename' is a null pointer.");
  ASSERT( openFlags!=0, "`openFlags' is a null pointer.");

  incrementInstanceCount( SpindleFile::MetaData );
  if ( ! this->open( filename, openFlags ) ) { 
    filename_ = 0;
    //    currentState_ = INVALID;
    FEXIT;
  }
  filename_  = filename;
  //  currentState_ = VALID;
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "bool SpindleFile::open( int filedes, const char* openFlags )"
bool SpindleFile::open( int filedes, const char* openFlags ) {
  FENTER;
  if ((fp_ = fdopen( filedes, openFlags )) == NULL ) {
    //    currentState_ = INVALID;
    closeOnDelete_ = dont;
    FRETURN(false);
  }
  closeOnDelete_ = file;
  FRETURN(true);
}
#undef __FUNC__

#define __FUNC__ "bool SpindleFile::open( const char* filename, const char* openFlags )"
bool SpindleFile::open( const char* filename, const char* openFlags ) {
  FENTER;
  char buffer[128];
  int len = strlen(filename);  // len does not include null char
 
  if (openFlags[0] == 'r') { // read the file
      // check for pipes
      if (strchr(filename,'|') != NULL) {
	strcpy(buffer, filename);  // make a local copy
	while( (buffer[len-1] == ' ') || (buffer[len-1] == '|')) { // remove trailing pipe and spaces.
	  len--;
	  buffer[len] = '\0';
	}
	if ((fp_ = popen( buffer, openFlags )) == NULL) { 
	  cerr << "Error: Cannot open pipe \'" << filename << "\'" <<  endl;
	  FRETURN(false);
	}
	closeOnDelete_ = pipe;
      } 
#ifdef DO_UNCOMPRESS
      else if ( (filename[len-2] == '.') && (filename[len-1]=='Z') ) {
	// open uncompress pipe
	sprintf( buffer, DO_UNCOMPRESS, filename);
	if ((fp_ = popen( buffer, openFlags )) == NULL) { 
	  cerr << "Error: Cannot open pipe \'" << buffer << "\'" <<  endl;
	  FRETURN(false);
	}
	closeOnDelete_ = pipe;
      } 
#endif
#ifdef DO_GUNZIP 
      else if ( (filename[len-3] == '.') && (filename[len-2]=='g') && (filename[len-1]=='z') ) {
	// open gunzip pipe
	sprintf( buffer, DO_GUNZIP, filename);
	if ((fp_ = popen( buffer, openFlags )) == NULL) { 
	  cerr << "Error: Cannot open pipe \'" << buffer << "\'" <<  endl;
	  FRETURN(false);
	}
	closeOnDelete_ = pipe;
      }
#endif
#ifdef DO_PKUNZIP
      else if ( (filename[len-4] == '.') && (filename[len-3]=='z') 
		&&  (filename[len-2]=='i') && (filename[len-1]=='p') ) {
	// open zip pipe
	sprintf( buffer, DO_PKUNZIP, filename);
	if ((fp_ = popen( buffer, openFlags )) == NULL) { 
	  cerr << "Error: Cannot open pipe \'" << buffer << "\'" <<  endl;
	  FRETURN(false);
	}
      } 
#endif
      else {
	// open regular file
	if ((fp_ = fopen( filename, openFlags )) == NULL) { 
	  cerr << "Error: Cannot open file \'" << filename << "\'" << endl;
	  FRETURN(false);
	}
	closeOnDelete_ = file;
      }  
  } else if (openFlags[0]=='w') { // write the file
    // check for pipes
    if (strchr(filename,'|') != NULL) {
      const char * temp = filename;
      while (( *temp == ' ') || (*temp  == '|')) { // skip leading pipe and spaces.
	temp++;
      }
      if ((fp_ = popen( temp, openFlags )) == NULL) { 
	cerr << "Error: Cannot open pipe \'" << filename << "\'" <<  endl;
	FRETURN(false);
      }
      closeOnDelete_ = pipe;
    }
#ifdef DO_COMPRESS
    else if ( (filename[len-2] == '.') && (filename[len-1]=='Z') ) {
      // open uncompress pipe
      sprintf( buffer, DO_COMPRESS, filename);
      if ((fp_ = popen( buffer, openFlags )) == NULL) { 
	cerr << "Error: Cannot open pipe \'" << buffer << "\'" <<  endl;
	FRETURN(false);
      }
      closeOnDelete_ = pipe;
    }
#endif
#ifdef DO_GZIP
    else if ( (filename[len-3] == '.') && (filename[len-2]=='g') && (filename[len-1]=='z') ) {
      // open gunzip pipe
      sprintf( buffer, DO_GZIP, filename);
      if ((fp_ = popen( buffer, openFlags )) == NULL) { 
	cerr << "Error: Cannot open pipe \'" << buffer << "\'" <<  endl;
	FRETURN(false);
      }
      closeOnDelete_ = pipe;
    }
#endif
#ifdef DO_PKZIP
    else if ( (filename[len-4] == '.') && (filename[len-3]=='z') &&  (filename[len-2]=='i') 
	      && (filename[len-1]=='p') ) {
      // open zip pipe
      // sprintf( buffer, ZIP, filename);
      // if ((fp_ = popen( buffer, openFlags )) == NULL) { 
      //    cerr << "Error: Cannot open pipe \'" << buffer << "\'" <<  endl;
      //  FRETURN(false);
      // }
      cerr << "Error: Cannot open pipe \'" << filename << "\'" <<  endl;
      cerr << "       PKZIP can't create zipfiles from a pipe!" << endl;
      FRETURN(false);
    }
#endif
    else {
      // open regular file
      if ((fp_ = fopen( filename, openFlags )) == NULL) { 
	cerr << "Error: Cannot open file \'" << filename << "\'" << endl;
	FRETURN(false);
      }
      closeOnDelete_ = file;
    }  
  } else { 
    cerr << "Error: illegal file flags \"" << openFlags << "\"" << endl;
    FRETURN(false);
  }
  FRETURN(true);
}
#undef __FUNC__


#define __FUNC__ "SpindleFile::~SpindleFile()"
SpindleFile::~SpindleFile() {
  FENTER;
  if ( (fp_ != 0) && (closeOnDelete_ != dont) ) {
    FCALL close();
  }
  decrementInstanceCount( SpindleFile::MetaData );
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "void SpindleFile::rename(const char* oldFilename, const char* newFilename )"
void
SpindleFile::rename(const char* oldFilename, const char* newFilename ) {
  FENTER;
  rename( oldFilename, newFilename );
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "void SpindleFile::remove(const char* filename )"
void
SpindleFile::remove(const char* filename) {
  FENTER;
  remove( filename );
  FEXIT;
}
#undef __FUNC__


#define __FUNC__ "void SpindleFile::flush()"
void
SpindleFile::flush() {
FENTER;
  ASSERT( fp_ != 0, "This file is invalid");
  fflush( fp_ );
FEXIT;
}
#undef __FUNC__


#define __FUNC__ "void SpindleFile::close()"
void 
SpindleFile::close() {
  FENTER;
  if ( fp_ != 0 ) {
    FCALL flush();
    if ( closeOnDelete_ == file ) {
      fclose( fp_ );
    } else if ( closeOnDelete_ == pipe ) {
      pclose( fp_ );
    }
  }
  fp_= 0;
  FEXIT;
}
#undef __FUNC__

#define __FUNC__ "long SpindleFile::seek( long offset, size_t from )"
bool
SpindleFile::seek( long offset, SpindleFile::SeekOption from ) {
  FENTER;
  int err = -1;
  if ( fp_ != 0 ) {
    switch ( from ) {
    case FILE_START:
      err = fseek( fp_, offset, SEEK_SET );
      break;
    case FILE_CUR:
      err = fseek( fp_, offset, SEEK_CUR );
      break;
    case FILE_END:
      err = fseek( fp_, offset, SEEK_END );
      break;
    default:
      WARN_IF( true, "Unrecognized seek option %d.", from );
    }
  }
  FRETURN( err != 0 );
}
#undef __FUNC__

#define __FUNC__ "void spindleFile::rewind()"
void
SpindleFile::rewind() {
  FENTER;
  if ( fp_ != 0 ) {
    ::rewind( fp_ );
  }
  FEXIT;
}
#undef __FUNC__

bool 
SpindleFile::isEOF() { 
  return ( feof(fp_) != 0 );
}

bool 
SpindleFile::isError() {
  return ( ferror(fp_) != 0);
}

#define __FUNC__ "size_t SpindleFile::read( void* buffer, size_t nBytes, size_t nObjs  )"
size_t 
SpindleFile::read( void* buffer, size_t nBytes, size_t nObjs  ) {
  FENTER;
  ASSERT( fp_ != NULL, "This file is invalid." );  
  FRETURN( fread( buffer, nBytes, nObjs, fp_ ) );
}
#undef __FUNC__

#define __FUNC__ "size_t SpindleFile::write( const void* buffer, size_t nBytes, size_t nObjs  )"
size_t 
SpindleFile::write( const void* buffer, size_t nBytes, size_t nObjs  ) {
  FENTER;
  ASSERT( fp_ != NULL, "This file is invalid." );
  FRETURN( fwrite( buffer, nBytes, nObjs, fp_ ) );
}
#undef __FUNC__


