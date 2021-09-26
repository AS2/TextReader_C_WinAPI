#ifndef __win_drawer_h__
#define __win_drawer_h__

#include <windows.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>

#include "../textReader/textReader.h"

#define FONT_SIZE_DEF 18

// 'one line file' struct
typedef struct line line_t;
struct line {
  char* lineBegin;                      // line begin pointer
  unsigned int lineLength;              // line length (in symbols cnt)
  unsigned int linesInWindowSize;       // size of line (in Window Lines)
};

// 'model view' struct
typedef struct winDrawer winDrawer_t;
struct winDrawer {
  unsigned int oldW, oldH, newW, newH;  // old and new window sizes
  unsigned int linesInWindow;           // count of lines, which Window can fit
  unsigned int symbolsPerWindowLine;    // max line in Window length

  line_t* lines;                        // lines pointer (after init: massive)
  unsigned int linesCnt,                // lines count
    totalLinesInWin;         // total different lines count

  unsigned int lineStart,               // number of line, where placed started subline
    sublineStart;            // number of started subline in line
};

// model view initialization function
// ARGS: winDrawer_t *wd - model view to init
//       int winW, winH - window sizes
//       HWND hwnd - window
// RETURNS: 0 - fail, 1 - success
int WD_Init(winDrawer_t* wd, textReader_t tr, int winW, int winH, HWND hwnd);

void WD_UpdateSizes(winDrawer_t* wd, int newW, int newH);

int WD_IsNeedToReparse(winDrawer_t wd);
int WD_ReparseText(textReader_t tr, winDrawer_t* wd);

void WD_ShiftTextPosition(winDrawer_t* wd, unsigned int linesToShift, int shiftType);

void WD_DrawText(HWND hwnd, winDrawer_t wd);

void WD_Destroy(winDrawer_t* wd);

#endif // __win_drawer_h__
