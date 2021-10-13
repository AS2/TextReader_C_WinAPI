#ifndef __SCROLL_H__
#define __SCROLL_H__

#include <windows.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>

#include "../winDrawer/winDrawer.h"

// replace scrolls positions function
// ARGS: winDrawer_t wd - model view to use
//       HWND hwnd - window with scrolls
// RETURNS: none.
void SC_ReplaceScrolls(HWND hwnd, winDrawer_t wd);

#endif // __SCROLL_H__
