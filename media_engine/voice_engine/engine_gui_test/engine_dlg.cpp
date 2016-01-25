// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"

#include "about_dlg.h"
#include "engine_dlg.h"

#include "engine_utility.h"
#include "engine_base.hpp"
#include "engine_audio_hardware.hpp"
#include "engine_voe.hpp"
#include "engine_voe_codec.hpp"
#include "engine_network.hpp"
#include "engine_voe_file.hpp"
#include "engine_audio_processing.hpp"
#include "engine_audio_volume.hpp"
#include "engine_debug.hpp"
#include "voe_api.h"
#include <time.h>

#define AMRWB_MAGIC_NUMBER "#!AMR-WB\n"

#ifdef ARM_DEBUG
static FILE *s_out_enc_file = 0;
static int s_record_channel_id = -1;
#endif // ARM_DEBUG

int outgo_audio_data(int channel, uint8_t bit_mask, uint8_t pt, uint16_t seq,
                     uint32_t tm, uint32_t ssrc, const char * data, int len) {
  SYSTEMTIME sm;
  GetLocalTime(&sm);
    
  voe_set_transport_data(channel, bit_mask, pt, seq, tm, ssrc, data, len);

  AtlTrace(_T("%02d:%02d:%02d.%03d Audio pkt(%d) size: %d\n"),
           sm.wHour, sm.wMinute, sm.wSecond, sm.wMilliseconds, channel, len);
#ifdef ARM_DEBUG
  if (s_out_enc_file)
    fwrite(data, 1, len, s_out_enc_file);
#endif // ARM_DEBUG
  
  return len;
}

BOOL CEngMainDlg::PreTranslateMessage(MSG* pMsg) {
  return CWindow::IsDialogMessage(pMsg);
}

BOOL CEngMainDlg::OnIdle() {
  return FALSE;
}

LRESULT CEngMainDlg::OnInitDialog(UINT, WPARAM, LPARAM, BOOL&) {
  // center the dialog on the screen
  CenterWindow();
  _lst_clients.m_hWnd = GetDlgItem(IDC_LST_CHANNELS);
  _lst_clients.ModifyStyle(0,
                           /*LVS_NOCOLUMNHEADER |*/ LVS_SHOWSELALWAYS /*| LVS_SINGLESEL*/);
  _lst_clients.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES,
                                        0);

  _lst_clients.AddColumn(_T("chl"), 0);
  _lst_clients.SetColumnWidth(0, 50);
  _lst_clients.AddColumn(_T("status"), 1);
  _lst_clients.SetColumnWidth(1, 185);

  InitEngine();

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

  CTrackBarCtrl tracker;
  tracker.m_hWnd = GetDlgItem(IDC_SLD_VAD);
  tracker.SetRangeMax(3);
  tracker.SetRangeMin(0);

  tracker.m_hWnd = GetDlgItem(IDC_SLD_MIC_EC);
  tracker.SetRangeMax(4);
  tracker.SetRangeMin(1);

  tracker.m_hWnd = GetDlgItem(IDC_SLD_MIC_NS);
  tracker.SetRangeMax(6);
  tracker.SetRangeMin(1);

  tracker.m_hWnd = GetDlgItem(IDC_SLD_RX_NS);
  tracker.SetRangeMax(6);
  tracker.SetRangeMin(1);

  tracker.m_hWnd = GetDlgItem(IDC_SLD_MIC_AGC);
  tracker.SetRangeMax(4);
  tracker.SetRangeMin(1);

  tracker.m_hWnd = GetDlgItem(IDC_SLD_RX_AGC);
  tracker.SetRangeMax(4);
  tracker.SetRangeMin(1);

  tracker = GetDlgItem(IDC_SLD_INPUT_VOLUME);
  tracker.SetRangeMax(255);
  tracker.SetRangeMin(0);

  tracker = GetDlgItem(IDC_SLD_OUTPUT_VOLUME);
  tracker.SetRangeMax(255);
  tracker.SetRangeMin(0);


  // load engine informations.
  GetEngineInfo();

  voe_debug_write_dump_date_externinfo("d:/7f Game/Projects/MediaService/build/Debug/log/audio/0311_160006_1000_1000_0_out_529.rtpdmp",
                             "123", 3);
  return TRUE;
}

