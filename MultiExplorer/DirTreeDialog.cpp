// DirTreeDialog.cpp : implementation file
//

#include "stdafx.h"
#include "MultiExplorer.h"
#include "DirTreeDialog.h"

#include "DiskFileManager.h"

// CDirTreeDialog dialog

IMPLEMENT_DYNAMIC(CDirTreeDialog, CDialog)

CDirTreeDialog::CDirTreeDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CDirTreeDialog::IDD, pParent)
{
	m_hClickedItem = NULL;
	m_bMouseClicked = FALSE;
}

CDirTreeDialog::~CDirTreeDialog()
{
}

void CDirTreeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDirTreeDialog, CDialog)
	ON_NOTIFY(TVN_DELETEITEM, IDC_TREE_DIR, OnDeleteitemTreeDir)
	ON_NOTIFY(TVN_ITEMEXPANDING, IDC_TREE_DIR, OnItemexpandingTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_TREE_DIR, OnRclickTreeDir)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_DIR, OnSelchangedTreeDir)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CDirTreeDialog::OnNMClickTreeDir)
END_MESSAGE_MAP()


// CDirTreeDialog message handlers
void CDirTreeDialog::SetPosition( const CRect& rcWnd )
{
	SetWindowPos(NULL, rcWnd.left, rcWnd.top, rcWnd.Width(), rcWnd.Height(), SWP_SHOWWINDOW);

	CRect rcClient;
	GetWindowRect(&rcClient);
	ScreenToClient(rcClient);

	rcClient.InflateRect(-2,-2,-2,-2);
	m_pDirTree->MoveWindow(rcClient, TRUE);
}

BOOL CDirTreeDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_pDirTree = ( CShellTree* ) GetDlgItem ( IDC_TREE_DIR );
	m_pDirTree->EnableImages();
	m_pDirTree->PopulateTree();

	return TRUE;
}

void CDirTreeDialog::OnDeleteitemTreeDir(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	m_pDirTree->OnDeleteShellItem(pNMHDR,pResult);

	*pResult = 0;
}

void CDirTreeDialog::OnItemexpandingTreeDir(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	m_pDirTree->OnFolderExpanding(pNMHDR,pResult);

	*pResult = 0;
}

void CDirTreeDialog::OnRclickTreeDir(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_pDirTree->GetContextMenu(pNMHDR,pResult);

	*pResult = 0;
}

void CDirTreeDialog::OnSelchangedTreeDir(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	CString szPath;
	if(m_pDirTree->OnFolderSelected(pNMHDR,pResult,szPath))
	{
		m_strCurDir = szPath;
		if ( m_strCurDir.GetAt(m_strCurDir.GetLength()-1) != _T('\\') )
			m_strCurDir += _T("\\");

		if ( m_bMouseClicked )
			m_pDiskFileManager->ShowDirInActiveFileExplorer(m_strCurDir);
		m_bMouseClicked = FALSE;
	}
	else
		m_strCurDir = "";

	*pResult = 0;
}

void CDirTreeDialog::OnNMClickTreeDir(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	m_hClickedItem = pNMTreeView->itemNew.hItem;

	CString straa = m_pDirTree->GetItemText(m_hClickedItem);

	m_bMouseClicked = TRUE;

	*pResult = 0;
}
