// KeyDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afe.h"
#include "KeyDlg.h"
#include "afxdialogex.h"


// KeyDlg dialog

IMPLEMENT_DYNAMIC(KeyDlg, CDialogEx)

KeyDlg::KeyDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_KEY_DIALOG, pParent)
{

}

KeyDlg::~KeyDlg()
{
}

void KeyDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_KEY, m_Key);
	//  DDX_Control(pDX, IDC_TOGGLE, m_Toggle);
	DDX_Control(pDX, IDC_TOGGLE, m_Toggle);
}


BEGIN_MESSAGE_MAP(KeyDlg, CDialogEx)
	ON_BN_CLICKED(IDC_TOGGLE, &KeyDlg::OnBnClickedToggle)
END_MESSAGE_MAP()


// KeyDlg message handlers


BOOL KeyDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN) {
		TCHAR buffer[MAX_PATH];
		m_Key.GetWindowText(buffer, MAX_PATH);
		key = buffer;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void KeyDlg::OnBnClickedToggle()
{
	// TODO: Add your control notification handler code here
	if (m_Toggle.GetCheck() == BST_CHECKED) {
		m_Key.SetPasswordChar(0);
	}
	else {
		m_Key.SetPasswordChar('*');
	}
	m_Key.SetFocus();
}