LRESULT CEngMainDlg::OnDestroy(UINT, WPARAM, LPARAM, BOOL&) {
  DestroyEngine();

  // unregister message filtering and idle updates
  CMessageLoop* pLoop = _Module.GetMessageLoop();
  ATLASSERT(pLoop != NULL);
  pLoop->RemoveMessageFilter(this);
  pLoop->RemoveIdleHandler(this);

  return 0;
}

LRESULT CEngMainDlg::OnAppAbout(WORD, WORD, HWND, BOOL&) {
  CAboutDlg dlg;
  dlg.DoModal();
  return 0;
}

LRESULT CEngMainDlg::OnCancel(WORD, WORD wID, HWND, BOOL&) {
  CloseDialog(wID);
  return 0;
}

void CEngMainDlg::CloseDialog(int nVal) {
  DestroyWindow();
  ::PostQuitMessage(nVal);
}

LRESULT CEngMainDlg::OnBtnStartTest(WORD, WORD, HWND, BOOL&) { 
  
  return 0;
}

LRESULT CEngMainDlg::OnBtnStopTest(WORD, WORD, HWND, BOOL&) {
  return 0;
}

void CEngMainDlg::GetEngineInfo() {
  /*MediaEngineVoiceHardware *voice_devices =
    MediaEngineVoiceHardware::GetInterface(s_engine);*/

  CComboBox cmb = GetDlgItem(IDC_CMB_DEV_OUTPUT);
  //assert(voice_devices);
  cmb.ResetContent();
  cmb.AddString(_T("Default device"));
  cmb.AddString(_T("Default communication device"));
  int dev_cnt = 0;
  voe_get_play_device_count(dev_cnt);
  //voice_devices->GetNumOfPlayoutDevices(dev_cnt);
  char dev_name[128], dev_guid[128];

  for(int i = 0; i < dev_cnt; ++ i) {
    //assert(voice_devices->GetPlayoutDeviceName(i, dev_name, dev_guid) == 0);
    assert(voe_get_play_device_name(i, dev_name, dev_guid) == 0);
    cmb.AddString(utf8_to_wchar(dev_name, -1));
  }

  cmb.SetCurSel(0);
  voe_set_play_device(-2);
  //voice_devices->SetPlayoutDevice(-2);

  cmb = GetDlgItem(IDC_CMB_DEV_INPUT);
  cmb.ResetContent();
  cmb.AddString(_T("Default device"));
  cmb.AddString(_T("Default communication device"));
  //voice_devices->GetNumOfRecordingDevices(dev_cnt);
  voe_get_rec_device_count(dev_cnt);

  for(int i = 0; i < dev_cnt; ++ i) {
    //assert(voice_devices->GetRecordingDeviceName(i, dev_name, dev_guid) == 0);
    assert(voe_get_rec_device_name(i, dev_name, dev_guid) == 0);
    cmb.AddString(utf8_to_wchar(dev_name, -1));
  }

  cmb.SetCurSel(0);
  //voice_devices->SetRecordingDevice(-2);
  voe_set_rec_device(-2);
  //voice_devices->Release(voice_devices);

  // get audio codec info.
  cmb = GetDlgItem(IDC_CMB_CODEC);
  cmb.ResetContent();
  /*MediaEngineVoeCodec *voe_codec =
    MediaEngineVoeCodec::GetInterface(s_engine);*/

  int codec_cnt = 0; //voe_codec->NumOfCodecs();
  voe_get_audio_codec_count(codec_cnt);
  CodecInst codec_inst;
  USES_CONVERSION;
  TCHAR codec_info[256];

  for (int i = 0; i < codec_cnt; ++ i) {
    //voe_codec->GetCodec(i, codec_inst);
    voe_get_audio_codec_detail(i, codec_inst);
    _stprintf(codec_info, _T("%s(%d/%d/%d)"), A2T(codec_inst.plname),
              codec_inst.plfreq, codec_inst.channels, codec_inst.pltype);
    int idx = cmb.AddString(codec_info);
    cmb.SetItemData(idx, i/*codec_inst.pltype*/);
  }

  cmb.SetCurSel(14);  

  unsigned int val;  
  voe_get_rec_device_volume(val);
  CTrackBarCtrl tracker = GetDlgItem(IDC_SLD_INPUT_VOLUME);
  tracker.SetPos(val);
    
  voe_get_play_device_volume(val);
  tracker = GetDlgItem(IDC_SLD_OUTPUT_VOLUME);
  tracker.SetPos(val);
}

