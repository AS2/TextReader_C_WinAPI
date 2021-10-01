#include "winDrawer.h"

#define MIN(A, B) A > B ? B : A   // get min preprocessor func
#define MAX(A, B) A > B ? A : B   // get max preprocessor func

// model view initialization function
// ARGS: winDrawer_t *wd - model view to init
//       int winW, winH - window sizes
//       HWND hwnd - window
// RETURNS: 0 - fail, 1 - success
int WD_Init(winDrawer_t* wd, textReader_t tr, int winW, int winH, HWND hwnd) {
  int i, lastBeginIndex = 0, lineIndex = 0, linesInReserv = 0, wasCharacter = 0;

  if (winW < 0 || winH < 0)
    return 0;

  wd->oldW = wd->newW = winW;
  wd->oldH = wd->newH = winH;

  // CREATE FONT AND SET IT
  HDC hdc = GetDC(hwnd);
  HFONT textFont = CreateFont(FONT_SIZE_DEF, (int)(FONT_SIZE_DEF / 2), 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, RUSSIAN_CHARSET, OUT_DEFAULT_PRECIS, OUT_OUTLINE_PRECIS, PROOF_QUALITY, FIXED_PITCH | FF_MODERN, (LPCWSTR)"Sans");
  SelectObject(hdc, textFont);
  SetTextColor(hdc, RGB(0, 0, 0));
  SetBkMode(hdc, TRANSPARENT);

  // PARSE LINES
  wd->maxLineLength = 0;
  if (tr.charsReaden == 0) {
    wd->linesCnt = 0;
    wd->lines = NULL;
  }
  else {
    // count different lines in file
    wd->linesCnt = 0;
    for (i = 0; tr.textBuf[i] != 0; i++) {
      // if saw character - add lines count from reserve
      if (wasCharacter == 0 && tr.textBuf[i] != '\n') {
        wd->linesCnt += linesInReserv;
        linesInReserv = 0;
        wasCharacter = 1;
      }
      // if saw line shift without any symbols before - increase lines in reserve to 1
      else if (wasCharacter == 0 && tr.textBuf[i] == '\n') {
        linesInReserv++;
      }
      // if saw line shift without some symbols before - increase lines to 1
      else if (wasCharacter == 1 && tr.textBuf[i] == '\n') {
        wasCharacter = 0;
        wd->linesCnt++;
      }
    }
    // increase lines cnt if last symbol was character
    if (wasCharacter == 1)
      wd->linesCnt++;

    if ((wd->lines = (line_t*)malloc(sizeof(line_t) * wd->linesCnt)) == NULL)
      return 0;

    // parse lines
    for (i = 0; tr.textBuf[i] != 0 && lineIndex < wd->linesCnt; i++)
      if (tr.textBuf[i] == '\n') {
        wd->lines[lineIndex].lineBegin = &(tr.textBuf[lastBeginIndex]);
        wd->lines[lineIndex++].lineLength = i - lastBeginIndex;

        if (wd->maxLineLength < i - lastBeginIndex)
          wd->maxLineLength = i - lastBeginIndex;

        if (tr.textBuf[i + 1] != 0)
          lastBeginIndex = i + 1;
      }
  }

  wd->sublineStart = wd->lineStart =
    wd->xScrollCoord = wd->yScrollCoord = 0;

  // set MV_FORMATED model view type as default
  wd->modelViewType = MV_FORMATED;

  return 1;
}

// switch model view type
// ARGS: winDrawer_t *wd - model view to switch type
//       HWND hwnd - window
// RETURNS: none
void WD_SwitchType(winDrawer_t* wd, HWND hwnd) {
  RECT rect;

  if (wd->modelViewType == MV_FORMATED) {
    wd->modelViewType = MV_ORIGINAL;
    wd->xScrollCoord = 0;

    ShowScrollBar(hwnd, SB_HORZ, TRUE);
    GetWindowRect(hwnd, &rect);
    WD_UpdateSizes(wd, rect.right - rect.left, rect.bottom - rect.top);
    ShowScrollBar(hwnd, SB_HORZ, FALSE);
  }
  else {
    wd->modelViewType = MV_FORMATED;
    wd->sublineStart = 0;

    ShowScrollBar(hwnd, SB_HORZ, FALSE);
    GetWindowRect(hwnd, &rect);
    WD_UpdateSizes(wd, rect.right - rect.left, rect.bottom - rect.top);
    ShowScrollBar(hwnd, SB_HORZ, TRUE);
  }
}

