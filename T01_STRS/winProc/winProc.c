#include "winProc.h"

#define ABS(X) X < 0 ? X * (-1) : X
#define SIGN(X) X < 0 ? -1 : 1

#define MAX_TXT_PATH 300

// 'WM_CREATE' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
//       winProcData_t *wpd - window process data
// RETURNS: none
void WPD_Create(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd) {
  CREATESTRUCT* tmpStrct = (CREATESTRUCT*)lParam;

  if (TR_InitText(&(wpd->tr), (char*)tmpStrct->lpCreateParams) != 0) {
    RECT rect;
    GetClientRect(hwnd, &rect);

    if (WD_Init(&(wpd->wd), wpd->tr, rect.right - rect.left, rect.bottom - rect.top, hwnd) == 0) {
      TR_ClearText(&(wpd->tr));
      MessageBox(hwnd, _T("Can't parse file into lines!"), _T("Error"), MB_ICONERROR);
      PostQuitMessage(0);
    }

    wpd->isInit = WPD_INIT;
    WD_ReparseText(&(wpd->wd));
    SC_ReplaceScrolls(hwnd, wpd->wd);
  }
  else {
    wpd->isInit = WPD_NOT_INIT;
    ShowScrollBar(hwnd, SB_VERT, FALSE);
  }
}


// Vertical scroll processor
// ARGS: HWND hwnd - window
//       int shift - vertical shift size
//       winProcData_t *wpd - window process data
// RETURNS: none.
static void WPD_VertScroll(HWND hwnd, int shift, winProcData_t *wpd) {
  if (wpd->isInit == WPD_NOT_INIT)
    return;

  unsigned int yScrollSize = wpd->wd.modelViewType == MV_FORMATED ? wpd->wd.totalLinesInWin : wpd->wd.linesCnt,
               yScrollPos = wpd->wd.modelViewType == MV_FORMATED ? wpd->wd.yScrollCoord : wpd->wd.lineStart;

  // no need to scroll - all lines can fit in window
  if (yScrollSize < wpd->wd.linesInWindow)
    return;

  int sign = SIGN(shift);

  if (sign < 0 && shift * sign > yScrollPos)
      shift = sign * yScrollPos;
  else if (sign > 0 && shift * sign > ((signed int)yScrollSize - (signed int)wpd->wd.linesInWindow - (signed int)yScrollPos))
      shift = (signed int)yScrollSize - (signed int)wpd->wd.linesInWindow - (signed int)yScrollPos;

  if (shift != 0) {
    WD_ShiftTextPosition(&(wpd->wd), (shift) * sign, sign);

    SC_ReplaceScrolls(hwnd, wpd->wd);

    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);
  }
}


// Horizontal scroll processor
// ARGS: HWND hwnd - window
//       int shift - vertical shift size
//       winProcData_t *wpd - window process data
// RETURNS: none.
static void WPD_HorzScroll(HWND hwnd, int shift, winProcData_t *wpd) {
  if (wpd->isInit == WPD_NOT_INIT)
    return;

  int sign = SIGN(shift);

  // no need to scroll - all lines can fit in window
  if (wpd->wd.maxLineLength < wpd->wd.symbolsPerWindowLine)
    return;

  if (sign < 0 && shift * sign > wpd->wd.xScrollCoord)
      shift = (-1) * (signed int)wpd->wd.xScrollCoord;
  else if (sign > 0 && shift * sign > (signed int)wpd->wd.maxLineLength - (signed int)wpd->wd.symbolsPerWindowLine - (signed int)wpd->wd.xScrollCoord)
      shift = (signed int)wpd->wd.maxLineLength - (signed int)wpd->wd.symbolsPerWindowLine - (signed int)wpd->wd.xScrollCoord;

  if (shift != 0) {
    WD_ShiftLineStart(&(wpd->wd), shift * sign, sign);

    SC_ReplaceScrolls(hwnd, wpd->wd);

    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);
  }
}


// 'WM_HSCROLL' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_HScrollUpdate(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd) {
  if (wpd->isInit == WPD_NOT_INIT)
    return;

  int shift = 0, newPos;

  switch (LOWORD(wParam)) {
  case SB_LINELEFT:
    shift = -1;
    break;
  case SB_LINERIGHT:
    shift = 1;
    break;
  case SB_PAGELEFT:
    shift = -1 * (signed int)(wpd->wd.symbolsPerWindowLine);
    break;
  case SB_PAGERIGHT:
    shift = wpd->wd.symbolsPerWindowLine;
    break;
  case SB_THUMBTRACK:
    newPos = HIWORD(wParam) / wpd->wd.xScrollScale;
    shift = newPos - wpd->wd.xScrollCoord;
    break;
  default:
    break;
  }

  WPD_HorzScroll(hwnd, shift, wpd);
}


// 'WM_VSCROLL' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_VScrollUpdate(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd) {
  if (wpd->isInit == WPD_NOT_INIT)
    return;

  int shift = 0, newPos;
  unsigned int yScrollPos = wpd->wd.modelViewType == MV_FORMATED ? wpd->wd.yScrollCoord : wpd->wd.lineStart;

  switch (LOWORD(wParam)) {
  case SB_LINEUP:
    shift = -1;
    break;
  case SB_LINEDOWN:
    shift = 1;
    break;
  case SB_PAGEUP:
    shift = -1 * (signed int)(wpd->wd.linesInWindow);
    break;
  case SB_PAGEDOWN:
    shift = wpd->wd.linesInWindow;
    break;
  case SB_THUMBTRACK:
    newPos = HIWORD(wParam) / wpd->wd.yScrollScale;
    shift = newPos - yScrollPos;
    break;
  default:
    break;
  }

  WPD_VertScroll(hwnd, shift, wpd);
}