int CEngMainDlg::InitEngine() {
  TCHAR file_path[256] = {0};
  GetModuleFileName(_Module.get_m_hInst(), file_path, 256);
  TCHAR *pos = _tcsrchr(file_path, _T('\\'));
  *(pos + 1) = 0;
  _tcscat(file_path, _T("engine.log"));

  voe_init(wchar_to_utf8(file_path, -1), kTraceAll, kTraceModuleAll);
  
  voe_register_audio_transport(outgo_audio_data);

  /*s_engine = MediaEngine::Create();

  MediaEngine::SetTraceFilter(kTraceAll, kTraceModuleAll);
  voe_set_log_filter(kTraceAll, kTraceModuleAll);

  s_engine->Init();

  
  MediaEngine::SetTraceFile(wchar_to_utf8(file_path, -1));
  voe_set_log_path(wchar_to_utf8(file_path, -1));*/

  /*s_voe = MediaEngineVoe::GetInterface(s_engine);

  s_network = MediaEngineNetwork::GetInterface(s_engine);

  s_transport.outgo_audio_data = outgo_audio_data;
  s_network->RegisterTransport(&s_transport);

  s_voe_file = MediaEngineVoiceFile::GetInterface(s_engine);

  s_apm = MediaEngineAPM::GetInterface(s_engine);

  s_voe_volume = EngineAudioVolume::GetInterface(s_engine);

  s_eng_dbg = MediaEngineDebug::GetInterface(s_engine);*/

  return 0;
}

int CEngMainDlg::DestroyEngine() {
  /*if (s_voe_volume) {
  EngineAudioVolume::Release(s_voe_volume);
  s_voe_volume = 0;
  }

  if (s_voe_file) {
  MediaEngineVoiceFile::Release(s_voe_file);
  }

  if (s_apm) {
  MediaEngineAPM::Release(s_apm);
  s_apm = 0;
  }

  if (s_eng_dbg) {
  MediaEngineDebug::Release(s_eng_dbg);
  s_eng_dbg = 0;
  }

  if (s_network) {
  s_transport.outgo_audio_data = 0;
  s_network->RegisterTransport(0);
  MediaEngineNetwork::Release(s_network);
  }

  if (s_voe) {
  MediaEngineVoe::Release(s_voe);
  s_voe = 0;
  }

  MediaEngine::Delete(s_engine);*/

  voe_release();
  return 0;
}

LRESULT CEngMainDlg::OnBtnLoopback(WORD /*wNotifyCode*/, WORD /*wID*/,
                                   HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
  static bool test_flag = false;

  if (test_flag) {
    test_flag = true;
  } else {
    test_flag = true;
  }

  return 0;
}

LRESULT CEngMainDlg::OnBtnInputToFile(WORD /*wNotifyCode*/, WORD /*wID*/,
                                      HWND /*hWndCtl*/, BOOL& /*bHandled*/) {
  char file_name[MAX_PATH] = {0};
  strcpy(file_name, "f:\\temp\\record.wav");
  //s_voe_file->StartRecordPlayout()
  voe_start_record_mircophone(file_name);

  return 0;
}

LRESULT CEngMainDlg::OnChkRtpDumpIn(WORD, WORD, HWND, BOOL&) {
  CFileDialog dlg(FALSE, _T("*.rtpdump"), 0,
                  OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                  _T("Rtp dump Files\0*.rtpdump\0All Files\0*.*\0\0"));
  int sel = -1;
  bool enable = IsDlgButtonChecked(IDC_CHK_RTP_DUMP_IN) == BST_CHECKED;

  sel = _lst_clients.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if (sel < 0)
    goto l_recover_state;

  int chl = _lst_clients.GetItemData(sel);

  if (chl < 0) goto l_recover_state;

  if (!enable) {
    voe_debug_stop_data_dump(chl, kRtpIncoming);
    return 0;
  }

  if (IDOK != dlg.DoModal()) {
    goto l_recover_state;
  }

  /*SetDlgItemText(IDC_EDT_MIC_FILE, dlg.m_szFileName); */
  int ret = voe_debug_start_data_dump(chl, wchar_to_utf8(dlg.m_szFileName, -1),
                                    kRtpIncoming);

  if (ret) {
    goto l_recover_state;
  }

  return 0;

l_recover_state:
  CheckDlgButton(IDC_CHK_RTP_DUMP_IN,
                 enable ? BST_UNCHECKED : BST_CHECKED);
  return 0;
}

