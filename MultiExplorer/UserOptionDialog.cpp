// UserOptionDialog.cpp : implementation file
//

#include "stdafx.h"
#include "UserOptionDialog.h"


// CUserOptionDialog dialog

IMPLEMENT_DYNAMIC(CUserOptionDialog, CDialog)

CUserOptionDialog::CUserOptionDialog(UserOptions* pUserOption, CWnd* pParent /*=NULL*/)
	: CDialog(CUserOptionDialog::IDD, pParent)
{
	m_pUserOption = pUserOption;
}

CUserOptionDialog::~CUserOptionDialog()
{
}

void CUserOptionDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_USER_OPTION, m_listUserOption);
}


BEGIN_MESSAGE_MAP(CUserOptionDialog, CDialog)
	ON_BN_CLICKED(IDOK, &CUserOptionDialog::OnBnClickedOk)
END_MESSAGE_MAP()


// CUserOptionDialog message handlers
BOOL CUserOptionDialog::OnInitDialog()
{	
	CDialog::OnInitDialog();

	CString strText;

	CustomColumn		gridColumn;
	CustomColumnList	gridColumnsList;

	gridColumn.Reset();
	gridColumn.strHeaderCaption = _M("Property Name");
	gridColumn.nWidth = 180;
	gridColumnsList.push_back(gridColumn);

	gridColumn.Reset();
	gridColumn.strHeaderCaption = _M("Property Value");
	gridColumn.nWidth = 360;
	gridColumnsList.push_back(gridColumn);

	CString strColumnOrder = "0;1;";
	m_listUserOption.SetColumnInfo(gridColumnsList, strColumnOrder);

	// Set Row Bk Color
	m_listUserOption.AddDefaultRowColor(RGB(239,239,239));

	// Height
	m_listUserOption.SetHeaderHeight(m_pUserOption->nRowHeight);
	m_listUserOption.SetRowHeigt(m_pUserOption->nRowHeight);

	m_listUserOption.EnableFilter(FALSE);
	m_listUserOption.EnableSort(FALSE);
	m_listUserOption.SetMultipleSelection(FALSE);
	m_listUserOption.SetShowSelection(FALSE);
	m_listUserOption.SetHighlightChangedCellText(TRUE);
	m_listUserOption.SetSelectThenEdit(FALSE);
	m_listUserOption.SetEditNextCellAfterReturnIsHit(FALSE);
	m_listUserOption.SetEditFirstCellAfterNewRowIsAdded(FALSE);
	m_listUserOption.SetEnablePopupMenu(FALSE);

	m_listUserOption.SetUser(this);
	

	m_listUserOption.m_defaultListFormat.cellType = cellTextReadOnly;

	// Language
	int nRow = m_listUserOption.AppendEmptyRow();
	CellFormat* pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->lfWeight = FW_BOLD;
	m_listUserOption.SetCell(nRow,0,_M("Language"), pCellFormat);

	nRow = m_listUserOption.AppendEmptyRow();
	m_listUserOption.SetCell(nRow,0,_T("  ") + _M("Language"));
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellTextCombo;
	m_listUserOption.SetCell(nRow,1, m_pUserOption->strLanguage, pCellFormat);

	// Dir Tree
	nRow = m_listUserOption.AppendEmptyRow();
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->lfWeight = FW_BOLD;
	m_listUserOption.SetCell(nRow,0,_M("Navigate Tree"), pCellFormat);

	nRow = m_listUserOption.AppendEmptyRow();
	m_listUserOption.SetCell(nRow,0,_T("  ") + _M("Show"));
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellCtrlCheckBox;
	pCellFormat->bChecked = m_pUserOption->bShowDirTree;
	m_listUserOption.SetCell(nRow,1, _T(""), pCellFormat);

	nRow = m_listUserOption.AppendEmptyRow();
	m_listUserOption.SetCell(nRow,0,_T("  ") + _M("Width"));
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellTextEdit;
	strText.Format(_T("%d"), m_pUserOption->nDirTreeDlgWidth);
	m_listUserOption.SetCell(nRow,1, strText, pCellFormat);

	// External Program
	nRow = m_listUserOption.AppendEmptyRow();
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->lfWeight = FW_BOLD;
	m_listUserOption.SetCell(nRow,0,_M("External Program"), pCellFormat);

	nRow = m_listUserOption.AppendEmptyRow();
	m_listUserOption.SetCell(nRow,0,_T("  ") + _M("Internet Explorer"));
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellCtrlFile;
	pCellFormat->bShowCtrl = TRUE;
	pCellFormat->SetPicture(m_pUserOption->strInternetExplorer);
	m_listUserOption.SetCell(nRow,1, m_pUserOption->strInternetExplorer, pCellFormat);

	nRow = m_listUserOption.AppendEmptyRow();
	m_listUserOption.SetCell(nRow,0,_T("  ") + _M("Beyond Compare"));
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellCtrlFile;
	pCellFormat->bShowCtrl = TRUE;
	pCellFormat->SetPicture(m_pUserOption->strBeyondComparePath);
	m_listUserOption.SetCell(nRow,1, m_pUserOption->strBeyondComparePath, pCellFormat);

	nRow = m_listUserOption.AppendEmptyRow();
	m_listUserOption.SetCell(nRow,0,_T("  ") + _M("WinRar"));
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellCtrlFile;
	pCellFormat->bShowCtrl = TRUE;
	pCellFormat->SetPicture(m_pUserOption->strWinRarPath);
	m_listUserOption.SetCell(nRow,1, m_pUserOption->strWinRarPath, pCellFormat);

	nRow = m_listUserOption.AppendEmptyRow();
	m_listUserOption.SetCell(nRow,0,_T("  ") + _M("UltraEdit"));
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellCtrlFile;
	pCellFormat->bShowCtrl = TRUE;
	pCellFormat->SetPicture(m_pUserOption->strUltraEditPath);
	m_listUserOption.SetCell(nRow,1, m_pUserOption->strUltraEditPath, pCellFormat);

	nRow = m_listUserOption.AppendEmptyRow();
	m_listUserOption.SetCell(nRow,0,_T("  ") + _M("NotePad++"));
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellCtrlFile;
	pCellFormat->bShowCtrl = TRUE;
	pCellFormat->SetPicture(m_pUserOption->strNotepadPPPath);
	m_listUserOption.SetCell(nRow,1, m_pUserOption->strNotepadPPPath, pCellFormat);

	// List Style
	nRow = m_listUserOption.AppendEmptyRow();
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->lfWeight = FW_BOLD;
	m_listUserOption.SetCell(nRow,0,_M("Explorer List Style"), pCellFormat);

	nRow = m_listUserOption.AppendEmptyRow();
	m_listUserOption.SetCell(nRow,0,_T("  ") + _M("Row Height"));
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellTextEdit;
	strText.Format(_T("%d"), m_pUserOption->nRowHeight);
	m_listUserOption.SetCell(nRow,1, strText, pCellFormat);

	nRow = m_listUserOption.AppendEmptyRow();
	m_listUserOption.SetCell(nRow,0,_T("  ") + _M("Odd Line BK Color(Active)"));
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellCtrlColor;
	m_listUserOption.SetCell(nRow,1, m_pUserOption->strOddLineBKColorActive, pCellFormat);

	nRow = m_listUserOption.AppendEmptyRow();
	m_listUserOption.SetCell(nRow,0,_T("  ") + _M("Even Line BK Color(Active)"));
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellCtrlColor;
	m_listUserOption.SetCell(nRow,1, m_pUserOption->strEvenLineBKColorActive, pCellFormat);

	nRow = m_listUserOption.AppendEmptyRow();
	m_listUserOption.SetCell(nRow,0,_T("  ") + _M("Odd Line BK Color(Inactive)"));
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellCtrlColor;
	m_listUserOption.SetCell(nRow,1, m_pUserOption->strOddLineBKColorInactive, pCellFormat);

	nRow = m_listUserOption.AppendEmptyRow();
	m_listUserOption.SetCell(nRow,0,_T("  ") + _M("Even Line BK Color(Inactive)"));
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellCtrlColor;
	m_listUserOption.SetCell(nRow,1, m_pUserOption->strEvenLineBKColorInactive, pCellFormat);

	// Toolbar Style
	nRow = m_listUserOption.AppendEmptyRow();
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->lfWeight = FW_BOLD;
	m_listUserOption.SetCell(nRow,0,_M("Toolbar"), pCellFormat);

	nRow = m_listUserOption.AppendEmptyRow();
	m_listUserOption.SetCell(nRow,0,_T("  ") + _M("Button Size"));
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellTextEdit;
	strText.Format(_T("%d"), m_pUserOption->nToolbarButtonSize);
	m_listUserOption.SetCell(nRow,1, strText, pCellFormat);

	nRow = m_listUserOption.AppendEmptyRow();
	m_listUserOption.SetCell(nRow,0,_T("  ") + _M("Auto Load From Desktop"));
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellCtrlCheckBox;
	pCellFormat->bChecked = m_pUserOption->bLoadQuickLaunchFromDesktop;
	m_listUserOption.SetCell(nRow,1, _T(""), pCellFormat);

	nRow = m_listUserOption.AppendEmptyRow();
	m_listUserOption.SetCell(nRow,0,_T("  ") + _M("Auto Load From Taskbar"));
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellCtrlCheckBox;
	pCellFormat->bChecked = m_pUserOption->bLoadQuickLaunchFromTaskbar;
	m_listUserOption.SetCell(nRow,1, _T(""), pCellFormat);

	// Exit
	nRow = m_listUserOption.AppendEmptyRow();
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->lfWeight = FW_BOLD;
	m_listUserOption.SetCell(nRow,0,_M("Exit"), pCellFormat);

	nRow = m_listUserOption.AppendEmptyRow();
	m_listUserOption.SetCell(nRow,0,_T("  ") + _M("Prompt For Save"));
	pCellFormat = m_listUserOption.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellCtrlCheckBox;
	pCellFormat->bChecked = m_pUserOption->bPromptForSave;
	m_listUserOption.SetCell(nRow,1, _T(""), pCellFormat);

	SetUIText();

	return TRUE;
}

