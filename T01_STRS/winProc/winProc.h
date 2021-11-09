#ifndef __WIN_PROC_H__
#define __WIN_PROC_H__

#include "../menu/defMenu.h"

#include "../textReader/textReader.h"
#include "../scroll/scroll.h"
#include "../winDrawer/winDrawer.h"

#define WIN_W 544
#define WIN_H 375

#define WPD_INIT 1
#define WPD_NOT_INIT 0

// window callback process'es data
typedef struct winProcData winProcData_t;
struct winProcData {
  char isInit;          // initialization flag (take values 'WPD_INIT' or 'WPD_NOT_INIT')

  textReader_t tr;      // 'text view' struct
  winDrawer_t wd;       // 'model view' struct
};


// 'WM_CREATE' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
//       winProcData_t *wpd - window process data
// RETURNS: none
void WPD_Create(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd);


// 'WM_HSCROLL' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_HScrollUpdate(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd);


// 'WM_VSCROLL' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_VScrollUpdate(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd);


// 'WM_COMMAND' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_Command(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd);


// 'WM_KEYDOWN' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_KeyDown(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd);


// 'WM_MOUSEWHEEL' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_MouseWheel(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd);


// 'WM_SIZE' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_Size(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd);


// 'WM_PAINT' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_Paint(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd);


// 'WM_DESTROY' processor
// ARGS: HWND hwnd - window
//       WPARAM wParam,
//       LPARAM lParam - message parameters
// RETURNS: none
void WPD_Destroy(HWND hwnd, WPARAM wParam, LPARAM lParam, winProcData_t *wpd);

#endif // __WIN_PROC_H__
