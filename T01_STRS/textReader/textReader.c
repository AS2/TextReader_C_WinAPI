#include "textReader.h"

#pragma warning(disable: 4996)

// text view initialization function
// ARGS: textReader_t *tr - text view to init
//       const char *filename - file name to open
// RETURNS: 0 - fail, 1 - success
int TR_InitText(textReader_t* tr, const char* filename) {
  FILE *F;

  if ((F = fopen(filename, "rt")) == NULL)
    return 0;

  fseek(F, 0, SEEK_END);
  int bufLen = ftell(F);
  fseek(F, 0, SEEK_SET);

  if ((tr->textBuf = (char *)malloc(sizeof(char) * bufLen + 1)) == NULL)
    return 0;

  tr->charsReaden = fread(tr->textBuf, 1, bufLen, F);
  tr->textBuf[tr->charsReaden] = 0;

  fclose(F);
  return 1;
}

// text view destroy
// ARGS: textReader_t *tr - text view to destroy
// RETURNS: none.
void TR_ClearText(textReader_t* tr) {
    if (tr->textBuf != NULL)
        free(tr->textBuf);
    tr->charsReaden = 0;
}
