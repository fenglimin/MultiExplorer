#pragma once

#include "..\ShellTree\ShellTree.h"
// CDirTreeDialog dialog
class CDiskFileManager;

class CDirTreeDialog : public CDialog
{
	DECLARE_DYNAMIC(CDirTreeDialog)

public:
	CDirTreeDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDirTreeDialog();

	CDiskFileManager*	m_pDiskFileManager;
	CString	m_strCurDir;

	CShellTree*	m_pDirTree;

	BOOL m_bMouseClicked;

	HTREEITEM m_hClickedItem;

// Dialog Data
	enum { IDD = IDD_DIALOG_DIR_TREE };

	void SetDiskFileManager(CDiskFileManager* pDiskFileManager) { m_pDiskFileManager = pDiskFileManager; }
	void SetPosition( const CRect& rcWnd );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnDeleteitemTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemexpandingTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRclickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSelchangedTreeDir(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
	
public:
	afx_msg void OnNMClickTreeDir(NMHDR *pNMHDR, LRESULT *pResult);
};