// model view update sizes after resize
// ARGS: winDrawer_t *wd - model view to update
//       int newW, newH - new window sizes
// RETURNS: none
void WD_UpdateSizes(winDrawer_t* wd, int newW, int newH) {
  if (newW <= 0 || newH <= 0)
    return;

  wd->oldW = wd->newW;
  wd->oldH = wd->newH;
  wd->newW = newW;
  wd->newH = newH;
}

// is text need to be reparsed check function
// ARGS: winDrawer_t wd - model view to check
// RETURNS: 0 - not need, 1 - need to reparse
int WD_IsNeedToReparse(winDrawer_t wd) {
  int oSymbPerH = wd.oldH / (FONT_SIZE_DEF), oSymbPerW = wd.oldW / (int)(FONT_SIZE_DEF / 2),
    nSymbPerH = wd.newH / (FONT_SIZE_DEF), nSymbPerW = wd.newW / (int)(FONT_SIZE_DEF / 2);

  if (oSymbPerH != nSymbPerH || oSymbPerW != nSymbPerW)
    return 1;
  return 0;
}

// shift vertical text position in 'MV_FORMATED' mode static function.
// ARGS: winDrawer_t *wd - model view with text to shift
//       unsigned int linesToShift - line to shift
//       int shiftType - '-1' - shift up, '1' - shift down
// RETURNS: none.
static void WD_ShiftTextPosition_Formated(winDrawer_t* wd, unsigned int linesToShift, int shiftType) {
  int i = 0;

  if (shiftType == -1) {
    for (i = 0; i < linesToShift; i++)
      if (wd->lineStart == 0 && wd->sublineStart == 0)
        return;
      else if (wd->sublineStart == 0) {
        wd->lineStart--;
        wd->sublineStart = wd->lines[wd->lineStart].linesInWindowSize - 1;
      }
      else
        wd->sublineStart--;
  }
  else if (shiftType == 1) {
    for (i = 0; i < linesToShift; i++)
      if (wd->lineStart == wd->linesCnt - 1 && wd->lines[wd->lineStart].linesInWindowSize - 1 == wd->sublineStart)
        return;
      else if (wd->lines[wd->lineStart].linesInWindowSize - 1 == wd->sublineStart) {
        wd->lineStart++;
        wd->sublineStart = 0;
      }
      else
        wd->sublineStart++;
  }
}

// shift vertical text position in 'MV_ORIGINAL' mode static function.
// ARGS: winDrawer_t *wd - model view with text to shift
//       unsigned int linesToShift - line to shift
//       int shiftType - '-1' - shift up, '1' - shift down
// RETURNS: none.
static void WD_ShiftTextPosition_Original(winDrawer_t* wd, unsigned int linesToShift, int shiftType) {
  int i = 0;

  if (shiftType == -1) {
    for (i = 0; i < linesToShift; i++)
      if (wd->lineStart == 0)
        return;
      else
        wd->lineStart--;
  }
  else if (shiftType == 1) {
    for (i = 0; i < linesToShift; i++)
      if (wd->lineStart == wd->linesCnt - 1)
        return;
      else
        wd->lineStart++;
  }
}

// shift vertical text position
// ARGS: winDrawer_t *wd - model view with text to shift
//       unsigned int linesToShift - line to shift
//       int shiftType - '-1' - shift up, '1' - shift down
// RETURNS: none.
void WD_ShiftTextPosition(winDrawer_t* wd, unsigned int linesToShift, int shiftType) {
  if (wd->modelViewType == MV_FORMATED)
    WD_ShiftTextPosition_Formated(wd, linesToShift, shiftType);
  else
    WD_ShiftTextPosition_Original(wd, linesToShift, shiftType);
}

