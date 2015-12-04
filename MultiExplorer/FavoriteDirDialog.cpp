// FavoriteDirDialog.cpp : implementation file
//

#include "stdafx.h"
#include "MultiExplorer.h"
#include "FavoriteDirDialog.h"
#include "afxdialogex.h"

#include "DiskFileManager.h"
#include "AddToFavoriteDialog.h"
// CFavoriteDirDialog dialog

IMPLEMENT_DYNAMIC(CFavoriteDirDialog, CDialogEx)

CFavoriteDirDialog::CFavoriteDirDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFavoriteDirDialog::IDD, pParent)
{

}

CFavoriteDirDialog::~CFavoriteDirDialog()
{

}

void CFavoriteDirDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CFavoriteDirDialog, CDialogEx)
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR_FAVIOURITE, &CFavoriteDirDialog::OnNMClickTreeDirFavorite)
	ON_NOTIFY(TVN_GETINFOTIP, IDC_TREE_DIR_FAVIOURITE, &CFavoriteDirDialog::OnTvnGetInfoTipTreeDirFavorite)
	ON_WM_DESTROY()
	ON_NOTIFY(NM_RCLICK, IDC_TREE_DIR_FAVIOURITE, &CFavoriteDirDialog::OnNMRClickTreeDirFaviourite)
END_MESSAGE_MAP()


// CFavoriteDirDialog message handlers
BOOL CFavoriteDirDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_pDirTree = (CTreeCtrl*) GetDlgItem ( IDC_TREE_DIR_FAVIOURITE );
	m_hRootItem = m_pDirTree->InsertItem(_M("Favorite Directory"));

	return TRUE;
}

void CFavoriteDirDialog::SetPosition( const CRect& rcWnd )
{
	SetWindowPos(NULL, rcWnd.left, rcWnd.top, rcWnd.Width(), rcWnd.Height(), SWP_SHOWWINDOW);

	CRect rcClient;
	GetWindowRect(&rcClient);
	ScreenToClient(rcClient);

	rcClient.InflateRect(-2,-2,-2,-2);
	m_pDirTree->MoveWindow(rcClient, TRUE);
}

HTREEITEM CFavoriteDirDialog::GetHitItem()
{
	POINT			pt;
	TV_HITTESTINFO	tvhti;

	::GetCursorPos((LPPOINT)&pt);
	ScreenToClient(&pt);
	tvhti.pt=pt;
	m_pDirTree->HitTest(&tvhti);

	return tvhti.hItem;
}

void CFavoriteDirDialog::OnNMClickTreeDirFavorite(NMHDR *pNMHDR, LRESULT *pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;	
	*pResult = 0;

	HTREEITEM hHitItem = GetHitItem();
	if (IsDirItem(hHitItem))
	{
		ShowDir(hHitItem);
	}	
}

void CFavoriteDirDialog::OnTvnGetInfoTipTreeDirFavorite(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTVGETINFOTIP pGetInfoTip = reinterpret_cast<LPNMTVGETINFOTIP>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	pGetInfoTip->pszText = "aaaaaaaaaaa";
}

void CFavoriteDirDialog::AddFavoriteDir( CString strGroup, CString strName, CString strDir )
{
	HTREEITEM hGroupItem = FindGroupItem(strGroup);
	if (hGroupItem == NULL)
	{
		hGroupItem = m_pDirTree->InsertItem(strGroup, m_hRootItem);
	}

	HTREEITEM hDir = m_pDirTree->InsertItem(strName, hGroupItem);

	CString* pTemp = new CString;
	*pTemp = strDir;
	m_pDirTree->SetItemData(hDir, (DWORD_PTR)pTemp);
}

HTREEITEM CFavoriteDirDialog::FindGroupItem( CString strGroup )
{
	HTREEITEM hChild = m_pDirTree->GetChildItem(m_hRootItem);
	while ( hChild != NULL && m_pDirTree->GetItemText(hChild) != strGroup)
	{
		hChild = m_pDirTree->GetNextSiblingItem(hChild);
	}

	return hChild;
}


void CFavoriteDirDialog::OnDestroy()
{
	HTREEITEM hChild = m_pDirTree->GetChildItem(m_hRootItem);
	while ( hChild != NULL)
	{
		HTREEITEM hGrandChild = m_pDirTree->GetChildItem(hChild);
		while (hGrandChild != NULL)
		{
			CString* pData = (CString*) m_pDirTree->GetItemData(hGrandChild);
			delete pData;

			hGrandChild = m_pDirTree->GetNextSiblingItem(hGrandChild);
		}

		hChild = m_pDirTree->GetNextSiblingItem(hChild);
	}

	CDialogEx::OnDestroy();

}

