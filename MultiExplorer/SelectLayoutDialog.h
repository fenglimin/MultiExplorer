#pragma once

#include "..\CustomListCtrl\CustomListCtrl.h"
#include "resource.h"
// CSelectLayoutDialog dialog

class CSelectLayoutDialog : public CDialog, public ICustomListUser
{
	DECLARE_DYNAMIC(CSelectLayoutDialog)

public:
	CSelectLayoutDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSelectLayoutDialog();

	void CheckClose();
	void InitList();
	void AddLayoutButton(int nRow, int nCol, HBITMAP hBitmap);
// Dialog Data
	enum { IDD = IDD_DIALOG_LAYOUT };

	BOOL m_bMustSelect;
	int	m_nCellWidth;
	int m_nCellHeight;

	int m_nIndex;

	vector<HBITMAP> m_vecBitmap;

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	virtual BOOL OnCellCtrlClicked(CListCtrl* pListCtrl, int nRow, int nCol, CellFormat* pCellFormat);

	CCustomListCtrl	m_listLayout;



	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClose();
	afx_msg void OnCancel();
};
