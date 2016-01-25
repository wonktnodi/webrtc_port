// HEAACSampleDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HEAACSample.h"
#include "HEAACSampleDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CHEAACSampleDlg 对话框




CHEAACSampleDlg::CHEAACSampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHEAACSampleDlg::IDD, pParent)
	, m_strInfo(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHEAACSampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RECORD, m_btnRecord);
	DDX_Control(pDX, IDC_PLAY, m_btnPlay);
	DDX_Text(pDX, IDC_INFO, m_strInfo);
}

BEGIN_MESSAGE_MAP(CHEAACSampleDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_RECORD, &CHEAACSampleDlg::OnBnClickedRecord)
	ON_BN_CLICKED(IDC_PLAY, &CHEAACSampleDlg::OnBnClickedPlay)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CHEAACSampleDlg 消息处理程序

BOOL CHEAACSampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_record = FALSE;
	m_play = FALSE;

	m_btnPlay.EnableWindow(FALSE);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CHEAACSampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CHEAACSampleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CHEAACSampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CHEAACSampleDlg::OnBnClickedRecord()
{
	if(!m_record)
	{
		m_record = TRUE;
		m_btnRecord.SetWindowText(_T("停止录音"));
		m_record_thr.start();

		m_btnPlay.EnableWindow(FALSE);

		SetTimer(1000, 1000, NULL);
	}
	else
	{
		KillTimer(1000);

		m_record_thr.stop();

		m_record = FALSE;
		m_btnRecord.SetWindowText(_T("开始录音"));
		
		m_btnPlay.EnableWindow(TRUE);
	}
}

void CHEAACSampleDlg::OnBnClickedPlay()
{
	// TODO: Add your control notification handler code here
	if(!m_play)
	{
		m_play = TRUE;
		m_btnPlay.SetWindowText(_T("停止播放"));

		m_play_thr.start();
		m_btnRecord.EnableWindow(FALSE);
	}
	else
	{
		m_play_thr.stop();
		m_play = FALSE;
		
		m_btnPlay.SetWindowText(_T("开始播放"));
		m_btnRecord.EnableWindow(TRUE);
	}
}

void CHEAACSampleDlg::OnTimer(UINT_PTR nIDEvent)
{
	if(nIDEvent == 1000)
	{
		char bandwidth_str[100] = {0};
		int bandwidth = 0, packet_count = 0;
		m_record_thr.get_bandwidth(packet_count, bandwidth);
		sprintf(bandwidth_str, "%d B/S, packets = %d", bandwidth, packet_count);
		m_strInfo = bandwidth_str;
		UpdateData(FALSE);
	}

	CDialog::OnTimer(nIDEvent);
}
