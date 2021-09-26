#include "winDrawer.h"

#define MIN(A, B) A > B ? B : A

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
  if (tr.charsReaden == 0) {
    wd->linesCnt = 0;
    wd->lines = NULL;
  }
  else {
    wd->linesCnt = 0;
    for (i = 0; tr.textBuf[i] != 0; i++) {
      if (wasCharacter == 0 && tr.textBuf[i] != '\n') {
        wd->linesCnt += linesInReserv;
        linesInReserv = 0;
        wasCharacter = 1;
      }
      else if (wasCharacter == 0 && tr.textBuf[i] == '\n') {
        linesInReserv++;
      }
      else if (wasCharacter == 1 && tr.textBuf[i] == '\n') {
        wasCharacter = 0;
        wd->linesCnt++;
      }
    }
    if (wasCharacter == 1)
      wd->linesCnt++;

    if ((wd->lines = (line_t*)malloc(sizeof(line_t) * wd->linesCnt)) == NULL)
      return 0;

    for (i = 0; tr.textBuf[i] != 0 && lineIndex < wd->linesCnt; i++)
      if (tr.textBuf[i] == '\n') {
        wd->lines[lineIndex].lineBegin = &(tr.textBuf[lastBeginIndex]);
        wd->lines[lineIndex++].lineLength = i - lastBeginIndex;

        if (tr.textBuf[i + 1] != 0)
          lastBeginIndex = i + 1;
      }
  }

  wd->sublineStart = wd->lineStart = 0;

  return 1;
}

void WD_UpdateSizes(winDrawer_t* wd, int newW, int newH) {
  if (newW <= 0 || newH <= 0)
    return;

  wd->oldW = wd->newW;
  wd->oldH = wd->newH;
  wd->newW = newW;
  wd->newH = newH;
}

int WD_IsNeedToReparse(winDrawer_t wd) {
  int oSymbPerH = wd.oldH / (FONT_SIZE_DEF), oSymbPerW = wd.oldW / (int)(FONT_SIZE_DEF / 2),
    nSymbPerH = wd.newH / (FONT_SIZE_DEF), nSymbPerW = wd.newW / (int)(FONT_SIZE_DEF / 2);

  if (oSymbPerH != nSymbPerH || oSymbPerW != nSymbPerW)
    return 1;
  return 0;
}

void WD_ShiftTextPosition(winDrawer_t* wd, unsigned int linesToShift, int shiftType) {
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

int WD_ReparseText(textReader_t tr, winDrawer_t* wd) {
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

  int yCoordToReturn = 0;
  for (i = 0; i < wd->lineStart; i++)
    yCoordToReturn += wd->lines[i].linesInWindowSize;
  yCoordToReturn += wd->sublineStart;

  return yCoordToReturn;
}

void WD_DrawText(HWND hwnd, winDrawer_t wd) {
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

void WD_Destroy(winDrawer_t* wd) {
  if (wd->lines != NULL)
    free(wd->lines);
}
