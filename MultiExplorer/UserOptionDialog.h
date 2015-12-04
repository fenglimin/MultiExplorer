#pragma once
#include "afxcmn.h"
#include "..\CustomListCtrl\CustomListCtrl.h"
#include "MultiExplorerDef.h"
#include "resource.h"

// CUserOptionDialog dialog

class CUserOptionDialog : public CDialog, public ICustomListUser
{
	DECLARE_DYNAMIC(CUserOptionDialog)

public:
	CUserOptionDialog(UserOptions* pUserOption, CWnd* pParent = NULL);   // standard constructor
	virtual ~CUserOptionDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_USER_OPTION };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:

	void	SetUIText(); 
	virtual BOOL OnCellComboDisplayed(CListCtrl* pListCtrl, int nRow, int nCol, CellFormat* pCellFormat, CStringList& strComboItem);
	virtual BOOL OnCellCtrlClicked(CListCtrl* pListCtrl, int nRow, int nCol, CellFormat* pCellFormat);
	CCustomListCtrl m_listUserOption;
	UserOptions*	m_pUserOption;
	afx_msg void OnBnClickedOk();
};
