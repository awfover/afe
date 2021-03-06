
// afeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "afe.h"
#include "afeDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CafeDlg dialog



CafeDlg::CafeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_AFE_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CafeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FILE_SELECTOR, m_FileSelector);
	DDX_Control(pDX, IDC_LOG, m_Log);
	DDX_Control(pDX, IDC_PROGRESS, m_Progress);
	DDX_Control(pDX, IDC_ENCRYPT, m_Encrypt);
	DDX_Control(pDX, IDC_DECRYPT, m_Decrypt);
	DDX_Control(pDX, IDC_DIR_SELECTOR, m_DirSelector);
}

BEGIN_MESSAGE_MAP(CafeDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_ENCRYPT, &CafeDlg::OnBnClickedEncrypt)
	ON_BN_CLICKED(IDC_DECRYPT, &CafeDlg::OnBnClickedDecrypt)
END_MESSAGE_MAP()


// CafeDlg message handlers

BOOL CafeDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	m_DirSelector.EnableFolderBrowseButton();

	TCHAR buffer[MAX_PATH];
	if (GetCurrentDirectory(MAX_PATH, buffer)) {
		m_DirSelector.SetWindowText(CString(buffer));
	}

	m_Progress.SetRange(0, 1000);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CafeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CafeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CafeDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: Add your message handler code here and/or call default
	TCHAR buffer[MAX_PATH];
	if (DragQueryFile(hDropInfo, 0, buffer, MAX_PATH)) {
		WIN32_FILE_ATTRIBUTE_DATA data;
		if (GetFileAttributesEx(buffer, GetFileExInfoStandard, &data) > 0) {
			if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
				m_DirSelector.SetWindowText(CString(buffer));
			}
			else {
				m_FileSelector.SetWindowText(CString(buffer));
			}
		}
	}

	CDialog::OnDropFiles(hDropInfo);
}

void CafeDlg::OnBnClickedEncrypt()
{
	// TODO: Add your control notification handler code here
	if (!CheckParameters()) {
		return;
	}

	AfxBeginThread(CafeDlg::EncryptFile, this);
}

void CafeDlg::OnBnClickedDecrypt()
{
	// TODO: Add your control notification handler code here
	if (!CheckParameters()) {
		return;
	}

	AfxBeginThread(CafeDlg::DecryptFile, this);
}

UINT CafeDlg::EncryptFile(LPVOID lParam) {
	CafeDlg *pDlg = (CafeDlg *)lParam;
	pDlg->SendMessage(WM_START_WORK, (WPARAM)AfeWorkType::Encryption, 0);

	TCHAR in_buffer[MAX_PATH];
	pDlg->m_FileSelector.GetWindowText(in_buffer, MAX_PATH);

	TCHAR out_buffer[MAX_PATH];
	pDlg->m_DirSelector.GetWindowText(out_buffer, MAX_PATH);

	FileEncrypter fe(pDlg->GetSafeHwnd(), pDlg->m_Key);
	fe.Encrypt(out_buffer, in_buffer);
	
	return 0;
}

UINT CafeDlg::DecryptFile(LPVOID lParam) {
	CafeDlg *pDlg = (CafeDlg *)lParam;
	pDlg->SendMessage(WM_START_WORK, (WPARAM)AfeWorkType::Decryption, 0);

	TCHAR in_buffer[MAX_PATH];
	pDlg->m_FileSelector.GetWindowText(in_buffer, MAX_PATH);

	TCHAR out_buffer[MAX_PATH];
	pDlg->m_DirSelector.GetWindowText(out_buffer, MAX_PATH);

	FileEncrypter fe(pDlg->GetSafeHwnd(), pDlg->m_Key);
	fe.Decrypt(out_buffer, in_buffer);

	return 0;
}

LRESULT CafeDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	// TODO: Add your specialized code here and/or call the base class
	switch (message) {
	case WM_START_WORK: {
		m_FileSelector.EnableWindow(false);
		m_DirSelector.EnableWindow(false);

		m_Encrypt.EnableWindow(false);
		m_Decrypt.EnableWindow(false);

		m_Progress.SetPos(0);

		CString log;
		m_Log.GetWindowText(log);
		if (wParam == AfeWorkType::Encryption) {
			log += L"Start encryption...\r\n";
		}
		else {
			log += L"Start decryption...\r\n";
		}

		CString file;
		m_FileSelector.GetWindowText(file);
		log += L"File path: " + file + L"\r\n";

		m_Log.SetWindowText(log);
		m_Log.LineScroll(m_Log.GetLineCount(), 0);

		break;
	}

	case WM_FILE_ENCRYPTER_MSG: {
		auto m = (FileEncrypter::Message *)lParam;
		switch (m->type) {
		case FileEncrypter::Message::Type::Error:
		case FileEncrypter::Message::Type::Success: {
			CString log;
			m_Log.GetWindowText(log);
			
			log += m->msg.c_str();
			log += L"\r\n\r\n";
			m_Log.SetWindowText(log);
			m_Log.LineScroll(m_Log.GetLineCount(), 0);

			m_FileSelector.EnableWindow(true);
			m_DirSelector.EnableWindow(true);

			m_Encrypt.EnableWindow(true);
			m_Decrypt.EnableWindow(true);

			break;
		}

		case FileEncrypter::Message::Type::Progress: {
			auto pm = (FileEncrypter::ProgressMessage *)lParam;
			m_Progress.SetPos((size_t)ceil(pm->progress * 1000));

			break;
		}
		}

		break;
	}
	}

	return CDialog::WindowProc(message, wParam, lParam);
}

BOOL CafeDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: Add your specialized code here and/or call the base class
	if ((pMsg->message == WM_KEYDOWN) || (pMsg->message == WM_KEYUP)) {
		if (pMsg->wParam == VK_RETURN) {
			return TRUE;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}