LRESULT CEngMainDlg::OnChkRtpDumpOut(WORD, WORD, HWND, BOOL&) {
  CFileDialog dlg(FALSE, _T("*.rtpdump"), 0,
                  OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                  _T("Rtp dump Files\0*.rtpdump\0All Files\0*.*\0\0"));
  int sel = -1;
  bool enable = IsDlgButtonChecked(IDC_CHK_RTP_DUMP_OUT) == BST_CHECKED;

  sel = _lst_clients.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if (sel < 0)
    goto l_recover_state;

  int chl = _lst_clients.GetItemData(sel);

  if (chl < 0) goto l_recover_state;

  if (!enable) {
    voe_debug_stop_data_dump(chl, kRtpOutgoing);
    return 0;
  }

  if (IDOK != dlg.DoModal()) {
    goto l_recover_state;
  }

  int ret = voe_debug_start_data_dump(chl, wchar_to_utf8(dlg.m_szFileName, -1),
                                    kRtpOutgoing);

  if (ret) {
    goto l_recover_state;
  }

  return 0;

l_recover_state:
  CheckDlgButton(IDC_CHK_RTP_DUMP_OUT,
                 enable ? BST_UNCHECKED : BST_CHECKED);
  return 0;
}

LRESULT CEngMainDlg::OnSelchangeCmbDevInput(WORD, WORD, HWND, BOOL&) {
  CComboBox cmb = GetDlgItem(IDC_CMB_DEV_INPUT);
  int sel = cmb.GetCurSel();
  int idx = -2;

  idx = sel - 2;
 /* MediaEngineVoiceHardware *voice_devices =
    MediaEngineVoiceHardware::GetInterface(s_engine);

  voice_devices->SetRecordingDevice(idx);
  MediaEngineVoiceHardware::Release(voice_devices);*/
  return 0;
}

LRESULT CEngMainDlg::OnSelchangeCmbDevOutput(WORD, WORD, HWND, BOOL&) {
  CComboBox cmb = GetDlgItem(IDC_CMB_DEV_OUTPUT);
  int sel = cmb.GetCurSel();
  int idx = -2;

  idx = sel - 2;
  /*MediaEngineVoiceHardware *voice_devices =
    MediaEngineVoiceHardware::GetInterface(s_engine);

  voice_devices->SetPlayoutDevice(idx);
  MediaEngineVoiceHardware::Release(voice_devices);*/
  return 0;
}

LRESULT CEngMainDlg::OnReleasedcaptureSldInputVolume(int, LPNMHDR, BOOL&) {
  CTrackBarCtrl tracker;
  tracker.m_hWnd = GetDlgItem(IDC_SLD_INPUT_VOLUME);
  int vol = tracker.GetPos();

  voe_set_rec_device_volume(vol);

  return 0;
}

LRESULT CEngMainDlg::OnReleasedcaptureSldOutputVolume(int, LPNMHDR, BOOL&) {
  CTrackBarCtrl tracker;
  tracker.m_hWnd = GetDlgItem(IDC_SLD_OUTPUT_VOLUME);
  int vol = tracker.GetPos();
  voe_set_play_device_volume(vol);  
  return 0;
}

