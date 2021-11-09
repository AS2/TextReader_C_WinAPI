#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
#define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <stdio.h>

#include "winProc/winProc.h"

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
    wincl.lpszMenuName = MAKEINTRESOURCE(ID_MYMENU);
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
    static winProcData_t wpd;

    switch (message)                  /* handle the messages */
    {
    case WM_DESTROY:
        WPD_Destroy(hwnd, wParam, lParam, &wpd);
        break;

    case WM_CREATE:
        WPD_Create(hwnd, wParam, lParam, &wpd);
        break;

    case WM_VSCROLL:
        WPD_VScrollUpdate(hwnd, wParam, lParam, &wpd);
        break;

    case WM_HSCROLL:
        WPD_HScrollUpdate(hwnd, wParam, lParam, &wpd);
        break;

    case WM_KEYDOWN:
        WPD_KeyDown(hwnd, wParam, lParam, &wpd);
        break;

    case WM_MOUSEWHEEL:
        WPD_MouseWheel(hwnd, wParam, lParam, &wpd);
        break;

    case WM_COMMAND:
        WPD_Command(hwnd, wParam, lParam, &wpd);
        break;

    case WM_SIZE:
        WPD_Size(hwnd, wParam, lParam, &wpd);
        break;

    case WM_PAINT:
        WPD_Paint(hwnd, wParam, lParam, &wpd);
        break;

    default:                      /* for messages that we don't deal with */
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}
