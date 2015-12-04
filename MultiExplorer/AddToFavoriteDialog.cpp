// AddToFavoriteDialog.cpp : implementation file
//

#include "stdafx.h"
#include "MultiExplorer.h"
#include "AddToFavoriteDialog.h"
#include "afxdialogex.h"


// CAddToFavoriteDialog dialog

IMPLEMENT_DYNAMIC(CAddToFavoriteDialog, CDialogEx)

CAddToFavoriteDialog::CAddToFavoriteDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAddToFavoriteDialog::IDD, pParent)
	, m_strName(_T(""))
	, m_strGroup(_T(""))
{
	m_bRenameDir = FALSE;
	m_bRenameGroup = FALSE;
}

CAddToFavoriteDialog::~CAddToFavoriteDialog()
{
}

void CAddToFavoriteDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_GROUP, m_comboGroup);
	DDX_Text(pDX, IDC_EDIT_NAME, m_strName);
	DDX_CBString(pDX, IDC_COMBO_GROUP, m_strGroup);
}


BEGIN_MESSAGE_MAP(CAddToFavoriteDialog, CDialogEx)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, &CAddToFavoriteDialog::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CAddToFavoriteDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetUIText();

	for ( list<CString>::iterator it = m_listGroup.begin(); it != m_listGroup.end(); it++)
	{
		m_comboGroup.AddString(*it);
	}

	int nSel = 0;
	if (m_bRenameGroup || m_bRenameDir)
	{
		m_comboGroup.EnableWindow(m_bRenameGroup);
		GetDlgItem(IDC_EDIT_NAME)->EnableWindow(m_bRenameDir);

		nSel = m_comboGroup.FindString(0, m_strGroup);
	}


	m_comboGroup.SetCurSel(nSel);

	return TRUE;
}

void CAddToFavoriteDialog::SetUIText()
{
	SetDlgItemText(IDOK, _M("OK"));
	SetDlgItemText(IDCANCEL, _M("Cancel"));
	SetDlgItemText(IDC_STATIC_GROUP, _M("Group"));
	SetDlgItemText(IDC_STATIC_NAME, _M("Name"));

	SetWindowText(_M("Add to favorite"));
}

// CAddToFavoriteDialog message handlers


void CAddToFavoriteDialog::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: Add your message handler code here
}


void CAddToFavoriteDialog::OnBnClickedOk()
{
	UpdateData(TRUE);

	CDialogEx::OnOK();
}
