#ifndef __GUI_RICH_UTIL_H__
#define __GUI_RICH_UTIL_H__

#include <windows.h>
#include <tchar.h>
#include <Richedit.h>

int InitializeRichEditModule(const TCHAR *);

void ReleaseRichEditModule();

void RchEdtAppenText(HWND ctrl_hwnd, const TCHAR *msg, const CHARFORMAT *cf = 0, bool redraw = false);


#endif // __GUI_RICH_UTIL_H__
