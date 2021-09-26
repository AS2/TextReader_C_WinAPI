#ifndef __win_drawer_h__
#define __win_drawer_h__

#include <windows.h>
#include <tchar.h>

#include <stdio.h>
#include <stdlib.h>

//#include "../textReader/textReader.h"
#include "textReader.h"

#define FONT_SIZE_DEF 18

typedef struct winDrawer winDrawer_t;
struct winDrawer {
    int oldW, oldH, newW, newH;

    char **stringsBegins;
    int lastStrLen, stringsCnt, allocatedPtrs;
};

int WD_Init(winDrawer_t* wd, int winW, int winH, HWND hwnd);

int WD_UpdateSizes(winDrawer_t* wd, int newW, int newH);

int WD_IsNeedToReparse(winDrawer_t wd);
int WD_ReparseText(textReader_t tr, winDrawer_t* wd);

void WD_DrawText(HWND hwnd, winDrawer_t wd);

void WD_Destroy(winDrawer_t* wd);

#endif // __win_drawer_h__
