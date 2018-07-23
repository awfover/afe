#pragma once


// KeyDlg dialog

#include <string>

#include "stdafx.h"
#include "afxwin.h"
#include "afxbutton.h"

class KeyDlg : public CDialogEx
{
	DECLARE_DYNAMIC(KeyDlg)

public:
	KeyDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~KeyDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_KEY_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CEdit m_Key;
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	std::wstring key;
	CButton m_Toggle;
	afx_msg void OnBnClickedToggle();
};
