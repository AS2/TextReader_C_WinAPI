#include "winDrawer.h"

int WD_Init(winDrawer_t* wd, int winW, int winH, HWND hwnd) {
  if (winW < 0 || winH < 0)
    return 0;

  wd->oldW = winW;
  wd->oldH = winH;
  wd->newW = wd->newH = 0;

  int nSymbPerH = winH / (FONT_SIZE_DEF);
  wd->allocatedPtrs = (int)(1080 / FONT_SIZE_DEF) > nSymbPerH ? (int)(1080 / FONT_SIZE_DEF) : nSymbPerH;

  if ((wd->stringsBegins = (char **)malloc(sizeof(char *) * wd->allocatedPtrs)) == NULL)
    return 0;

  HDC hdc = GetDC(hwnd);
  HFONT hFnt = CreateFont(FONT_SIZE_DEF, (int)(FONT_SIZE_DEF / 2), 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, RUSSIAN_CHARSET, OUT_DEFAULT_PRECIS, OUT_OUTLINE_PRECIS, PROOF_QUALITY, FIXED_PITCH | FF_MODERN, (LPCSTR)"Sans");

  SelectObject(hdc, hFnt);
  SetTextColor(hdc, RGB(0, 0, 0));
  SetBkMode(hdc, TRANSPARENT);

  return 1;
}

int WD_UpdateSizes(winDrawer_t* wd, int newW, int newH) {
  if (newW < 0 || newH < 0 || wd->stringsBegins == NULL)
    return 0;

  wd->oldW = wd->newW;
  wd->oldH = wd->newH;
  wd->newW = newW;
  wd->newH = newH;

  int nSymbPerH = wd->newH / (FONT_SIZE_DEF);

  // if we need to realloc
  if (nSymbPerH > wd->allocatedPtrs) {
    wd->allocatedPtrs = nSymbPerH;

    char **newBuf;
    if ((newBuf = (char **)realloc(wd->stringsBegins, sizeof(char *) * wd->allocatedPtrs)) == NULL)
      return 0;

    wd->stringsBegins = newBuf;
  }

  return 1;
}

int WD_IsNeedToReparse(winDrawer_t wd) {
  int oSymbPerH = wd.oldH / (FONT_SIZE_DEF), oSymbPerW = wd.oldW / (int)(FONT_SIZE_DEF / 2),
      nSymbPerH = wd.newH / (FONT_SIZE_DEF), nSymbPerW = wd.newW / (int)(FONT_SIZE_DEF / 2);

  if (oSymbPerH != nSymbPerH || oSymbPerW != nSymbPerW)
    return 1;
  return 0;
}

int WD_ReparseText(textReader_t tr, winDrawer_t* wd) {
  int nSymbPerH = wd->newH / (FONT_SIZE_DEF), nSymbPerW = wd->newW / (int)(FONT_SIZE_DEF / 2);

  int y = 0, x = 0, totalPos = 0;

  while (y < nSymbPerH) {
    x = 0;
    wd->stringsBegins[y++] = &(tr.textBuf[totalPos]);

    while (x < nSymbPerW) {
      x++;
      totalPos++;

      if (totalPos == tr.charsReaden)
        break;
      else if (tr.textBuf[totalPos] == '\n') {
        totalPos++;
        break;
      }
    }

    if (totalPos == tr.charsReaden)
      break;
  }
  // if symbols less than can fit client window
  wd->stringsCnt = y;
  wd->lastStrLen = x;

  return 1;
}

void WD_DrawText(HWND hwnd, winDrawer_t wd) {
  PAINTSTRUCT ps;
  HDC hdc = BeginPaint(hwnd, &ps);
  HFONT hFnt = CreateFont(FONT_SIZE_DEF, (int)(FONT_SIZE_DEF / 2), 0, 0, FW_REGULAR, FALSE, FALSE, FALSE, RUSSIAN_CHARSET, OUT_DEFAULT_PRECIS, OUT_OUTLINE_PRECIS, PROOF_QUALITY, FIXED_PITCH | FF_MODERN, (LPCSTR)"Sans");
  SelectObject(hdc, hFnt);
  SetTextColor(hdc, RGB(0, 0, 0));
  SetBkMode(hdc, TRANSPARENT);

  int y = 0;

  while (y < wd.stringsCnt) {
    if (y == wd.stringsCnt - 1)
      TextOut(hdc, 0, y * FONT_SIZE_DEF, wd.stringsBegins[y], wd.lastStrLen);
    else
      TextOut(hdc, 0, y * FONT_SIZE_DEF, wd.stringsBegins[y], wd.stringsBegins[y + 1] - wd.stringsBegins[y]);

    y++;
  }

  EndPaint(hwnd, &ps);
}

void WD_Destroy(winDrawer_t* wd) {
  if (wd->stringsBegins != NULL)
    free(wd->stringsBegins);
}
