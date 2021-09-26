#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
#define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <stdio.h>

#include "textReader/textReader.h"
#include "winDrawer/winDrawer.h"
//#include "textReader.h"
//#include "winDrawer.h"

#define SIGN(X) X < 0 ? -1 : 1

#define WIN_W 544
#define WIN_H 375

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[] = _T("CodeBlocksWindowsApp");

int WINAPI WinMain(HINSTANCE hThisInstance,
  HINSTANCE hPrevInstance,
  LPSTR lpszArgument,
  int nCmdShow)
{
  HWND hwnd;               /* This is the handle for our window */
  MSG messages;            /* Here messages to the application are saved */
  WNDCLASSEX wincl;        /* Data structure for the windowclass */

  /* The Window structure */
  wincl.hInstance = hThisInstance;
  wincl.lpszClassName = szClassName;
  wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
  wincl.style = CS_DBLCLKS | CS_CLASSDC | CS_OWNDC;                 /* Catch double-clicks */
  wincl.cbSize = sizeof(WNDCLASSEX);

  /* Use default icon and mouse-pointer */
  wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
  wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
  wincl.lpszMenuName = NULL;                 /* No menu */
  wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
  wincl.cbWndExtra = 0;                      /* structure or the window instance */
  /* Use Windows's default colour as the background of the window */
  wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

  /* Register the window class, and if it fails quit the program */
  if (!RegisterClassEx(&wincl))
    return 0;

  /* The class is registered, let's create the program*/
  hwnd = CreateWindowEx(
    0,                                /* Extended possibilites for variation */
    szClassName,                      /* Classname */
    _T("Text reader"),                /* Title Text */
    WS_OVERLAPPEDWINDOW | WS_VSCROLL, /* default window with vertical scroll */
    CW_USEDEFAULT,                    /* Windows decides the position */
    CW_USEDEFAULT,                    /* where the window ends up on the screen */
    WIN_W,                            /* The programs width */
    WIN_H,                            /* and height in pixels */
    HWND_DESKTOP,                     /* The window is a child-window to desktop */
    NULL,                             /* No menu */
    hThisInstance,                    /* Program Instance handler */
    lpszArgument                      /* The only Window Creation data - filename */
  );

  /* Make the window visible on the screen */
  ShowWindow(hwnd, nCmdShow);

  /* Run the message loop. It will run until GetMessage() returns 0 */
  while (GetMessage(&messages, NULL, 0, 0))
  {
    /* Translate virtual-key messages into character messages */
    TranslateMessage(&messages);
    /* Send message to WindowProcedure */
    DispatchMessage(&messages);
  }

  /* The program return-value is 0 - The value that PostQuitMessage() gave */
  return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  static textReader_t tr;
  static winDrawer_t wd;
  static SCROLLINFO si;

  switch (message)                  /* handle the messages */
  {
  case WM_DESTROY: {
    TR_ClearText(&tr);
    WD_Destroy(&wd);
    PostQuitMessage(0);       /* send a WM_QUIT to the message queue */
    break;
  }

  // Load file then create window
  case WM_CREATE: {
    CREATESTRUCT* tmpStrct = (CREATESTRUCT*)lParam;

    //if (TR_InitText(&tr, "orwell.txt") == 0) {
    if (TR_InitText(&tr, (char*)tmpStrct->lpCreateParams) == 0) {
      MessageBox(hwnd, _T("Can't load file to read!"), _T("Error"), MB_ICONERROR);
      PostQuitMessage(0);
    }

    if (WD_Init(&wd, tr, WIN_W, WIN_H, hwnd) == 0) {
      MessageBox(hwnd, _T("Can't parse file into lines!"), _T("Error"), MB_ICONERROR);
      PostQuitMessage(0);
    }

    // make first text parse
    int yPos = WD_ReparseText(tr, &wd);

    // fill vert scroll info
    si.cbSize = sizeof(si);
    si.fMask = SIF_RANGE | SIF_PAGE;
    si.nMin = 0;
    si.nPos = yPos;
    si.nMax = wd.totalLinesInWin - 1;
    si.nPage = wd.linesInWindow;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

    break;
  }

  case WM_VSCROLL: {
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
      WD_ShiftTextPosition(&wd, (si.nPos - scrPos) * sign, sign);
      InvalidateRect(hwnd, NULL, TRUE);
      UpdateWindow(hwnd);
    }
  }

                 // Load text to draw then resize window
  case WM_SIZE: {
    WD_UpdateSizes(&wd, LOWORD(lParam), HIWORD(lParam));

    // reread, if params changed
    if (WD_IsNeedToReparse(wd) == 1) {
      // repars symbols
      int newPos = WD_ReparseText(tr, &wd);

      // fill vert scroll info
      si.cbSize = sizeof(si);
      si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
      si.nMin = 0;
      si.nMax = wd.totalLinesInWin - 1;
      si.nPos = newPos;
      si.nPage = wd.linesInWindow;
      SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

      // draw text
      InvalidateRect(hwnd, NULL, TRUE);
      UpdateWindow(hwnd);
    }
    break;
  }
  case WM_PAINT: {
    WD_DrawText(hwnd, wd);
    break;
  }

  default:                      /* for messages that we don't deal with */
    return DefWindowProc(hwnd, message, wParam, lParam);
  }
  return 0;
}