// shift horizontal text position
// ARGS: winDrawer_t *wd - model view with text to shift
//       unsigned int charsToShift - chars to shift
//       int shiftType - '-1' - shift right, '1' - shift left
// RETURNS: none.
void WD_ShiftLineStart(winDrawer_t* wd, unsigned int charsToShift, int shiftType) {
  if (shiftType == -1)
    wd->xScrollCoord = MAX(0, wd->xScrollCoord - charsToShift);
  else if (shiftType == 1)
    wd->xScrollCoord = MIN(wd->maxLineLength - 1, wd->xScrollCoord + charsToShift);
}

// reparse text in 'MV_FORMATED' mode static function
// ARGS: winDrawer_t *wd - model view to reparse
// RETURNS: none.
static void WD_ReparseText_Formated(winDrawer_t* wd) {
  unsigned int i;

  wd->totalLinesInWin = 0;
  wd->symbolsPerWindowLine = wd->newW / (int)(FONT_SIZE_DEF / 2);
  wd->linesInWindow = wd->newH / (FONT_SIZE_DEF);
  for (i = 0; i < wd->linesCnt; i++) {
    // if line - just '\n'
    if (wd->lines[i].lineLength == 0)
      wd->lines[i].linesInWindowSize = 1;
    // if line contains symbols
    else
      wd->lines[i].linesInWindowSize = wd->lines[i].lineLength / wd->symbolsPerWindowLine + (wd->lines[i].lineLength % wd->symbolsPerWindowLine > 0 ? 1 : 0);

    wd->totalLinesInWin += wd->lines[i].linesInWindowSize;
  }

  // reparse started lines
  if (wd->lines[wd->lineStart].linesInWindowSize - 1 < wd->sublineStart)
    wd->sublineStart = wd->lines[wd->lineStart].linesInWindowSize - 1;

  // count new scroll position
  wd->yScrollCoord = 0;
  for (i = 0; i < wd->lineStart; i++)
    wd->yScrollCoord += wd->lines[i].linesInWindowSize;
  wd->yScrollCoord += wd->sublineStart;

  // shift strings down to remove empty space, which created at the end of file
  if (wd->totalLinesInWin - wd->yScrollCoord < wd->linesInWindow) {
    WD_ShiftTextPosition_Formated(wd, wd->linesInWindow - (wd->totalLinesInWin - wd->yScrollCoord), -1);
    wd->yScrollCoord = MAX(wd->yScrollCoord - (wd->linesInWindow - (wd->totalLinesInWin - wd->yScrollCoord)), 0);
  }
}

// reparse text in 'MV_ORIGINAL' mode static function
// ARGS: winDrawer_t *wd - model view to reparse
// RETURNS: none.
static void WD_ReparseText_Original(winDrawer_t* wd) {
  unsigned int i;

  wd->symbolsPerWindowLine = wd->newW / (int)(FONT_SIZE_DEF / 2);
  wd->linesInWindow = wd->newH / (FONT_SIZE_DEF);

  // shift strings down to remove empty space, which created at the end of file
  if (wd->linesCnt - wd->lineStart < wd->linesInWindow) {
    WD_ShiftTextPosition_Original(wd, wd->linesInWindow - (wd->linesCnt - wd->lineStart), -1);
    wd->lineStart = MAX(0, wd->lineStart - (wd->linesInWindow - (wd->linesCnt - wd->lineStart)));
  }

  // shift strings right to remove empty space, which created at the right of client area
  if (wd->maxLineLength - wd->xScrollCoord < wd->symbolsPerWindowLine) {
    WD_ShiftLineStart(wd, wd->symbolsPerWindowLine - (wd->maxLineLength - wd->xScrollCoord), -1);
    wd->xScrollCoord = MAX(0, wd->xScrollCoord - (wd->symbolsPerWindowLine - (wd->maxLineLength - wd->xScrollCoord)));
  }
}

