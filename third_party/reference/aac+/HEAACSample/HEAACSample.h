// HEAACSample.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CHEAACSampleApp:
// �йش����ʵ�֣������ HEAACSample.cpp
//

class CHEAACSampleApp : public CWinApp
{
public:
	CHEAACSampleApp();

// ��д
	public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CHEAACSampleApp theApp;