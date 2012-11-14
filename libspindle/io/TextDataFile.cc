//
// TextDataFile.cc
//
//

#include "spindle/TextDataFile.h"


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

#ifdef BZIP2
#define DO_BZIP2       BZIP2 " -9 - -c > %s"
#endif
#ifdef BUNZIP2
#define DO_BUNZIP2      BUNZIP2 " -c %s"
#endif 


extern "C" { 
#include <errno.h>
}


#ifdef HAVE_NAMESPACES
using namespace SPINDLE_NAMESPACE;
#endif

#ifdef _MSC_VER
#  define popen _popen
#  define pclose _pclose
#endif 

TextDataFile::~TextDataFile() {
  close();
}

bool
TextDataFile::close() {
  if ( fp != 0 ) {
    if ( filetype == file ){
      fclose(fp);
      fp = 0;
      return true;
    } else {
      pclose(fp);
      fp = 0;
      return true;
    }
  }
  return false;
}

bool 
TextDataFile::open( const char* string, const char* mode ) {
  close();
  m_isFormatted = true;
  for ( const char * pchar = mode; *pchar != '\0'; ++pchar ) { 
    if ( *pchar == 'b' ) { 
      m_isFormatted = false;
      m_isBinary = true;
    }
  }
  if ( mode[0] == 'r' ) { 
    return openRead( string, mode );
  } else if ( mode[0]=='w' || mode[0]=='a' ) {
    return openWrite( string, mode );
  } else { 
    return false;
  }
}


bool 
TextDataFile::openRead( const char* string, const char* mode ) {
  char buffer[128];

  int len = strlen(string);  // len does not include null char
 
  // check for pipes
  if (strchr(string,'|') != NULL) {
    strcpy(buffer, string);  // make a local copy
    while( (buffer[len-1] == ' ') || (buffer[len-1] == '|')) { // remove trailing pipe and spaces.
      len--;
      buffer[len] = '\0';
    }
    if ((fp = popen( buffer, mode )) == NULL) { 
      cerr << "Error: Cannot open pipe \'" << string << "\' for reading." <<  endl;
      cerr << "       mode = \"" << mode << "\"" << endl;
      checkERRNO();
      fp = 0;
      return false;
    }
    filetype = pipe;
  }
  else if ( (string[len-2] == '.') && (string[len-1]=='Z') ) {
#ifdef DO_UNCOMPRESS 
    // open uncompress pipe
    sprintf( buffer, DO_UNCOMPRESS, string);
    if ((fp = popen( buffer, mode )) == NULL) { 
      cerr << "Error: Cannot open pipe \'" << buffer << "\' for reading." <<  endl;
      cerr << "       mode = \"" << mode << "\"" << endl;
      checkERRNO();
      fp = 0;
      return false;
    }
    filetype = pipe;
#else
    cerr << "Error: \'compress\' not found at build time." << endl;
    cerr << "       Cannot read a \'.Z\' file now." << endl;
    checkERRNO();
    fp = 0;
    return false;
#endif
  }  else if ( (string[len-3] == '.') && (string[len-2]=='g') && (string[len-1]=='z') ) {
#ifdef DO_GUNZIP 
    // open gunzip pipe
    sprintf( buffer, DO_GUNZIP, string);
    if ((fp = popen( buffer, mode )) == NULL) { 
      cerr << "Error: Cannot open pipe \'" << buffer << "\' for reading." <<  endl;
      cerr << "       mode = \"" << mode << "\"" << endl;
      checkERRNO();
      fp = 0;
      return false;
    }
    filetype = pipe;
#else
    cerr << "Error: \'gunzip\' not found at build time." << endl;
    cerr << "       Cannot read a \'.gz\' file now." << endl;
    checkERRNO();
    fp = 0;
    return false;
#endif
  } else if ( (string[len-4] == '.') && (string[len-3]=='z') &&  (string[len-2]=='i') && (string[len-1]=='p') ) {
#ifdef DO_PKUNZIP
    // open zip pipe
    sprintf( buffer, DO_PKUNZIP, string);
    if ((fp = popen( buffer, mode )) == NULL) { 
      cerr << "Error: Cannot open pipe \'" << buffer << "\' for reading." <<  endl;
      cerr << "       mode = \"" << mode << "\"" << endl;
      checkERRNO();
      fp = 0;
      return false;
    }
#else
    cerr << "Error: \'pkzip\' not found at build time." << endl;
    cerr << "       Cannot read a \'.zip\' file now." << endl;
    checkERRNO();
    fp = 0;
    return false;
#endif 
  } else if ( (string[len-4] == '.') && (string[len-3]=='b') &&  (string[len-2]=='z') && (string[len-1]=='2') ) {
#ifdef DO_BUNZIP2
    // open bzip2 pipe
    sprintf( buffer, DO_BUNZIP2, string);
    if ((fp = popen( buffer, mode )) == NULL) { 
      cerr << "Error: Cannot open pipe \'" << buffer << "\' for reading." <<  endl;
      cerr << "       mode = \"" << mode << "\"" << endl;
      checkERRNO();
      fp = 0;
      return false;
    }
#else
    cerr << "Error: \'bunzip2\' not found at build time." << endl;
    cerr << "       Cannot read a \'.bz2\' file now." << endl;
    checkERRNO();
    fp = 0;
    return false;
#endif 
  } else {
    // open regular file
    if ((fp = fopen( string, mode )) == NULL) { 
      cerr << "Error: Cannot open file \'" << string << "\' for reading." << endl;
      cerr << "       mode = \"" << mode << "\"" << endl;
      checkERRNO();
      fp = 0;
      return false;
    }
    filetype = file;
  }  
  return true;
}
 
