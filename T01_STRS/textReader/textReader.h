#ifndef __text_reader_h__
#define __text_reader_h__
#include <stdio.h>
#include <stdlib.h>

typedef struct textReader textReader_t;
struct textReader {
    char *textBuf;
    unsigned int charsReaden;
};

int TR_InitText(textReader_t* tr, const char* filename);
void TR_ClearText(textReader_t* tr);

#endif // __text_reader_h__
