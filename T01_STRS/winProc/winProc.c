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

  if (TR_InitText(&(wpd->tr), (char*)tmpStrct->lpCreateParams) == 0) {
    MessageBox(hwnd, _T("Can't load file to read!"), _T("Error"), MB_ICONERROR);
    PostQuitMessage(0);
  }

  if (WD_Init(&(wpd->wd), wpd->tr, WIN_W, WIN_H, hwnd) == 0) {
    TR_ClearText(&(wpd->tr));
    MessageBox(hwnd, _T("Can't parse file into lines!"), _T("Error"), MB_ICONERROR);
    PostQuitMessage(0);
  }

  wpd->isInit = WPD_INIT;
  SC_ReplaceScrolls(hwnd, wpd->wd);
}


// 'WM_HSCROLL' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_HScrollUpdate(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd) {
  if (wpd->isInit == WPD_NOT_INIT)
    return;

  SCROLLINFO si;

  si.fMask = SIF_ALL;
  GetScrollInfo(hwnd, SB_HORZ, &si);
  int scrPos = si.nPos;

  switch (LOWORD(wParam)) {
  case SB_LINELEFT:
    si.nPos -= 1;
    break;
  case SB_LINERIGHT:
    si.nPos += 1;
    break;
  case SB_PAGELEFT:
    si.nPos -= si.nPage;
    break;
  case SB_PAGERIGHT:
    si.nPos += si.nPage;
    break;
  case SB_THUMBTRACK:
    si.nPos = si.nTrackPos;
    break;
  default:
    break;
  }

  si.fMask = SIF_POS;
  SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
  GetScrollInfo(hwnd, SB_HORZ, &si);

  if (si.nPos != scrPos) {
    int sign = SIGN(si.nPos - scrPos);
    WD_ShiftLineStart(&(wpd->wd), (si.nPos - scrPos) * sign, sign);
    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);
  }
}


// 'WM_VSCROLL' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_VScrollUpdate(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd) {
  if (wpd->isInit == WPD_NOT_INIT)
    return;

  SCROLLINFO si;

  si.fMask = SIF_ALL;
  GetScrollInfo(hwnd, SB_VERT, &si);
  int scrPos = si.nPos;

  switch (LOWORD(wParam)) {
  case SB_LINEUP:
    si.nPos -= 1;
    break;
  case SB_LINEDOWN:
    si.nPos += 1;
    break;
  case SB_PAGEUP:
    si.nPos -= si.nPage;
    break;
  case SB_PAGEDOWN:
    si.nPos += si.nPage;
    break;
  case SB_THUMBTRACK:
    si.nPos = si.nTrackPos;
    break;
  default:
    break;
  }

  si.fMask = SIF_POS;
  SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
  GetScrollInfo(hwnd, SB_VERT, &si);

  if (si.nPos != scrPos) {
    int sign = SIGN(si.nPos - scrPos);
    WD_ShiftTextPosition(&(wpd->wd), (si.nPos - scrPos) * sign, sign);
    InvalidateRect(hwnd, NULL, TRUE);
    UpdateWindow(hwnd);
  }
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
    GetWindowRect(hwnd, &rect);

    if (WD_Init(&(wpd->wd), wpd->tr, rect.right - rect.left, rect.bottom - rect.top, hwnd) == 0) {
      TR_ClearText(&(wpd->tr));
      MessageBox(hwnd, _T("Can't parse file into lines!"), _T("Error"), MB_ICONERROR);
      PostQuitMessage(0);
    }

    wpd->isInit = WPD_INIT;

    WD_ReparseText(&(wpd->wd));
    SC_ReplaceScrolls(hwnd, wpd->wd);
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

  SCROLLINFO si;
  si.fMask = SIF_ALL;
  int scrPos;

  switch (wParam) {
    // horiz scroll navigation
    case VK_LEFT:
    case VK_RIGHT:
      if(wpd->wd.modelViewType == MV_FORMATED)
        return;

      GetScrollInfo(hwnd, SB_HORZ, &si);
      scrPos = si.nPos;

      si.nPos += (wParam == VK_RIGHT ? 1 : -1);

      si.fMask = SIF_POS;
      SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
      GetScrollInfo(hwnd, SB_HORZ, &si);

      if (si.nPos != scrPos) {
        WD_ShiftLineStart(&(wpd->wd), 1, (wParam == VK_RIGHT ? 1 : -1));
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
      }
      return;

    // vert scroll navigation
    case VK_UP:
    case VK_DOWN:
    case VK_PRIOR:
    case VK_NEXT:

      GetScrollInfo(hwnd, SB_VERT, &si);
      scrPos = si.nPos;

      if (wParam == VK_DOWN)
        si.nPos += 1;
      else if (wParam == VK_UP)
        si.nPos -= 1;
      else if (wParam == VK_PRIOR)
        si.nPos -= si.nPage;
      else
        si.nPos += si.nPage;

      si.fMask = SIF_POS;
      SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
      GetScrollInfo(hwnd, SB_VERT, &si);

      if (si.nPos != scrPos) {
        int sign = SIGN(si.nPos - scrPos);
        WD_ShiftTextPosition(&(wpd->wd), (si.nPos - scrPos) * sign, sign);
        InvalidateRect(hwnd, NULL, TRUE);
        UpdateWindow(hwnd);
      }
      return;
  }
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
