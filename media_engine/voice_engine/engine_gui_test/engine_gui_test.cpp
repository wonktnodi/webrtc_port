// engine_gui_test.cpp : main source file for engine_gui_test.exe
//

#include "stdafx.h"

#include "resource.h"

#include "about_dlg.h"
#include "engine_dlg.h"
#include "gui/rich_util.h"


CAppModule _Module;

int Run(LPTSTR /*lpstrCmdLine*/ = NULL, int nCmdShow = SW_SHOWDEFAULT)
{
  CMessageLoop theLoop;
  _Module.AddMessageLoop(&theLoop);

  CEngMainDlg dlgMain;

  if(dlgMain.Create(NULL) == NULL)
  {
    ATLTRACE(_T("Main dialog creation failed!\n"));
    return 0;
  }

  dlgMain.ShowWindow(nCmdShow);

  int nRet = theLoop.Run();

  _Module.RemoveMessageLoop();
  return nRet;
}

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
  HRESULT hRes = ::CoInitialize(NULL);
// If you are running on NT 4.0 or higher you can use the following call instead to 
// make the EXE free threaded. This means that calls come in on a random RPC thread.
//	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
  ATLASSERT(SUCCEEDED(hRes));

  // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
  ::DefWindowProc(NULL, 0, 0, 0L);

  AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

  InitializeRichEditModule(CRichEditCtrl::GetLibraryName());


  hRes = _Module.Init(NULL, hInstance);
  ATLASSERT(SUCCEEDED(hRes));

  int nRet = Run(lpstrCmdLine, nCmdShow);

  ReleaseRichEditModule();

  _Module.Term();
  ::CoUninitialize();

  return nRet;
}
