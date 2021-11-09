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
    for (i = 0; lineIndex < wd->linesCnt; i++) {
      if (tr.textBuf[i] == '\n' || tr.textBuf[i] == 0) {
        wd->lines[lineIndex++] = &(tr.textBuf[lastBeginIndex]);

        if (wd->maxLineLength < i - lastBeginIndex)
          wd->maxLineLength = i - lastBeginIndex;


        if (lineIndex == wd->linesCnt)
          wd->lastLineLength = i - lastBeginIndex > 0 ? i - lastBeginIndex : 0;

        lastBeginIndex = i + 1;
      }

      if (tr.textBuf[i] == 0)
        break;
    }
  }

  wd->sublineStart = wd->lineStart =
    wd->xScrollCoord = wd->yScrollCoord = 0;

  // set MV_FORMATED model view type as default
  wd->modelViewType = MV_FORMATED;

  return 1;
}

// count line length
// ARGS: winDrawer_t wd - model view to get line
//       unsigned int ind - line index
// RETURNS: unsigned int - line length
static unsigned int WD_GetLineLength(winDrawer_t wd, unsigned int ind) {
    if (ind < wd.linesCnt)
      return ind + 1 < wd.linesCnt ? wd.lines[ind + 1] - wd.lines[ind] - 1 : wd.lastLineLength;
    return 0;
}

// count line substrings
// ARGS: winDrawer_t wd - model view to get line
//       unsigned int ind - line index
// RETURNS: unsigned int - line substrings count
static unsigned int WD_GetSublineCnt(winDrawer_t wd, unsigned int ind) {
    if (ind < wd.linesCnt) {
      unsigned int currentLineLength = WD_GetLineLength(wd, ind);
      if (currentLineLength == 0)
          return 1;

      unsigned int sublinesCnt = currentLineLength / wd.symbolsPerWindowLine + (currentLineLength % wd.symbolsPerWindowLine > 0 ? 1 : 0);;
      return sublinesCnt;
    }
    return 0;
}

