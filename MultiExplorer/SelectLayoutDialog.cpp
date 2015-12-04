// SelectLayoutDialog.cpp : implementation file
//

#include "stdafx.h"
#include "MultiExplorer.h"
#include "SelectLayoutDialog.h"


// CSelectLayoutDialog dialog

IMPLEMENT_DYNAMIC(CSelectLayoutDialog, CDialog)

CSelectLayoutDialog::CSelectLayoutDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSelectLayoutDialog::IDD, pParent)
{

}

CSelectLayoutDialog::~CSelectLayoutDialog()
{
}

void CSelectLayoutDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LAYOUT, m_listLayout);
}


BEGIN_MESSAGE_MAP(CSelectLayoutDialog, CDialog)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


BOOL CSelectLayoutDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitList();

	SetWindowText(_M("Change Layout"));
	return TRUE;
}

void CSelectLayoutDialog::InitList()
{
	CustomColumn		gridColumn;
	CustomColumnList	gridColumnsList;

	gridColumn.Reset();
	gridColumn.strHeaderCaption = _M("");
	gridColumn.nWidth = m_nCellWidth+14;

	for ( int i = 0; i < 3; i ++)
		gridColumnsList.push_back(gridColumn);

	CString strColumnOrder = "0;1;2;";
	m_listLayout.SetColumnInfo(gridColumnsList, strColumnOrder);

	// Set Color
	COLORREF colorBackground = RGB(220,220,220);
	m_listLayout.AddDefaultRowColor(colorBackground);
	m_listLayout.SetGridLineColor(colorBackground,colorBackground);

	// Height
	m_listLayout.SetRowHeigt(m_nCellHeight+14);
	m_listLayout.SetHeaderHeight(0);

	m_listLayout.SetMultipleSelection(FALSE);
	m_listLayout.SetShowSelection(FALSE);
	m_listLayout.SetUser(this);

	int nRow = 0;
	CellFormat* pCellFormat = NULL;
	for ( int i = 0; i < m_vecBitmap.size(); i += 3 )
	{
		nRow = m_listLayout.AppendEmptyRow();

		AddLayoutButton(nRow,0,m_vecBitmap[i]);
		if ( i+1 < m_vecBitmap.size() )
		{
			AddLayoutButton(nRow,1,m_vecBitmap[i+1]);
		}
		if ( i+2 < m_vecBitmap.size() )
		{
			AddLayoutButton(nRow,2,m_vecBitmap[i+2]);
		}
	}

}

void CSelectLayoutDialog::AddLayoutButton( int nRow, int nCol, HBITMAP hBitmap )
{
	CellFormat* pCellFormat = m_listLayout.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellCtrlPictureButton;
	pCellFormat->hPicture = hBitmap;
	pCellFormat->strCellCtrlText = _T("");
	m_listLayout.SetCell(nRow, nCol, _T(""), pCellFormat );	
}

BOOL CSelectLayoutDialog::OnCellCtrlClicked( CListCtrl* pListCtrl, int nRow, int nCol, CellFormat* pCellFormat )
{
	m_nIndex = nRow*3 + nCol;

	if ( m_nIndex < m_vecBitmap.size() )
	{
		CDialog::OnOK();
	}

	return TRUE;
}


// CSelectLayoutDialog message handlers

void CSelectLayoutDialog::OnClose()
{
	CheckClose();
}

void CSelectLayoutDialog::OnCancel()
{
	CheckClose();
}

void CSelectLayoutDialog::CheckClose()
{
	if ( m_bMustSelect )
	{
		AfxMessageBox(_M("Please select a layout!"));
		return;
	}

	CDialog::OnCancel();
}
