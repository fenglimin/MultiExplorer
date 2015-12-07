#pragma once

#include "..\CustomListCtrl\CustomListCtrl.h"

// NetworkOperationDialog dialog
class CDiskFileManager;

class CNetworkOperationDialog : public CDialog, public ICustomListUser
{
	DECLARE_DYNAMIC(CNetworkOperationDialog)

public:
	CNetworkOperationDialog(CDiskFileManager* pDiskFileManager, CWnd* pParent = NULL);   // standard constructor
	virtual ~CNetworkOperationDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_NETWORK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnRowLDblClicked(CListCtrl* pListCtrl, int nRow, int nCol, UINT nFlags, CPoint point);

	CCustomListCtrl m_listMachine;
	CDiskFileManager* m_pDiskFileManager;

	void SetUIText();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonGetData();
};
