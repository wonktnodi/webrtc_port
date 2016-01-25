// HEAACSampleDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"

#include "aac_thread.h"


// CHEAACSampleDlg �Ի���
class CHEAACSampleDlg : public CDialog
{
// ����
public:
	CHEAACSampleDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_HEAACSAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	RecordThread m_record_thr;
	PlayThread   m_play_thr;

	BOOL		 m_record;
	BOOL		 m_play;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CButton m_btnRecord;
	CButton m_btnPlay;
	CString m_strInfo;
	afx_msg void OnBnClickedRecord();
	afx_msg void OnBnClickedPlay();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