LRESULT CEngMainDlg::OnItemchangedLstChannels(int, LPNMHDR, BOOL&) {
  int sel = _lst_clients.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if (sel < 0) return 0;

  int chl = _lst_clients.GetItemData(sel);

  if (chl < 0) return 0;

  // get current channel info.
  CTrackBarCtrl tracker;
  CWindow ctrl;
  bool enable = false, other_flags;

  // NS mode
  tracker = GetDlgItem(IDC_SLD_MIC_NS);
  ctrl = GetDlgItem(IDC_CHK_NS);
  int ns_mode = 0;
    
  if (0 == voe_get_ns_mode(enable, ns_mode)) {
    CheckDlgButton(IDC_CHK_NS, enable ? BST_CHECKED : BST_UNCHECKED);
    tracker.SetPos(ns_mode);
    tracker.EnableWindow(enable);
  }

  // rx NS mode
  tracker = GetDlgItem(IDC_SLD_RX_NS);
  ctrl = GetDlgItem(IDC_CHECK_RX_NS);

  if (0 == voe_get_rx_ns_mode(chl, enable, ns_mode)) {
    CheckDlgButton(IDC_CHECK_RX_NS, enable ? BST_CHECKED : BST_UNCHECKED);
    tracker.SetPos(ns_mode);
    tracker.EnableWindow(enable);
  }

  // VAD mode
  tracker = GetDlgItem(IDC_SLD_VAD);
  ctrl = GetDlgItem(IDC_CHK_VAD);
  int vad_mode;

  if (0 == voe_channel_get_vad(chl, enable, vad_mode, other_flags)) {
    CheckDlgButton(IDC_CHK_VAD, enable ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(IDC_CHK_DTX, other_flags ? BST_CHECKED : BST_UNCHECKED);
    tracker.SetPos(vad_mode);
    tracker.EnableWindow(enable);
  }

  // EC mode
  tracker = GetDlgItem(IDC_SLD_MIC_EC);
  ctrl = GetDlgItem(IDC_CHK_EC);
  int ec_mode;

  if (0 == voe_get_ec_mode(enable, ec_mode)) {
    CheckDlgButton(IDC_CHK_EC, enable ? BST_CHECKED : BST_UNCHECKED);
    tracker.SetPos(ec_mode);
    tracker.EnableWindow(enable);
  }

  // AGC mode
  tracker = GetDlgItem(IDC_SLD_MIC_AGC);
  ctrl = GetDlgItem(IDC_CHK_AGC);
  int agc_mode;

  if (0 == voe_get_agc_mode(enable, agc_mode)) {
    CheckDlgButton(IDC_CHK_AGC, enable ? BST_CHECKED : BST_UNCHECKED);
    tracker.SetPos(agc_mode);
    tracker.EnableWindow(enable);
  }

  // rx AGC mode
  tracker = GetDlgItem(IDC_SLD_RX_AGC);
  ctrl = GetDlgItem(IDC_CHECK_RX_AGC);

  if (0 == voe_get_rx_agc_mode(chl, enable, agc_mode)) {
    CheckDlgButton(IDC_CHECK_RX_AGC, enable ? BST_CHECKED : BST_UNCHECKED);
    tracker.SetPos(agc_mode);
    tracker.EnableWindow(enable);
  }

  if (0 == voe_channel_send_status(chl, enable)) {
    if (enable)
      SetDlgItemText(IDC_EDT_SEND_STATUS, _T("Sending"));
    else
      SetDlgItemText(IDC_EDT_SEND_STATUS, _T("Idle"));
  } else {
    SetDlgItemText(IDC_EDT_SEND_STATUS, _T("Error"));
  }

  return 0;
}

LRESULT CEngMainDlg::OnChkNS(WORD, WORD, HWND, BOOL&) {
  bool enable = IsDlgButtonChecked(IDC_CHK_NS) == BST_CHECKED;
  CTrackBarCtrl tracker = GetDlgItem(IDC_SLD_MIC_NS);
  int mode = (NsModes) tracker.GetPos();

  if (!enable) {
    if (voe_set_ns_mode(enable)) goto l_recover_state;
  } else {
    if (voe_set_ns_mode(enable, mode)) goto l_recover_state;
  }

  tracker.EnableWindow(enable);
  return 0;

l_recover_state:
  CheckDlgButton(IDC_CHK_NS,
                 enable ? BST_UNCHECKED : BST_CHECKED);
  return 0;
}

LRESULT CEngMainDlg::OnReleasedcaptureSldMicNS(int, LPNMHDR, BOOL&) {
  return 0;
}

LRESULT CEngMainDlg::OnChkEC(WORD, WORD, HWND, BOOL&) {
  bool enable = IsDlgButtonChecked(IDC_CHK_EC) == BST_CHECKED;
  CTrackBarCtrl tracker = GetDlgItem(IDC_SLD_MIC_EC);
  int mode = (EcModes) tracker.GetPos();

  if (!enable) {
    if (voe_set_ec_mode(enable)) goto l_recover_state;
  } else {
    if (voe_set_ec_mode(enable, mode)) goto l_recover_state;
  }

  tracker.EnableWindow(enable);
  return 0;

l_recover_state:
  CheckDlgButton(IDC_CHK_EC,
                 enable ? BST_UNCHECKED : BST_CHECKED);
  return 0;
}

LRESULT CEngMainDlg::OnReleasedcaptureSldMicEC(int, LPNMHDR, BOOL&) {
  return 0;
}

LRESULT CEngMainDlg::OnReleasedcaptureSldVAD(int, LPNMHDR, BOOL&) {
  CTrackBarCtrl tracker;
  tracker.m_hWnd = GetDlgItem(IDC_SLD_VAD);
  /*int pos = tracker.GetPos();
  tracker.SetRangeMin(0);
  tracker.SetRangeMax(255);*/
  return 0;
}

LRESULT CEngMainDlg::OnChkVAD(WORD, WORD, HWND, BOOL&) {
  bool enable = IsDlgButtonChecked(IDC_CHK_VAD) == BST_CHECKED;
  bool disable_dtx = IsDlgButtonChecked(IDC_CHK_DTX) != BST_CHECKED;
  CTrackBarCtrl tracker = GetDlgItem(IDC_SLD_VAD);
  int mode = tracker.GetPos();
  int sel = _lst_clients.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if (sel < 0)
    goto l_recover_state;

  int chl = _lst_clients.GetItemData(sel);

  if (chl < 0) goto l_recover_state;

  if (!enable) {
    if (voe_channel_get_vad(chl, enable, mode, disable_dtx)) goto l_recover_state;
  } else {
    if (voe_channel_set_vad(chl, enable, mode, disable_dtx)) goto l_recover_state;
  }

  tracker.EnableWindow(enable);

  return 0;

l_recover_state:
  CheckDlgButton(IDC_CHK_VAD,
                 enable ? BST_UNCHECKED : BST_CHECKED);
  return 0;
}

LRESULT CEngMainDlg::OnChkPlayFileIn(WORD, WORD, HWND, BOOL&) {
  CFileDialog dlg(TRUE, _T("*.wav"), 0,
                  OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                  _T("Wave Files\0*.wav\0All Files\0*.*\0\0"));
  int sel = -1;
  bool enable = IsDlgButtonChecked(IDC_CHK_CHL_PLAY_FILE_IN) == BST_CHECKED;
  bool mute_mic = IsDlgButtonChecked(IDC_CHK_PLAY_FILE_IN_MUTE_MIC) ==
                  BST_CHECKED;
  sel = _lst_clients.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if (sel < 0)
    goto l_recover_state;

  int chl = _lst_clients.GetItemData(sel);

  if (chl < 0) goto l_recover_state;

  if (!enable) {
    voe_channel_stop_play_file_as_mic(chl);    
    return 0;
  }

  if (IDOK != dlg.DoModal()) {
    goto l_recover_state;
  }

  int ret = voe_channel_start_play_file_as_mic(chl, wchar_to_utf8(dlg.m_szFileName, -1),
                                       true, mute_mic, kFileFormatWavFile);

  if (ret) {
    goto l_recover_state;
  }

  return 0;

l_recover_state:
  CheckDlgButton(IDC_CHK_CHL_PLAY_FILE_IN,
                 enable ? BST_UNCHECKED : BST_CHECKED);
  return 0;
}

LRESULT CEngMainDlg::OnChkChlPlayFileOut(WORD, WORD, HWND, BOOL&) {
  int sel = -1;
  bool enable = IsDlgButtonChecked(IDC_CHK_CHL_PLAY_FILE_OUT) == BST_CHECKED;

  CFileDialog dlg(TRUE, _T("*.wav"), 0,
                  OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
                  _T("Wave Files\0*.wav\0All Files\0*.*\0\0"));

  sel = _lst_clients.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if (sel < 0) {
    goto l_recover_state;
  }

  int chl = _lst_clients.GetItemData(sel);

  if (chl < 0) {
    goto l_recover_state;
  }

  if (!enable) {
    voe_channel_stop_play_file_locally(chl);
    return 0;
  }

  if (IDOK != dlg.DoModal()) {
    goto l_recover_state;
  }

  int ret = voe_channel_start_play_file_locally(chl, wchar_to_utf8(dlg.m_szFileName, -1),
                                         true, kFileFormatWavFile);

  if (ret) {
    goto l_recover_state;
  }

  return 0;

l_recover_state:
  CheckDlgButton(IDC_CHK_CHL_PLAY_FILE_OUT,
                 enable ? BST_UNCHECKED : BST_CHECKED);
  return 0;
}

LRESULT CEngMainDlg::OnBtnCreateChl(WORD, WORD, HWND, BOOL& ) {
  int chl = -1;
  chl = voe_create_channel();
  //chl = s_voe->CreateChannel();

  if (chl < 0) return 0;

  CComboBox cmb = GetDlgItem(IDC_CMB_CODEC);
  int code_sel = cmb.GetCurSel();

  if (code_sel < 0) return 0;

  CodecInst code_inst;
  voe_get_audio_codec_detail(cmb.GetItemData(code_sel), code_inst);    
  if (code_inst.pltype == 114) { // codec amr
    voe_channel_enable_packet_combine(chl, true, 5);    
    voe_channel_enable_packet_separate(chl, true);
  }
  
  TCHAR msg[256] = {0};
  _stprintf(msg, _T("Chl %u"), chl);
  int col = _lst_clients.AddItem(_lst_clients.GetItemCount(), 0, msg);

  if (col < 0) return 0;

  _lst_clients.SetItemData(col, chl);
  if (103 == code_inst.pltype)
    code_inst.pacsize *= 2;
  voe_channel_set_codec(chl, code_inst.pltype);  

  voe_set_ns_mode(true, kNsModerateSuppression);
  voe_channel_set_vad(chl, true, kVadAggressiveMid, false);
  voe_set_ec_mode(true, kEcAecm);
  voe_set_aecm_mode(false, kAecmSpeakerphone);  
  return 0;
}

LRESULT CEngMainDlg::OnBtnDelChl(WORD, WORD, HWND, BOOL& ) {
  int sel = _lst_clients.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if (sel < 0) return 0;

  int chl = _lst_clients.GetItemData(sel);

  if (chl < 0) return 0;

  voe_delete_channel(chl);
  _lst_clients.DeleteItem(sel);
  return 0;
}

LRESULT CEngMainDlg::OnBtnChlStartSend(WORD, WORD, HWND, BOOL& ) {
  int sel = _lst_clients.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if (sel < 0) return 0;

  int chl = _lst_clients.GetItemData(sel);

  if (chl < 0) return 0;

  voe_channel_start_send(chl);  

#ifdef ARM_DEBUG
  char file_path[512] = {0};
  sprintf(file_path, "./%lu.awb", time(0));
  s_out_enc_file = fopen(file_path, "wb");
  if (s_out_enc_file)
    fwrite(AMRWB_MAGIC_NUMBER, sizeof(char), strlen(AMRWB_MAGIC_NUMBER), s_out_enc_file);
#endif // ARM_DEBUG
  
  /* s_voe_file->StartPlayAsMic(chl,
  "d:/7f Game/Projects/MediaService/VoiceEngine/resources/voice_engine/audio_long16.wav",
  true, false, kFileFormatWavFile);    */

  return 0;
}

LRESULT CEngMainDlg::OnBtnChlStopSend(WORD, WORD, HWND, BOOL& ) {
  int sel = _lst_clients.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if (sel < 0) return 0;

  int chl = _lst_clients.GetItemData(sel);

  if (chl < 0) return 0;

  voe_channel_stop_send(chl);  

#ifdef ARM_DEBUG
  if (s_out_enc_file) {
    fclose(s_out_enc_file);
    s_out_enc_file = 0;
  }
#endif // ARM_DEBUG
  return 0;
}

LRESULT CEngMainDlg::OnBtnChlStartReceive(WORD, WORD, HWND, BOOL& ) {
  int sel = _lst_clients.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if (sel < 0) return 0;

  int chl = _lst_clients.GetItemData(sel);

  if (chl < 0) return 0;

  voe_channel_start_receive(chl);  
  return 0;
}

LRESULT CEngMainDlg::OnBtnChlStopReceive(WORD, WORD, HWND, BOOL& ) {
  int sel = _lst_clients.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if (sel < 0) return 0;

  int chl = _lst_clients.GetItemData(sel);

  if (chl < 0) return 0;

  voe_channel_stop_receive(chl);  
  return 0;
}

LRESULT CEngMainDlg::OnChlStartPlayout(WORD, WORD, HWND, BOOL& ) {
  int sel = _lst_clients.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if (sel < 0) return 0;

  int chl = _lst_clients.GetItemData(sel);

  if (chl < 0) return 0;

  voe_channel_start_playout(chl);
  return 0;
}

LRESULT CEngMainDlg::OnBtnChlStopPlayout(WORD, WORD, HWND, BOOL& ) {
  int sel = _lst_clients.GetNextItem(-1, LVNI_ALL | LVNI_SELECTED);

  if (sel < 0) return 0;

  int chl = _lst_clients.GetItemData(sel);

  if (chl < 0) return 0;

  voe_channel_stop_playout(chl);  
  
  return 0;
}
