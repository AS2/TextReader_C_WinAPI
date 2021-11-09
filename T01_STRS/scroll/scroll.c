#include "scroll.h"

#define MAX(A, B) A > B ? A : B   // get max preprocessor func

// replace scrolls positions function
// ARGS: winDrawer_t wd - model view to use
//       HWND hwnd - window with scrolls
// RETURNS: none.
void SC_ReplaceScrolls(HWND hwnd, winDrawer_t wd) {
  int yLength, yPos;
  int yRange, xRange, yPageSize, xPageSize;

  if (wd.modelViewType == MV_FORMATED) {
    yLength = wd.totalLinesInWin;
    yPos = wd.yScrollCoord;
    ShowScrollBar(hwnd, SB_HORZ, FALSE);
  }
  else {
    yLength = wd.linesCnt;
    yPos = wd.lineStart;
  }

  // fill vert scroll info

  if (yLength > wd.linesInWindow) {
    yPageSize = wd.linesInWindow * wd.yScrollScale;
    if (yLength - wd.linesInWindow > MAX_SCROLL_RANGE)
        yRange = MAX_SCROLL_RANGE;
    else
        yRange = yLength - wd.linesInWindow;

    SetScrollRange(hwnd, SB_VERT, 0, yRange, TRUE);
    SetScrollPos(hwnd, SB_VERT, (int)(yPos * wd.yScrollScale), TRUE);
    ShowScrollBar(hwnd, SB_VERT, TRUE);
  }
  else
    ShowScrollBar(hwnd, SB_VERT, FALSE);


  if (wd.modelViewType == MV_FORMATED)
    ShowScrollBar(hwnd, SB_HORZ, FALSE);
  else {
    // fill vert scroll info
    if (wd.maxLineLength > wd.symbolsPerWindowLine) {
        xPageSize = wd.symbolsPerWindowLine * wd.xScrollScale;
        if (wd.maxLineLength - wd.symbolsPerWindowLine > MAX_SCROLL_RANGE)
            xRange = MAX_SCROLL_RANGE;
        else
            xRange = wd.maxLineLength - wd.symbolsPerWindowLine;

        SetScrollRange(hwnd, SB_HORZ, 0, xRange, TRUE);
        SetScrollPos(hwnd, SB_HORZ, (int)(wd.xScrollCoord * wd.xScrollScale), TRUE);
        ShowScrollBar(hwnd, SB_HORZ, TRUE);
    }
    else
        ShowScrollBar(hwnd, SB_HORZ, FALSE);
  }
}