list<CString> CFavoriteDirDialog::GetGroupList()
{
	list<CString> listGroup;

	HTREEITEM hChild = m_pDirTree->GetChildItem(m_hRootItem);
	while ( hChild != NULL)
	{
		listGroup.push_back(m_pDirTree->GetItemText(hChild));
		hChild = m_pDirTree->GetNextSiblingItem(hChild);
	}

	return listGroup;
}

BOOL CFavoriteDirDialog::IsGroupItem(HTREEITEM hItem)
{
	if (hItem == NULL)
		return FALSE;

	HTREEITEM hParent = m_pDirTree->GetParentItem(hItem);
	if ( hParent != NULL && m_pDirTree->GetItemText(hParent) == _M("Favorite Directory"))
		return TRUE;

	return FALSE;
}

BOOL CFavoriteDirDialog::IsDirItem(HTREEITEM hItem)
{
	if (hItem == NULL)
		return FALSE;

	HTREEITEM hParent = m_pDirTree->GetParentItem(hItem);
	return IsGroupItem(hParent);
}

void CFavoriteDirDialog::OnNMRClickTreeDirFaviourite(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;

	HTREEITEM hHitItem = GetHitItem();
	BOOL bIsGroup = IsGroupItem(hHitItem);
	BOOL bIsDir = IsDirItem(hHitItem);

	if (!bIsDir && !bIsGroup)
		return;

	POINT			pt;
	::GetCursorPos((LPPOINT)&pt);	
	
	CMenu menu;
	menu.CreatePopupMenu();

	CString strMenuText, strGroup, strName;
	if (bIsGroup)
	{
		strGroup = m_pDirTree->GetItemText(hHitItem);

		strMenuText.Format(_M("Delete Group [%s]"), strGroup);
		menu.AppendMenu(MF_STRING|MF_ENABLED, 1, strMenuText );

		strMenuText.Format(_M("Rename Group [%s]"), strGroup);
		menu.AppendMenu(MF_STRING|MF_ENABLED, 2, strMenuText );
	}
	else
	{
		strName = m_pDirTree->GetItemText(hHitItem);
		CString* pDir = (CString*) m_pDirTree->GetItemData(hHitItem);

		strMenuText.Format(_M("Open Dir [%s - %s]"), strName, *pDir);
		menu.AppendMenu(MF_STRING|MF_ENABLED, 3, strMenuText );

		strMenuText.Format(_M("Delete Dir [%s]"), strName);
		menu.AppendMenu(MF_STRING|MF_ENABLED, 4, strMenuText );

		strMenuText.Format(_M("Rename Dir [%s]"), strName);
		menu.AppendMenu(MF_STRING|MF_ENABLED, 5, strMenuText );
	}


	int nResult = menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);
	switch (nResult)
	{
	case 1:	DeleteNode(_M("Group"), hHitItem);
			break;
	case 2:	RenameNode(hHitItem, TRUE, strGroup, FALSE, strName);
			break;
	case 3:	ShowDir(hHitItem);
			break;
	case 4:	DeleteNode(_M("Dir"), hHitItem);
			break;
	case 5:	RenameNode(hHitItem, FALSE, strGroup, TRUE, strName);
			break;
	}

}

void CFavoriteDirDialog::DeleteNode(CString strHint, HTREEITEM hItem )
{
	CString strMsg;
	strMsg.Format(_M("Are you sure to delete this %s?"), strHint);
	
	if (AfxMessageBox(strMsg, MB_YESNO) != IDYES)
		return;

	m_pDirTree->DeleteItem(hItem);
}

void CFavoriteDirDialog::RenameNode(HTREEITEM hHitItem, BOOL bRenameGroup, CString strGroup, BOOL bRenameDir, CString strDir )
{
	CAddToFavoriteDialog atfd;
	atfd.m_bRenameGroup = bRenameGroup;
	atfd.m_strGroup = strGroup;
	atfd.m_bRenameDir = bRenameDir;
	atfd.m_strName = strDir;
	atfd.m_listGroup = GetGroupList();

	if (atfd.DoModal() != IDOK)
		return;

	CString strNewText = bRenameGroup? atfd.m_strGroup : atfd.m_strName;
	m_pDirTree->SetItemText(hHitItem,strNewText);
}

void CFavoriteDirDialog::ShowDir( HTREEITEM hHitItem )
{
	CString* pDir = (CString*) m_pDirTree->GetItemData(hHitItem);
	m_pDiskFileManager->ShowDirInActiveFileExplorer(*pDir);
}
