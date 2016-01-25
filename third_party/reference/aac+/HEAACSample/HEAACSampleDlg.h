// HEAACSampleDlg.h : 头文件
//

#pragma once
#include "afxwin.h"

#include "aac_thread.h"


// CHEAACSampleDlg 对话框
class CHEAACSampleDlg : public CDialog
{
// 构造
public:
	CHEAACSampleDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_HEAACSAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	RecordThread m_record_thr;
	PlayThread   m_play_thr;

	BOOL		 m_record;
	BOOL		 m_play;

	// 生成的消息映射函数
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
