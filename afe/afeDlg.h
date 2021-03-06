
// afeDlg.h : header file
//

#pragma once
#include <string>

#include "stdafx.h"
#include "afxwin.h"
#include "afxcmn.h"

#include "KeyDlg.h"

#include "FileEncrypter.h"
#include "afxeditbrowsectrl.h"

#define WM_START_WORK WM_USER + 1000

enum AfeWorkType {
	Encryption,
	Decryption,
};

// CafeDlg dialog
class CafeDlg : public CDialog
{
// Construction
public:
	CafeDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_AFE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	bool CheckParameters() {
		if (m_FileSelector.GetWindowTextLength() == 0) {
			AfxMessageBox(L"Please select a file to encrypt (or decrypt).");
			return false;
		}

		KeyDlg dlg;
		if (dlg.DoModal() != IDOK) {
			return false;
		}
		
		m_Key = dlg.key;
		if (m_Key.size() == 0) {
			AfxMessageBox(L"Please enter a key to encrypt (or decrypt).");
			return false;
		}

		return true;
	};

	static UINT EncryptFile(LPVOID lParam);
	static UINT DecryptFile(LPVOID lParam);

public:
	afx_msg void OnDropFiles(HDROP hDropInfo);
	std::wstring m_Key;
	CMFCEditBrowseCtrl m_FileSelector;
	CEdit m_Log;
	CProgressCtrl m_Progress;
	CMFCButton m_Encrypt;
	CMFCButton m_Decrypt;
	afx_msg void OnBnClickedEncrypt();
	afx_msg void OnBnClickedDecrypt();
	CMFCEditBrowseCtrl m_DirSelector;
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
