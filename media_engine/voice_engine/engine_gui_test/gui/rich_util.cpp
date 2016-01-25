#include "rich_util.h"
#include <windows.h>
#include <Richedit.h>
#include <assert.h>

static HMODULE s_rchedt_module = 0;

int InitializeRichEditModule(const TCHAR *class_name) {
  s_rchedt_module = LoadLibrary(class_name);
  return (s_rchedt_module != NULL) ? 0 : -1;
}

void ReleaseRichEditModule() {
  FreeLibrary(s_rchedt_module);
  s_rchedt_module = 0;
}

int GetRichTextLength(HWND hwnd) {
  GETTEXTLENGTHEX gtl;

  gtl.flags = GTL_PRECISE;
  gtl.codepage = CP_ACP ;
  return (int) SendMessage(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 0);
}

//static int		logPixelSY = 0;
//static int		logPixelSX = 0;

void RchEdtAppenText(HWND edt_handle, const TCHAR *msg,  const CHARFORMAT * /*cf = 0*/, bool /*redraw = false*/) {
  assert(msg && edt_handle);

  BOOL bRedraw  = FALSE;

  POINT point = {0};
  CHARRANGE oldsel, sel;
  WPARAM wp;
  BOOL bFlag = FALSE;
  SCROLLINFO scroll;

  scroll.cbSize = sizeof(SCROLLINFO);
  scroll.fMask = SIF_RANGE | SIF_POS | SIF_PAGE;
  GetScrollInfo(edt_handle, SB_VERT, &scroll);
  SendMessage(edt_handle, EM_GETSCROLLPOS, 0, (LPARAM) &point);

  // do not scroll to bottom if there is a selection
  SendMessage(edt_handle, EM_EXGETSEL, 0, (LPARAM) &oldsel);
  if (oldsel.cpMax != oldsel.cpMin)
    SendMessage(edt_handle, WM_SETREDRAW, FALSE, 0);

  //set the insertion point at the bottom
  sel.cpMin = sel.cpMax = GetRichTextLength(edt_handle);
  SendMessage(edt_handle, EM_EXSETSEL, 0, (LPARAM) &sel);

  // fix for the indent... must be a M$ bug
  if (sel.cpMax == 0)
    bRedraw = TRUE;

  // should the event(s) be appended to the current log
  wp = bRedraw ? SF_RTF : SFF_SELECTION | SF_RTF;

  //get the number of pixels per logical inch
  if (bRedraw) {
    /*HDC hdc;
    hdc = GetDC(NULL);
    logPixelSY = GetDeviceCaps(hdc, LOGPIXELSY);
    logPixelSX = GetDeviceCaps(hdc, LOGPIXELSX);
    ReleaseDC(NULL, hdc);*/
    SendMessage(edt_handle, WM_SETREDRAW, FALSE, 0);
    bFlag = TRUE;
    //			SetCursor(LoadCursor(NULL, IDC_ARROW));
  }

  SendMessage(edt_handle, EM_REPLACESEL, 0, (LPARAM)msg);

  // scroll log to bottom if the log was previously scrolled to bottom, else restore old position
  if ((bRedraw || (UINT)scroll.nPos >= (UINT)scroll.nMax - scroll.nPage - 5 ||
    scroll.nMax - scroll.nMin - scroll.nPage < 50)) {
    //SendMessage(GetParent(hwndRich), GC_SCROLLTOBOTTOM, 0, 0); 
    SCROLLINFO si = { 0 };
    CHARRANGE sel;
    si.cbSize = sizeof(si);
    si.fMask = SIF_PAGE | SIF_RANGE;
    GetScrollInfo(edt_handle, SB_VERT, &si);

    si.fMask = SIF_POS;
    si.nPos = si.nMax - si.nPage + 1;
    SetScrollInfo(edt_handle, SB_VERT, &si, TRUE);

    sel.cpMin = sel.cpMax = GetRichTextLength(edt_handle);
    SendMessage(edt_handle, EM_EXSETSEL, 0, (LPARAM) & sel);
    PostMessage(edt_handle, WM_VSCROLL, MAKEWPARAM(SB_BOTTOM, 0), 0);
  } else {
    SendMessage(edt_handle, EM_SETSCROLLPOS, 0, (LPARAM) &point);
  }
  // need to invalidate the window
  if (bFlag) {
    sel.cpMin = sel.cpMax = GetRichTextLength(edt_handle);
    SendMessage(edt_handle, EM_EXSETSEL, 0, (LPARAM) & sel);
    SendMessage(edt_handle, WM_SETREDRAW, TRUE, 0);
    InvalidateRect(edt_handle, NULL, TRUE);
  }
}