#pragma once

#include <list>
// CFavoriteDirDialog dialog
class CDiskFileManager;

class CFavoriteDirDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CFavoriteDirDialog)

public:
	CFavoriteDirDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFavoriteDirDialog();

	CDiskFileManager*	m_pDiskFileManager;
	CTreeCtrl*	m_pDirTree;
	HTREEITEM	m_hRootItem;
// Dialog Data
	enum { IDD = IDD_DIALOG_DIR_FAVIOURITE };

	void SetDiskFileManager(CDiskFileManager* pDiskFileManager) { m_pDiskFileManager = pDiskFileManager; }
	void SetPosition( const CRect& rcWnd );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnNMClickTreeDirFavorite(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnGetInfoTipTreeDirFavorite(NMHDR *pNMHDR, LRESULT *pResult);
	void AddFavoriteDir(CString strGroup,  CString strName, CString strDir);
	HTREEITEM FindGroupItem( CString strGroup );
	afx_msg void OnDestroy();
	list<CString> GetGroupList();
	afx_msg void OnNMRClickTreeDirFaviourite(NMHDR *pNMHDR, LRESULT *pResult);
	HTREEITEM GetHitItem();
	BOOL IsGroupItem(HTREEITEM hItem);
	BOOL IsDirItem(HTREEITEM hItem);
	void DeleteNode(CString strHint, HTREEITEM hItem );
	void RenameNode(HTREEITEM hHitItem, BOOL bRenameGroup, CString strGroup, BOOL bRenameDir, CString strDir );
	void ShowDir( HTREEITEM hHitItem );
};
