// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "aboutdlg.h"
#include "MainDlg.h"
#include "aac_codec.h"

#include "mp4file.h"
#include "au_channel.h"
extern "C" {
#include "aacenc.h"
}

BOOL CMainDlg::PreTranslateMessage(MSG* pMsg) {
  return CWindow::IsDialogMessage(pMsg);
}

BOOL CMainDlg::OnIdle() {
  return FALSE;
}

LRESULT CMainDlg::OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/,
                               LPARAM /*lParam*/, BOOL& /*bHandled*/) {
  // center the dialog on the screen
  CenterWindow();

  // set icons
  HICON hIcon = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR,
                                 ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON));
  SetIcon(hIcon, TRUE);
  HICON hIconSmall = AtlLoadIconImage(IDR_MAINFRAME, LR_DEFAULTCOLOR,
                                      ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON));
  SetIcon(hIconSmall, FALSE);

  // register object for message filtering and idle updates
  CMessageLoop* pLoop = _Module.GetMessageLoop();
  ATLASSERT(pLoop != NULL);
  pLoop->AddMessageFilter(this);
  pLoop->AddIdleHandler(this);

  UIAddChildWindowContainer(m_hWnd);

  SetDlgItemText(IDC_EDT_SOURCE, _T("c:\\audio\\ave_maria.wav"));
  SetDlgItemText(IDC_EDT_DEST, _T("f:\\temp\\im\\ave_maria.aac"));
  return TRUE;
}

LRESULT CMainDlg::OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/,
                            BOOL& /*bHandled*/) {
  // unregister message filtering and idle updates
  CMessageLoop* pLoop = _Module.GetMessageLoop();
  ATLASSERT(pLoop != NULL);
  pLoop->RemoveMessageFilter(this);
  pLoop->RemoveIdleHandler(this);

  return 0;
}

LRESULT CMainDlg::OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/,
                             HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
  CAboutDlg dlg;
  dlg.DoModal();
  return 0;
}

LRESULT CMainDlg::OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/,
                       BOOL& /*bHandled*/) {
  // TODO: Add validation code
  CloseDialog(wID);
  return 0;
}

LRESULT CMainDlg::OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/,
                           BOOL& /*bHandled*/) {
  CloseDialog(wID);
  return 0;
}

void CMainDlg::CloseDialog(int nVal) {
  DestroyWindow();
  ::PostQuitMessage(nVal);
}

LRESULT CMainDlg::OnBtnTest(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                            BOOL& bHandled) {
  HANDLE_MP4_FILE hMp4File;
  hAudioChannel inputFile = NULL;
  AuChanInfo inputInfo;
  AuChanMode auFlags = AU_CHAN_READ;
  AuChanType auType = TYPE_AUTODETECT ; /* must be set */

  inputInfo.bitsPerSample  = 16 ;       /* only relevant if valid == 1 */
  inputInfo.sampleRate     = 44100 ;    /* only relevant if valid == 1 */
  inputInfo.nChannels      = 2 ;        /* only relevant if valid == 1 */
  inputInfo.nSamples       = 0 ;        /* only relevant if valid == 1 */
  inputInfo.isLittleEndian = 1;

  inputInfo.fpScaleFactor  = AACENC_PCM_LEVEL ; /* must be set */
  inputInfo.valid          = 1 ;        /* must be set */
  inputInfo.useWaveExt     = 0;

  char file_path[MAX_PATH] = {0};
  GetDlgItemTextA(m_hWnd, IDC_EDT_SOURCE, file_path, MAX_PATH);
  int ret = AuChannelOpen (&inputFile, file_path, auFlags, &auType, &inputInfo);

  struct aac_encoder_t *encoder = 0;
  open_aac_encoder(encoder, inputInfo.sampleRate, inputInfo.sampleRate,
    inputInfo.nChannels, inputInfo.nChannels);

  //AACENC_CONFIG config;
  //AacInitDefaultConfig(&config);

  //unsigned char ASConfigBuffer[80];
  //unsigned int  nConfigBits;
  //unsigned int  nConfigBytes;
  //    
  //memset (ASConfigBuffer, 0, 80);
  //if (GetMPEG4ASConfig(22050,
  //                       1,
  //                       ASConfigBuffer,
  //                       &nConfigBits,
  //                       1,
  //                       1) ) {
  //  fprintf(stderr, "\nCould not initialize Audio Specific Config\n");
  //  exit(10);
  //}

  if (encoder)
    close_aac_encoder(encoder);
  if (inputFile)
    AuChannelClose (inputFile);

  return 0;
}

LRESULT CMainDlg::OnBtnFileSrc(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                               BOOL& bHandled) {
  return 0;
}

LRESULT CMainDlg::OnBtnFileDest(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                                BOOL& bHandled) {
  return 0;
}

static HANDLE _job_thr = 0;

LRESULT CMainDlg::OnBtnStart(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                             BOOL& bHandled) {
  return 0;
}

LRESULT CMainDlg::OnBtnStop(WORD wNotifyCode, WORD wID, HWND hWndCtl,
                            BOOL& bHandled) {
  return 0;
}
