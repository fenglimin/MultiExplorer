#pragma once

#include <list>
#include "afxwin.h"
// CAddToFavoriteDialog dialog

class CAddToFavoriteDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CAddToFavoriteDialog)

public:
	CAddToFavoriteDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddToFavoriteDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_ADD_TO_FAVORITE };

	list<CString>	m_listGroup;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	void SetUIText();
public:
	afx_msg void OnDestroy();
	CComboBox m_comboGroup;
	afx_msg void OnBnClickedOk();
	CString m_strName;
	CString m_strGroup;

	BOOL	m_bRenameGroup;
	BOOL	m_bRenameDir;
};