// reparse text mode function
// ARGS: winDrawer_t *wd - model view to reparse
// RETURNS: none.
void WD_ReparseText(winDrawer_t* wd) {
  if (wd->modelViewType == MV_FORMATED)
    WD_ReparseText_Formated(wd);
  else
    WD_ReparseText_Original(wd);
}

// replace scrolls positions function
// ARGS: winDrawer_t wd - model view to use
//       HWND hwnd - window with scrolls
// RETURNS: none.
void WD_ReplaceScrolls(HWND hwnd, winDrawer_t wd) {
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

// draw text in 'MV_FORMATED' mode static function
// ARGS: winDrawer_t wd - model view to draw
//       HWND hwnd - window
// RETURNS: none.
static void WD_DrawText_Formated(HWND hwnd, winDrawer_t wd) {
  if (wd.linesInWindow <= 0)
    return;

  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(hwnd, &ps);

  int x, y, linesDrawed = 0;

  // write first sublines
  for (x = wd.sublineStart; x < wd.lines[wd.lineStart].linesInWindowSize; x++) {
    TextOut(hdc, 0, linesDrawed++ * FONT_SIZE_DEF, wd.lines[wd.lineStart].lineBegin + wd.symbolsPerWindowLine * x,
      MIN(wd.lines[wd.lineStart].lineLength - wd.symbolsPerWindowLine * x, wd.symbolsPerWindowLine));

    if (linesDrawed == wd.linesInWindow)
      break;
  }

  if (linesDrawed == wd.linesInWindow) {
    EndPaint(hwnd, &ps);
    return;
  }

  // write remained sublines
  for (y = wd.lineStart + 1; y < wd.linesCnt; y++) {
    for (x = 0; x < wd.lines[y].linesInWindowSize; x++) {
      TextOut(hdc, 0, linesDrawed++ * FONT_SIZE_DEF, wd.lines[y].lineBegin + wd.symbolsPerWindowLine * x,
        MIN(wd.lines[y].lineLength - wd.symbolsPerWindowLine * x, wd.symbolsPerWindowLine));

      if (linesDrawed == wd.linesInWindow)
        break;
    }
    if (linesDrawed == wd.linesInWindow)
      break;
  }

  EndPaint(hwnd, &ps);
}

// draw text in 'MV_ORGINAL' mode static function
// ARGS: winDrawer_t wd - model view to draw
//       HWND hwnd - window
// RETURNS: none.
static void WD_DrawText_Original(HWND hwnd, winDrawer_t wd) {
  if (wd.linesInWindow <= 0)
    return;

  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(hwnd, &ps);

  int x, y, linesDrawed = 0;

  // write lines
  for (y = wd.lineStart; y < wd.linesCnt; y++) {
    if (wd.lines[y].lineLength > wd.xScrollCoord)
      TextOut(hdc, 0, linesDrawed++ * FONT_SIZE_DEF, wd.lines[y].lineBegin + wd.xScrollCoord,
        MIN(wd.lines[y].lineLength - wd.xScrollCoord, wd.symbolsPerWindowLine));
    else
      linesDrawed++;


    if (linesDrawed == wd.linesInWindow)
      break;
  }

  EndPaint(hwnd, &ps);
}

// draw text function
// ARGS: winDrawer_t wd - model view to draw
//       HWND hwnd - window
// RETURNS: none.
void WD_DrawText(HWND hwnd, winDrawer_t wd) {
  if (wd.modelViewType == MV_FORMATED)
    WD_DrawText_Formated(hwnd, wd);
  else
    WD_DrawText_Original(hwnd, wd);
}

// model view destroy
// ARGS: winDrawer_t *wd - model view to destroy
// RETURNS: none.
void WD_Destroy(winDrawer_t* wd) {
  if (wd->lines != NULL)
    free(wd->lines);
}