static void CloseOldTxt(winProcData_t *wpd) {
  if (wpd->isInit == WPD_INIT) {
    wpd->isInit = WPD_NOT_INIT;

    TR_ClearText(&(wpd->tr));
    WD_Destroy(&(wpd->wd));
  }
}


static void OpenNewTxt(HWND hwnd, winProcData_t *wpd) {

  OPENFILENAME ofn;
  char fileName[MAX_TXT_PATH];
  fileName[0] = 0;

  // init OPENFILENAME
  memset(&ofn, 0, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hwnd;
  ofn.lpstrFile = fileName;
  ofn.nMaxFile = sizeof(fileName);
  ofn.lpstrFilter = "*.txt\0";
  ofn.nFilterIndex = 1;
  ofn.lpstrInitialDir = NULL;
  ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

  if (GetOpenFileName(&ofn) == TRUE) {
    CloseOldTxt(wpd);

    if (TR_InitText(&(wpd->tr), ofn.lpstrFile) == 0) {
      MessageBox(hwnd, _T("Can't load file to read!"), _T("Error"), MB_ICONERROR);
      PostQuitMessage(0);
    }

    RECT rect;
    GetClientRect(hwnd, &rect);

    if (WD_Init(&(wpd->wd), wpd->tr, rect.right - rect.left, rect.bottom - rect.top, hwnd) == 0) {
      TR_ClearText(&(wpd->tr));
      MessageBox(hwnd, _T("Can't parse file into lines!"), _T("Error"), MB_ICONERROR);
      PostQuitMessage(0);
    }

    wpd->isInit = WPD_INIT;

    WD_ReparseText(&(wpd->wd));
    SC_ReplaceScrolls(hwnd, wpd->wd);


    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);
  }

  return;
}


// 'WM_COMMAND' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_Command(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd) {
  switch (LOWORD(wParam)) {
    case IDM_FILE_OPEN:
      OpenNewTxt(hwnd, wpd);
      return;

    case IDM_FILE_CLOSE:
      CloseOldTxt(wpd);

      ShowScrollBar(hwnd, SB_VERT, FALSE);
      ShowScrollBar(hwnd, SB_HORZ, FALSE);

      InvalidateRect(hwnd, NULL, TRUE);
      UpdateWindow(hwnd);
      return;

    case IDM_FILE_EXIT:
      WPD_Destroy(hwnd, wParam, lParam, wpd);
      return;

    case IDM_VIEW_SWITCH:
      if (wpd->isInit == WPD_NOT_INIT)
        return;

      WD_SwitchType(&(wpd->wd), hwnd);
      WD_ReparseText(&(wpd->wd));

      SC_ReplaceScrolls(hwnd, wpd->wd);
      // draw text
      InvalidateRect(hwnd, NULL, TRUE);
      UpdateWindow(hwnd);
      return;
  }
}


// 'WM_KEYDOWN' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_KeyDown(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd) {
  if (wpd->isInit == WPD_NOT_INIT)
    return;

  int shift = 0;

  switch (wParam) {
    // horiz scroll navigation
    case VK_LEFT:
    case VK_RIGHT:
      if(wpd->wd.modelViewType == MV_FORMATED)
        return;

      shift = (wParam == VK_RIGHT ? 1 : -1);

      WPD_HorzScroll(hwnd, shift, wpd);
      return;

    // vert scroll navigation
    case VK_UP:
    case VK_DOWN:
    case VK_PRIOR:
    case VK_NEXT:
      if (wParam == VK_DOWN)
        shift = 1;
      else if (wParam == VK_UP)
        shift = -1;
      else if (wParam == VK_PRIOR)
        shift = -1 * (signed int)wpd->wd.linesInWindow;
      else
        shift = (signed int)wpd->wd.linesInWindow;

      WPD_VertScroll(hwnd, shift, wpd);
      return;
  }
}


// 'WM_MOUSEWHEEL' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_MouseWheel(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd) {
  if (wpd->isInit == WPD_NOT_INIT)
    return;

  int shift = -1 * GET_WHEEL_DELTA_WPARAM(wParam) / 120;
  WPD_VertScroll(hwnd, shift, wpd);
}


// 'WM_SIZE' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_Size(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd) {
  if (wpd->isInit == WPD_NOT_INIT)
    return;

  WD_UpdateSizes(&(wpd->wd), LOWORD(lParam), HIWORD(lParam));

  // reread, if params changed
  if (WD_IsNeedToReparse(wpd->wd) == 1) {
    // repars symbols
    WD_ReparseText(&(wpd->wd));

    // reparse scrolls
    SC_ReplaceScrolls(hwnd, wpd->wd);

    // draw text
    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);
  }
}


// 'WM_PAINT' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_Paint(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd) {
  if (wpd->isInit == WPD_NOT_INIT)
    return;

  WD_DrawText(hwnd, wpd->wd);
}


// 'WM_DESTROY' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_Destroy(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd) {
  CloseOldTxt(wpd);
  PostQuitMessage(0);       /* send a WM_QUIT to the message queue */
}
