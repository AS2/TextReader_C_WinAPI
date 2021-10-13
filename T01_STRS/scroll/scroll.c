#include "scroll.h"

// replace scrolls positions function
// ARGS: winDrawer_t wd - model view to use
//       HWND hwnd - window with scrolls
// RETURNS: none.
void SC_ReplaceScrolls(HWND hwnd, winDrawer_t wd) {
  static SCROLLINFO si;
  static int yLength, yPos;

  if (wd.modelViewType == MV_FORMATED) {
    yLength = wd.totalLinesInWin - 1;
    yPos = wd.yScrollCoord;
    ShowScrollBar(hwnd, SB_HORZ, FALSE);
  }
  else {
    yLength = wd.linesCnt - 1;
    yPos = wd.lineStart;
    ShowScrollBar(hwnd, SB_HORZ, TRUE);
  }

  // fill vert scroll info
  si.cbSize = sizeof(si);
  si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
  si.nMin = 0;
  si.nMax = yLength;
  si.nPos = yPos;
  si.nPage = wd.linesInWindow;
  SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

  if (wd.modelViewType == MV_FORMATED)
    ShowScrollBar(hwnd, SB_HORZ, FALSE);
  else {
    // fill horiz scroll info
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = 0;
    si.nPos = wd.xScrollCoord;
    si.nMax = wd.maxLineLength - 1;
    si.nPage = wd.symbolsPerWindowLine;
    SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
  }
}