bool 
TextDataFile::openWrite( const char * string, const char * mode ) {
  char buffer[128];
  
  int len = strlen(string);  // len does not include null char
  
  // check for pipes
  if (strchr(string,'|') != NULL) {
    const char * temp = string;
    while (( *temp == ' ') || (*temp  == '|')) { // skip leading pipe and spaces.
      temp++;
    }
    if ((fp = popen( temp, mode )) == NULL) { 
      cerr << "Error: Cannot open pipe \'" << string << "\' for writing." <<  endl;
      cerr << "       mode = \"" << mode << "\"" << endl;
      checkERRNO();
      return false;
    }
    filetype = pipe;
  } else if ( (string[len-2] == '.') && (string[len-1]=='Z') ) {
#ifdef DO_COMPRESS
    // open uncompress pipe
    sprintf( buffer, DO_COMPRESS, string);
    if ((fp = popen( buffer, mode )) == NULL) { 
      cerr << "Error: Cannot open pipe \'" << buffer << "\' for writing." <<  endl;
      cerr << "       mode = \"" << mode << "\"" << endl;
      checkERRNO();
      return false;
    }
    filetype = pipe;
#else
     cerr << "Error: \'compress\' not found at build time." << endl;
    cerr << "       Cannot write a \'.Z\' file now." << endl;
    checkERRNO();
    return false;
#endif
  } else if ( (string[len-3] == '.') && (string[len-2]=='g') && (string[len-1]=='z') ) {
#ifdef DO_GZIP
    // open gunzip pipe
    sprintf( buffer, DO_GZIP, string);
    if ((fp = popen( buffer, mode )) == NULL) { 
      cerr << "Error: Cannot open pipe \'" << buffer << "\' for writing." <<  endl;
      cerr << "       mode = \"" << mode << "\"" << endl;
      checkERRNO();
      return false;
    }
    filetype = pipe;
#else
    cerr << "Error: \'gunzip\' not found at build time." << endl;
    cerr << "       Cannot write a \'.gz\' file now." << endl;
    checkERRNO();
    return false;
#endif
  } else if ( (string[len-4] == '.') && (string[len-3]=='z') &&  (string[len-2]=='i') && (string[len-1]=='p') ) {
#ifdef DO_PKZIP
    // open zip pipe
    // sprintf( buffer, ZIP, string);
    // if ((fp = popen( buffer, mode )) == NULL) { 
    //    cerr << "Error: Cannot open pipe \'" << buffer << "\'" <<  endl;
    //  return false;
    // }
    cerr << "Error: Cannot open pipe \'" << string << "\' for writing." <<  endl;
    cerr << "       mode = \"" << mode << "\"" << endl;
    cerr << "       PKZIP can't create zipfiles from a pipe!" << endl;
    return false;
#endif
  } else if ( (string[len-4] == '.') && (string[len-3]=='b') && (string[len-2]=='z') && (string[len-1]=='2') ) {
#ifdef DO_BZIP2
    // open bzip2 pipe
    sprintf( buffer, DO_BZIP2, string);
    if ((fp = popen( buffer, mode )) == NULL) { 
      cerr << "Error: Cannot open pipe \'" << buffer << "\' for writing." <<  endl;
      cerr << "       mode = \"" << mode << "\"" << endl;
      checkERRNO();
      return false;
    }
    filetype = pipe;
#else
    cerr << "Error: \'bzip2\' not found at build time." << endl;
    cerr << "       Cannot write a \'.bz2\' file now." << endl;
    checkERRNO();
    return false;
#endif
  } else {
    // open regular file
    if ((fp = fopen( string, mode )) == NULL) { 
      cerr << "Error: Cannot open file \'" << string << "\' for writing." << endl;
      cerr << "       mode = \"" << mode << "\"" << endl;
      checkERRNO();
      return false;
    }
    filetype = file;
  }  
  return true;
}

 
void TextDataFile::checkERRNO() {
  switch ( errno ) { 
  case EINVAL:
    cerr << "        (errno==EINVAL) implies mode argument is invalid." << endl;
    break;
  case EAGAIN:
    cerr << "        (errno==EAGAIN) implies fork could not alloc enough" << endl 
	 << "                        memory to copy parent's page tables." << endl;
    break;
  case ENOMEM:
    cerr << "        (errno==ENOMEM) implies fork failed to allocate necessary" << endl
	 << "                        kernel structures." << endl;
    break;
  case EMFILE:
    cerr << "        (errno==EMFILE) implies too many file descriptors are in use by process." << endl;
    break;
  case ENFILE:
    cerr << "        (errno==ENFILE) implies the system file table is full." << endl;
    break;
  case EFAULT:
    cerr << "        (errno==EFAULT) implies the filedes is not valid." << endl;
    break;
  case 0:
    break;
  default:
    cerr << "        (errno==" << errno << ") ??? " << endl;
    break;
  }
}
