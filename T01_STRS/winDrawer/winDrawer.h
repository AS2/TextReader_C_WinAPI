#ifndef __win_drawer_h__
#define __win_drawer_h__

#include <windows.h>
#include <tchar.h>

#include <stdlib.h>

#include "../textReader/textReader.h"

#define FONT_SIZE_DEF 18                // font height

// 'one line file' struct
typedef struct line line_t;
struct line {
  char* lineBegin;                      // line begin pointer
  unsigned int lineLength;              // line length (in symbols cnt)
  unsigned int linesInWindowSize;       // size of line (in Window Lines)
};

// model view types
enum modelViewTypes {
  MV_FORMATED = 0,                      // has only vertical scroll
  MV_ORIGINAL = 1                       // has vertical and horizontal scrolls
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

  // SCROLL params (client area position)
  char modelViewType;                   // model view type (watch 'modelViewTypes')
  unsigned int lineStart;               // number of line, where placed started subline
  // 'MV_FORMATED' param
  unsigned int sublineStart;            // number of started subline in line
  unsigned int yScrollCoord;            // vertical scroll coord (after reparse)
  // 'MV_ORIGINAL' params
  unsigned int maxLineLength,           // maximum of lines length
    xScrollCoord;            // horizontal scroll coord (after reparse)
};


// model view initialization function
// ARGS: winDrawer_t *wd - model view to init
//       int winW, winH - window sizes
//       HWND hwnd - window
// RETURNS: 0 - fail, 1 - success
int WD_Init(winDrawer_t* wd, textReader_t tr, int winW, int winH, HWND hwnd);


// switch model view type
// ARGS: winDrawer_t *wd - model view to switch type
//       HWND hwnd - window
// RETURNS: none
void WD_SwitchType(winDrawer_t* wd, HWND hwnd);


// model view update sizes after resize
// ARGS: winDrawer_t *wd - model view to update
//       int newW, newH - new window sizes
// RETURNS: none
void WD_UpdateSizes(winDrawer_t* wd, int newW, int newH);


// is text need to be reparsed check function
// ARGS: winDrawer_t wd - model view to check
// RETURNS: 0 - not need, 1 - need to reparse
int WD_IsNeedToReparse(winDrawer_t wd);


// reparse text function
// ARGS: winDrawer_t *wd - model view to reparse
// RETURNS: none.
void WD_ReparseText(winDrawer_t* wd);


// shift vertical text position
// ARGS: winDrawer_t *wd - model view with text to shift
//       unsigned int linesToShift - line to shift
//       int shiftType - '-1' - shift up, '1' - shift down
// RETURNS: none.
void WD_ShiftTextPosition(winDrawer_t* wd, unsigned int linesToShift, int shiftType);


// shift horizontal text position
// ARGS: winDrawer_t *wd - model view with text to shift
//       unsigned int charsToShift - chars to shift
//       int shiftType - '-1' - shift right, '1' - shift left
// RETURNS: none.
void WD_ShiftLineStart(winDrawer_t* wd, unsigned int charsToShift, int shiftType);


// draw text function
// ARGS: winDrawer_t wd - model view to draw
//       HWND hwnd - window
// RETURNS: none.
void WD_DrawText(HWND hwnd, winDrawer_t wd);


// model view destroy
// ARGS: winDrawer_t *wd - model view to destroy
// RETURNS: none.
void WD_Destroy(winDrawer_t* wd);

#endif // __win_drawer_h__