// switch model view type
// ARGS: winDrawer_t *wd - model view to switch type
//       HWND hwnd - window
// RETURNS: none
void WD_SwitchType(winDrawer_t* wd, HWND hwnd) {
  if (wd->modelViewType == MV_FORMATED) {
    wd->modelViewType = MV_ORIGINAL;
    wd->xScrollCoord = 0;

    ShowScrollBar(hwnd, SB_HORZ, TRUE);
  }
  else {
    wd->modelViewType = MV_FORMATED;
    wd->sublineStart = 0;

    ShowScrollBar(hwnd, SB_HORZ, FALSE);
    //GetWindowRect(hwnd, &rect);
    //WD_UpdateSizes(wd, rect.right - rect.left, rect.bottom - rect.top);
    //ShowScrollBar(hwnd, SB_HORZ, TRUE);
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
  unsigned int i = 0, linesInWindowSize = WD_GetSublineCnt(*wd, wd->lineStart);

  if (shiftType == -1) {
    for (i = 0; i < linesToShift; i++) {
      if (wd->lineStart == 0 && wd->sublineStart == 0)
        return;
      else if (wd->sublineStart == 0) {
        wd->lineStart--;
        linesInWindowSize = WD_GetSublineCnt(*wd, wd->lineStart);
        wd->sublineStart = linesInWindowSize - 1;
      }
      else
        wd->sublineStart--;
      wd->yScrollCoord--;
    }
  }
  else if (shiftType == 1) {
      for (i = 0; i < linesToShift; i++) {
        if (wd->lineStart == wd->linesCnt - 1 && linesInWindowSize - 1 == wd->sublineStart)
          return;
        else if (linesInWindowSize - 1 == wd->sublineStart) {
          wd->lineStart++;
          linesInWindowSize = WD_GetSublineCnt(*wd, wd->lineStart);
          wd->sublineStart = 0;
        }
        else
            wd->sublineStart++;
        wd->yScrollCoord++;
    }
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
  for (i = 0; i < wd->linesCnt; i++)
    wd->totalLinesInWin += WD_GetSublineCnt(*wd, i);

  // reparse started lines
  if (WD_GetSublineCnt(*wd, wd->lineStart) - 1 < wd->sublineStart)
    wd->sublineStart = WD_GetSublineCnt(*wd, wd->lineStart) - 1;

  // count new scroll position
  wd->yScrollCoord = 0;
  for (i = 0; i < wd->lineStart; i++)
    wd->yScrollCoord += WD_GetSublineCnt(*wd, i);
  wd->yScrollCoord += wd->sublineStart;

  // shift strings down to remove empty space, which created at the end of file
  if (wd->totalLinesInWin - wd->yScrollCoord < wd->linesInWindow && wd->totalLinesInWin > wd->linesInWindow) {
    WD_ShiftTextPosition_Formated(wd, wd->linesInWindow - (wd->totalLinesInWin - wd->yScrollCoord), -1);
    wd->yScrollCoord = MAX(wd->yScrollCoord - (wd->linesInWindow - (wd->totalLinesInWin - wd->yScrollCoord)), 0);
  }

  // update scrolls values
  if (wd->totalLinesInWin - wd->linesInWindow > MAX_SCROLL_RANGE)
      wd->yScrollScale = (float)(MAX_SCROLL_RANGE) / (float)(wd->totalLinesInWin - wd->linesInWindow);
  else
      wd->yScrollScale = 1;
}

// reparse text in 'MV_ORIGINAL' mode static function
// ARGS: winDrawer_t *wd - model view to reparse
// RETURNS: none.
static void WD_ReparseText_Original(winDrawer_t* wd) {
  wd->symbolsPerWindowLine = wd->newW / (int)(FONT_SIZE_DEF / 2);
  wd->linesInWindow = wd->newH / (FONT_SIZE_DEF);

  // update scrolls values
  if (wd->linesCnt - wd->linesInWindow > MAX_SCROLL_RANGE)
      wd->yScrollScale = ((float)MAX_SCROLL_RANGE) / ((float)(wd->linesCnt - wd->linesInWindow));
  else
      wd->yScrollScale = 1;

  if (wd->maxLineLength - wd->symbolsPerWindowLine > MAX_SCROLL_RANGE)
      wd->xScrollScale = ((float)MAX_SCROLL_RANGE) / ((float)(wd->maxLineLength - wd->symbolsPerWindowLine));
  else
      wd->xScrollScale = 1;

  // shift strings down to remove empty space, which created at the end of file
  if (wd->linesCnt - wd->lineStart < wd->linesInWindow && wd->linesCnt > wd->linesInWindow) {
    WD_ShiftTextPosition_Original(wd, wd->linesInWindow - (wd->linesCnt - wd->lineStart), -1);
    wd->lineStart = MAX(0, wd->lineStart - (wd->linesInWindow - (wd->linesCnt - wd->lineStart)));
  }
  else if (wd->linesCnt <= wd->linesInWindow) {
    WD_ShiftTextPosition_Original(wd, wd->linesCnt, -1);
    wd->lineStart = 0;
  }

  // shift strings right to remove empty space, which created at the right of client area
  if (wd->maxLineLength - wd->xScrollCoord < wd->symbolsPerWindowLine && wd->maxLineLength > wd->symbolsPerWindowLine) {
    WD_ShiftLineStart(wd, wd->symbolsPerWindowLine - (wd->maxLineLength - wd->xScrollCoord), -1);
    wd->xScrollCoord = MAX(0, wd->xScrollCoord - (wd->symbolsPerWindowLine - (wd->maxLineLength - wd->xScrollCoord)));
  }
  else if (wd->maxLineLength <= wd->symbolsPerWindowLine && wd->xScrollCoord > 0) {
    WD_ShiftLineStart(wd, wd->xScrollCoord, -1);
    wd->xScrollCoord = 0;
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

// draw text in 'MV_FORMATED' mode static function
// ARGS: winDrawer_t wd - model view to draw
//       HWND hwnd - window
// RETURNS: none.
static void WD_DrawText_Formated(HWND hwnd, winDrawer_t wd) {
  if (wd.linesInWindow <= 0)
    return;

  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(hwnd, &ps);

  unsigned int x, y, linesDrawed = 0, currentLineLength,
      linesInStartLine = WD_GetSublineCnt(wd, wd.lineStart),
      linesInCurLine;

  // write first sublines
  for (x = wd.sublineStart; x < linesInStartLine; x++) {
    currentLineLength = WD_GetLineLength(wd, wd.lineStart);

    TextOut(hdc, 0, linesDrawed++ * FONT_SIZE_DEF, wd.lines[wd.lineStart] + wd.symbolsPerWindowLine * x,
      MIN(currentLineLength - wd.symbolsPerWindowLine * x, wd.symbolsPerWindowLine));

    if (linesDrawed == wd.linesInWindow)
      break;
  }

  if (linesDrawed == wd.linesInWindow) {
    EndPaint(hwnd, &ps);
    return;
  }

  // write remained sublines
  for (y = wd.lineStart + 1; y < wd.linesCnt; y++) {
    currentLineLength = WD_GetLineLength(wd, y);
    linesInCurLine = WD_GetSublineCnt(wd, y);

    for (x = 0; x < linesInCurLine; x++) {
      TextOut(hdc, 0, linesDrawed++ * FONT_SIZE_DEF, wd.lines[y] + wd.symbolsPerWindowLine * x,
        MIN(currentLineLength - wd.symbolsPerWindowLine * x, wd.symbolsPerWindowLine));

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

  unsigned int y, linesDrawed = 0, currentLineLength;

  // write lines
  for (y = wd.lineStart; y < wd.linesCnt; y++) {
    currentLineLength = WD_GetLineLength(wd, y);

    if (currentLineLength > wd.xScrollCoord)
      TextOut(hdc, 0, linesDrawed++ * FONT_SIZE_DEF, wd.lines[y] + wd.xScrollCoord,
        MIN(currentLineLength - wd.xScrollCoord, wd.symbolsPerWindowLine));
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

  wd->linesCnt = wd->totalLinesInWin = 0;
}