void CUserOptionDialog::OnBnClickedOk()
{
	int nRow = 0;

	nRow++;
	m_pUserOption->strLanguage					= m_listUserOption.GetItemText(nRow++,1);

	nRow++;
	m_pUserOption->bShowDirTree					= m_listUserOption.GetCellFormat(nRow++,1)->bChecked;
	m_pUserOption->nDirTreeDlgWidth				= atoi(m_listUserOption.GetItemText(nRow++,1));
	
	nRow++;
	m_pUserOption->strInternetExplorer			= m_listUserOption.GetItemText(nRow++,1);
	m_pUserOption->strBeyondComparePath			= m_listUserOption.GetItemText(nRow++,1);
	m_pUserOption->strWinRarPath				= m_listUserOption.GetItemText(nRow++,1);
	m_pUserOption->strUltraEditPath				= m_listUserOption.GetItemText(nRow++,1);
	m_pUserOption->strNotepadPPPath				= m_listUserOption.GetItemText(nRow++,1);

	nRow++;
	m_pUserOption->nRowHeight					= atoi(m_listUserOption.GetItemText(nRow++,1));
	m_pUserOption->strOddLineBKColorActive		= m_listUserOption.GetItemText(nRow++,1);
	m_pUserOption->strEvenLineBKColorActive		= m_listUserOption.GetItemText(nRow++,1);
	m_pUserOption->strOddLineBKColorInactive	= m_listUserOption.GetItemText(nRow++,1);
	m_pUserOption->strEvenLineBKColorInactive	= m_listUserOption.GetItemText(nRow++,1);

	nRow++;
	m_pUserOption->nToolbarButtonSize			= atoi(m_listUserOption.GetItemText(nRow++,1));
	m_pUserOption->bLoadQuickLaunchFromDesktop	= m_listUserOption.GetCellFormat(nRow++,1)->bChecked;
	m_pUserOption->bLoadQuickLaunchFromTaskbar	= m_listUserOption.GetCellFormat(nRow++,1)->bChecked;

	nRow++;
	m_pUserOption->bPromptForSave				= m_listUserOption.GetCellFormat(nRow++,1)->bChecked;

	OnOK();
}

BOOL CUserOptionDialog::OnCellComboDisplayed( CListCtrl* pListCtrl, int nRow, int nCol, CellFormat* pCellFormat, CStringList& strComboItem )
{
	if ( nRow == 1 && nCol == 1 )
	{
		strComboItem.AddTail(_T("English"));
		strComboItem.AddTail(_T("ÖÐÎÄ"));
	}

	return TRUE;
}

void CUserOptionDialog::SetUIText()
{
	SetDlgItemText(IDOK, _M("Save"));
	SetDlgItemText(IDCANCEL, _M("Cancel"));

	SetWindowText(_M("Options"));
}

BOOL CUserOptionDialog::OnCellCtrlClicked( CListCtrl* pListCtrl, int nRow, int nCol, CellFormat* pCellFormat )
{
	return TRUE;
}
