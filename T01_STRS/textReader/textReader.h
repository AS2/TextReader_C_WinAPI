#ifndef __text_reader_h__
#define __text_reader_h__

#include <stdio.h>
#include <stdlib.h>

// 'text view' struct
typedef struct textReader textReader_t;
struct textReader {
  char* textBuf;                  // file chars in string
  unsigned int charsReaden;       // string length
};

// text view initialization function
// ARGS: textReader_t *tr - text view to init
//       const char *filename - file name to open
// RETURNS: 0 - fail, 1 - success
int TR_InitText(textReader_t* tr, const char* filename);

// text view destroy
// ARGS: textReader_t *tr - text view to destroy
// RETURNS: none.
void TR_ClearText(textReader_t* tr);

#endif // __text_reader_h__

