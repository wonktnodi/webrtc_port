// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

class CEngMainDlg : public CDialogImpl<CEngMainDlg>, public CUpdateUI<CEngMainDlg>,
    public CMessageFilter, public CIdleHandler {
public:
  enum { IDD = IDD_MAINDLG };

  virtual BOOL PreTranslateMessage(MSG* pMsg);
  virtual BOOL OnIdle();

  BEGIN_UPDATE_UI_MAP(CEngMainDlg)
  END_UPDATE_UI_MAP()

  BEGIN_MSG_MAP(CEngMainDlg)    
    COMMAND_HANDLER(IDC_BTN_LOOPBACK, BN_CLICKED, OnBtnLoopback)
    COMMAND_HANDLER(IDC_BTN_INPUT_TO_FILE, BN_CLICKED, OnBtnInputToFile)
    COMMAND_HANDLER(IDC_CHK_RTP_DUMP_IN, BN_CLICKED, OnChkRtpDumpIn)
    COMMAND_HANDLER(IDC_CHK_RTP_DUMP_OUT, BN_CLICKED, OnChkRtpDumpOut)
    COMMAND_HANDLER(IDC_CMB_DEV_INPUT, CBN_SELCHANGE, OnSelchangeCmbDevInput)
    COMMAND_HANDLER(IDC_CMB_DEV_OUTPUT, CBN_SELCHANGE, OnSelchangeCmbDevOutput)
    NOTIFY_HANDLER(IDC_SLD_INPUT_VOLUME, NM_RELEASEDCAPTURE, OnReleasedcaptureSldInputVolume)
    NOTIFY_HANDLER(IDC_SLD_OUTPUT_VOLUME, NM_RELEASEDCAPTURE, OnReleasedcaptureSldOutputVolume)
    NOTIFY_HANDLER(IDC_LST_CHANNELS, LVN_ITEMCHANGED, OnItemchangedLstChannels)
    COMMAND_HANDLER(IDC_CHK_NS, BN_CLICKED, OnChkNS)
    NOTIFY_HANDLER(IDC_SLD_MIC_NS, NM_RELEASEDCAPTURE, OnReleasedcaptureSldMicNS)
    COMMAND_HANDLER(IDC_CHK_EC, BN_CLICKED, OnChkEC)
    NOTIFY_HANDLER(IDC_SLD_MIC_EC, NM_RELEASEDCAPTURE, OnReleasedcaptureSldMicEC)
    NOTIFY_HANDLER(IDC_SLD_VAD, NM_RELEASEDCAPTURE, OnReleasedcaptureSldVAD)
    COMMAND_HANDLER(IDC_CHK_VAD, BN_CLICKED, OnChkVAD)
    COMMAND_HANDLER(IDC_CHK_CHL_PLAY_FILE_IN, BN_CLICKED, OnChkPlayFileIn)
    COMMAND_HANDLER(IDC_CHK_CHL_PLAY_FILE_OUT, BN_CLICKED, OnChkChlPlayFileOut)
    COMMAND_HANDLER(IDC_BTN_CHL_START_RECEIVE, BN_CLICKED, OnBtnChlStartReceive)
    COMMAND_HANDLER(IDC_BTN_CHL_STOP_RECEIVE, BN_CLICKED, OnBtnChlStopReceive)
    COMMAND_HANDLER(IDC_BTN_CHL_START_SEND, BN_CLICKED, OnBtnChlStartSend)
    COMMAND_HANDLER(IDC_BTN_CHL_STOP_SEND, BN_CLICKED, OnBtnChlStopSend)
    COMMAND_HANDLER(IDC_BTN_CHL_START_PLAYOUT, BN_CLICKED, OnChlStartPlayout)
    COMMAND_HANDLER(IDC_BTN_CHL_STOP_PLAYOUT, BN_CLICKED, OnBtnChlStopPlayout)
    COMMAND_HANDLER(IDC_BTN_CREATE_CHL, BN_CLICKED, OnBtnCreateChl)
    COMMAND_HANDLER(IDC_BTN_DEL_CHL, BN_CLICKED, OnBtnDelChl)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    COMMAND_HANDLER(IDC_BTN_START_TEST, BN_CLICKED, OnBtnStartTest)
    COMMAND_HANDLER(IDC_BTN_STOP_TEST, BN_CLICKED, OnBtnStopTest)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    COMMAND_ID_HANDLER(ID_APP_ABOUT, OnAppAbout)		
    COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
  END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

  LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
  LRESULT OnAppAbout(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);	
  LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

  void CloseDialog(int nVal);
  LRESULT OnBtnStartTest(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnBtnStopTest(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnBtnCreateChl(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnBtnDelChl(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnBtnChlStartSend(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnBtnChlStopSend(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnChlStartPlayout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnBtnChlStopPlayout(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnBtnChlStartReceive(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnBtnChlStopReceive(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnChkPlayFileIn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnChkChlPlayFileOut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnChkVAD(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnReleasedcaptureSldVAD(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
  LRESULT OnChkNS(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnReleasedcaptureSldMicNS(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
  LRESULT OnChkEC(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnReleasedcaptureSldMicEC(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
  LRESULT OnChkVad(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnItemchangedLstChannels(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
  LRESULT OnReleasedcaptureSldInputVolume(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
  LRESULT OnReleasedcaptureSldOutputVolume(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
  LRESULT OnSelchangeCmbDevInput(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnSelchangeCmbDevOutput(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnChkRtpDumpIn(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnChkRtpDumpOut(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnBtnInputToFile(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
  LRESULT OnBtnLoopback(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

protected:
  void GetEngineInfo();
  int InitEngine();
  int DestroyEngine();

private:
  CListViewCtrl _lst_clients;
};
