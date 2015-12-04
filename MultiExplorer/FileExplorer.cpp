// FileExplorer.cpp : implementation file
//

#include "stdafx.h"
#include "MultiExplorer.h"
#include "FileExplorer.h"
#include "Resource.h"

// CFileExplorer dialog

IMPLEMENT_DYNAMIC(CFileExplorer, CDialog)

CFileExplorer::CFileExplorer(CWnd* pParent /*=NULL*/)
	: CDialog(CFileExplorer::IDD, pParent)
	, m_strCurDir(_T(""))
	, m_strFilter(_T(""))
{
	m_pDiskFileManager = NULL;
	m_bShowHidden = FALSE;
	m_bClosed = FALSE;

	m_bUpdateIconInProgress = FALSE;
	m_bQuickSearching = FALSE;

	m_bInitializing = TRUE;

	m_nLayoutIndex = -1;

	m_bRecursiveSearch = FALSE;
}

CFileExplorer::~CFileExplorer()
{
}

void CFileExplorer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DIR, m_comboDir);
	DDX_Control(pDX, IDC_EDIT_SEARCH, m_editSearch);
	DDX_Control(pDX, IDC_LIST_FILE, m_listFile);
	DDX_CBString(pDX, IDC_COMBO_DIR, m_strCurDir);
	DDX_Text(pDX, IDC_EDIT_SEARCH, m_strFilter);
	DDX_Control(pDX, IDC_STATIC_STATISTICS, m_textStatistics);
}


BEGIN_MESSAGE_MAP(CFileExplorer, CDialog)
	ON_CBN_SELCHANGE(IDC_COMBO_DIR, &CFileExplorer::OnCbnSelchangeComboDir)
	ON_EN_CHANGE(IDC_EDIT_SEARCH, &CFileExplorer::OnEnChangeEditSearch)
	ON_BN_CLICKED(IDC_BUTTON_UPWARD, &CFileExplorer::OnBnClickedButtonUpward)
	ON_BN_CLICKED(IDC_BUTTON_BACKWARD, &CFileExplorer::OnBnClickedButtonBackward)
	ON_BN_CLICKED(IDC_BUTTON_FORWARD, &CFileExplorer::OnBnClickedButtonForward)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CFileExplorer::OnBnClickedButtonRefresh)
	ON_BN_CLICKED(IDC_CHECK_SHOW_HIDDEN, &CFileExplorer::OnBnClickedCheckShowHidden)
	ON_BN_CLICKED(IDC_BUTTON_QUIT, &CFileExplorer::OnBnClickedButtonQuit)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CFileExplorer::OnInitDialog()
{
	CDialog::OnInitDialog();

	InitButtons();
	InitList();
	AddDirSelection();

	SetUIText();

	SetDlgItemText(IDC_COMBO_DIR, m_strCurDir);
	SetDlgItemText(IDC_EDIT_SEARCH, m_strFilter);

	m_bInitializing = FALSE;

	ShowDir(m_strCurDir, m_strFilter, TRUE);

	return TRUE;
}

BOOL CFileExplorer::PreTranslateMessage(MSG* pMsg) 
{
	BOOL bRetVal=FALSE;

	if (pMsg->message==WM_KEYDOWN)
	{
		if (pMsg->wParam==VK_RETURN) // Enter - input is ok
		{
			if ( m_comboDir.IsChild(GetFocus()) )
			{
				m_comboDir.GetWindowText(m_strCurDir);
				FormatDirName(m_strCurDir);
				ShowDir(m_strCurDir, _T(""), TRUE);
			}
			else if ( GetFocus() == &m_editSearch )
			{
				m_bRecursiveSearch = TRUE;
				OnEnChangeEditSearch();
			}
			bRetVal=TRUE;
		}
		else if (pMsg->wParam==VK_ESCAPE) // ESCAPE - ignore input
		{
			bRetVal=TRUE;
		}
	}
	else if (pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_RBUTTONDOWN || 
			 pMsg->message == WM_MBUTTONDOWN )
	{
		if ( pMsg->message == WM_MBUTTONDOWN )
		{
			m_pDiskFileManager->SwitchWithActiveFileExplorer(this);
		}
		else
		{
			m_pDiskFileManager->SetActiveFileExplorer(this);
		}
		
	}

	if (!bRetVal)
		bRetVal=CDialog::PreTranslateMessage(pMsg);

	return bRetVal;
}

void CFileExplorer::SetPosition(const CRect& rcWnd, BOOL bEnableClose )
{
	SetWindowPos(NULL, rcWnd.left, rcWnd.top, rcWnd.Width(), rcWnd.Height(), SWP_SHOWWINDOW);

	CRect rcClient;
	GetWindowRect(&rcClient);
	ScreenToClient(rcClient);

	int nRight = rcClient.right - 13;
	int nBottom = rcClient.bottom - 13;

	LayoutWnd(&m_editSearch, -1, nRight-27, -1, -1);
	LayoutWnd(&m_btClose,nRight-25, nRight+5, -1, -1);
	LayoutWnd(&m_comboDir, -1, nRight, -1, -1);
	LayoutWnd(&m_listFile, -1, nRight, -1, nBottom-25 );
	LayoutWnd(&m_textStatistics, -1, nRight, nBottom-18, nBottom);

	m_btClose.EnableWindow(bEnableClose);
}

void CFileExplorer::InitList()
{
	CustomColumn		gridColumn;
	CustomColumnList	gridColumnsList;

	gridColumn.Reset();
	gridColumn.strHeaderCaption = _T("Name");
	gridColumn.nWidth = m_vecColumnWidth[0];
	gridColumnsList.push_back(gridColumn);

	gridColumn.Reset();
	gridColumn.strHeaderCaption = _T("Size");
	gridColumn.compareType = compareAsNumber;
	gridColumn.nFormat = LVCFMT_RIGHT;
	gridColumn.nWidth = m_vecColumnWidth[1];
	gridColumnsList.push_back(gridColumn);

	gridColumn.Reset();
	gridColumn.strHeaderCaption = _T("Ext");
	gridColumn.nWidth = m_vecColumnWidth[2];
	gridColumnsList.push_back(gridColumn);

	gridColumn.Reset();
	gridColumn.strHeaderCaption = _T("Modified");
	gridColumn.nWidth = m_vecColumnWidth[3];
	gridColumnsList.push_back(gridColumn);

	CString strColumnOrder = "0;1;2;3;";
	m_listFile.SetColumnInfo(gridColumnsList, strColumnOrder);

	// Set Row Bk Color
	m_listFile.AddDefaultRowColor(m_pDiskFileManager->m_userOption.strOddLineBKColorInactive);
	m_listFile.AddDefaultRowColor(m_pDiskFileManager->m_userOption.strEvenLineBKColorInactive);

	// Height
	m_listFile.SetHeaderHeight(m_pDiskFileManager->m_userOption.nRowHeight);
	m_listFile.SetRowHeigt(m_pDiskFileManager->m_userOption.nRowHeight);

	m_listFile.EnableFilter(FALSE);
	m_listFile.SetMultipleSelection(FALSE);
	m_listFile.SetShowSelection(TRUE);
	m_listFile.SetHighlightChangedCellText(TRUE);
	m_listFile.SetSelectThenEdit(TRUE);
	m_listFile.SetEnablePopupMenu(FALSE);
	m_listFile.SetEditNextCellAfterReturnIsHit(TRUE);

	m_listFile.SetUser(m_pDiskFileManager);
}

void CFileExplorer::ShowDir( const CString& strDir, CString strFilter, BOOL bAddToHistory, BOOL bUpdateDirTree )
{
	m_strCurDir = strDir;

	if ( !m_bQuickSearching )
		DisableAllCtrls();

	m_comboDir.SetWindowText(strDir);
	m_strFilter = strFilter;
	UpdateData(FALSE);

	m_listFile.DeleteAllItems(FALSE);
	strFilter.MakeLower();

	CString strFilterName = strFilter;
	CString strFilterExt = _T("");
	int nToken = strFilter.ReverseFind(_T('.'));
	int nExtLen = strFilter.GetLength()-nToken;
	if ( nToken != -1 )
	{
		strFilterName = strFilter.Left(nToken);
		strFilterExt = strFilter.Right(nExtLen);
	}

	m_listFile.m_strHighlightText = strFilterName;

	vector<CDirectoryInfo> vecDir;
	vector<CFileInfo> vecFile;

	CString strAcutalDir = GetCurDir();
	m_pDiskFileManager->GetDirFileListRecursive(strAcutalDir, m_bRecursiveSearch, _T(""), vecDir, vecFile, m_bShowHidden, strFilterName, strFilterExt);

	m_nDirCount = (int)vecDir.size();
	for ( int i = 0; i < m_nDirCount; i ++ )
	{
		AppendDir(vecDir[i]);
	}

	m_nFileCount = (int)vecFile.size();
	for ( int i = 0; i < m_nFileCount; i ++ )
	{
		AppendFile(vecFile[i]);
	}

	m_bRecursiveSearch = FALSE;

	ShowStatictics(m_nDirCount, m_nFileCount, 0);

	if ( bAddToHistory)
		m_pDiskFileManager->DirShown(&m_listFile, strDir);

	if ( m_bQuickSearching )
	{
		m_pDiskFileManager->DoUpdateIcon(this);
		m_bQuickSearching = FALSE;
		EnableAllCtrls();
		m_listFile.SortPrevious();
	}
	else
		m_pDiskFileManager->StartUpdateIconThread(this);

	if ( m_pDiskFileManager->m_userOption.bShowDirTree && bUpdateDirTree)
	{
		m_pDiskFileManager->m_dirTreeDialog.m_pDirTree->TunnelTree(strAcutalDir);
	}
}

void CFileExplorer::AddDirSelection()
{
	m_comboDir.AddString(_M("Desktop"));
	m_comboDir.AddString(_M("My Documents"));

	vector<CDiskDriverInfo> vecDiskDriverInfo = m_pDiskFileManager->m_vecDiskDriverInfo;;

	int nCount = vecDiskDriverInfo.size();
	for ( int i = 0; i < nCount; i ++ )
	{
		m_comboDir.AddString(vecDiskDriverInfo[i].strDriverLetter);
	}
}

int CFileExplorer::AppendDir( const CDirectoryInfo& di )
{
	int nRow = m_listFile.AppendEmptyRow();

	CellFormat* pCellFormat = m_listFile.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellCtrlDirectory;
	pCellFormat->bShowCtrl = FALSE;
	pCellFormat->bShowHighlight = TRUE;
	m_listFile.SetCell(nRow, 0, di.strName, pCellFormat);

	m_listFile.SetCell(nRow, 3, di.strDate);

	return nRow;
}

int CFileExplorer::AppendFile( const CFileInfo& fi )
{
	int nRow = m_listFile.AppendEmptyRow();

	CellFormat* pCellFormat = m_listFile.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->cellType = cellCtrlFile;
	pCellFormat->bShowCtrl = FALSE;
	pCellFormat->SetPicture(m_pDiskFileManager->GetIconFromCache(fi.strExt));
	pCellFormat->bShowHighlight = TRUE;
	m_listFile.SetCell(nRow, 0, fi.strName, pCellFormat);

	pCellFormat = m_listFile.m_defaultListFormat.CopyFormat(levelCell);
	pCellFormat->uTextAlign = DT_RIGHT;
	m_listFile.SetCell(nRow, 1, m_pDiskFileManager->m_commonTool.ConvertToFileSize(fi.nSize), pCellFormat);

	m_listFile.SetCell(nRow, 2, fi.strExt);
	m_listFile.SetCell(nRow, 3, fi.strDate);

	return nRow;
}

void CFileExplorer::InitButtons()
{
	m_btShowBackward.SubclassDlgItem ( IDC_BUTTON_BACKWARD, this );
	m_btShowBackward.SetIcon(IDI_ICON_BACKWARD);
	m_btShowBackward.EnableWindow(FALSE);

	m_btShowForward.SubclassDlgItem ( IDC_BUTTON_FORWARD, this );
	m_btShowForward.SetIcon(IDI_ICON_FORWARD);
	m_btShowForward.EnableWindow(FALSE);

	m_btShowUpward.SubclassDlgItem ( IDC_BUTTON_UPWARD, this );
	m_btShowUpward.SetIcon(IDI_ICON_UPWARD);
	m_btShowUpward.EnableWindow(FALSE);

	m_btRefresh.SubclassDlgItem ( IDC_BUTTON_REFRESH, this );
	m_btRefresh.SetIcon(IDI_ICON_REFRESH);
	m_btRefresh.SetTooltipText(_M("Refresh"));

	m_btShowHidden.SubclassDlgItem ( IDC_CHECK_SHOW_HIDDEN, this );
	m_btShowHidden.SetIcon(IDI_ICON_SHOW_HIDDEN);
	
	m_btClose.SubclassDlgItem ( IDC_BUTTON_QUIT, this );
	m_btClose.SetIcon(IDI_ICON_CLOSE);
}




// CFileExplorer message handlers

void CFileExplorer::OnCbnSelchangeComboDir()
{
	int nIndex = m_comboDir.GetCurSel();
	m_comboDir.GetLBText(nIndex, m_strCurDir);

	ShowDir(m_strCurDir, _T(""), TRUE);
}

void CFileExplorer::OnEnChangeEditSearch()
{
	if ( m_bInitializing )
		return;

	if ( m_strCurDir.IsEmpty() )
		return;

	UpdateData(TRUE);

	FormatDirName(m_strCurDir);

	m_bQuickSearching = TRUE;
	ShowDir(m_strCurDir, m_strFilter, FALSE, FALSE);
}

BOOL CFileExplorer::ShowDirectory( const CString& strDir, BOOL bAddToHistory )
{
	m_strCurDir = strDir;
	FormatDirName(m_strCurDir);

	ShowDir(m_strCurDir, _T(""), bAddToHistory);

	return TRUE;
}

void CFileExplorer::OnBnClickedButtonUpward()
{
	m_strCurDir = m_strCurDir.Left(m_strCurDir.GetLength()-1);

	int nToken = m_strCurDir.ReverseFind(_T('\\'));
	m_strCurDir = m_strCurDir.Left(nToken+1);

	CString strFilter;
	GetDlgItemText(IDC_EDIT_SEARCH, strFilter);

	m_bQuickSearching = TRUE;
	ShowDir(m_strCurDir, _T(""), TRUE);
}

void CFileExplorer::SelectChanged()
{
	ShowStatictics(m_nDirCount, m_nFileCount, m_listFile.FindSelectedRow().size());
}

void CFileExplorer::ShowStatictics( int nDirCount, int nFileCount, int nSelectionCount )
{
	CString strStatistics;
	strStatistics.Format(_M("Total %d directory(s), %d file(s). %d item(s) selected."), 
		nDirCount, nFileCount, nSelectionCount);
	m_textStatistics.SetWindowText(strStatistics);
}

void CFileExplorer::OnBnClickedButtonBackward()
{
	m_pDiskFileManager->ShowPrevDir(&m_listFile);
}

void CFileExplorer::OnBnClickedButtonForward()
{
	m_pDiskFileManager->ShowNextDir(&m_listFile);
}

void CFileExplorer::OnBnClickedButtonRefresh()
{
	int nScrollPos = m_listFile.GetScrollPos(SB_VERT);
	m_bQuickSearching = TRUE;
	ShowDir(m_strCurDir, m_strFilter, FALSE, FALSE);

	m_listFile.SetScrollPos(SB_VERT, nScrollPos);
}

void CFileExplorer::OnBnClickedCheckShowHidden()
{
	m_bShowHidden = !m_bShowHidden;
	m_btShowHidden.SetCheck(m_bShowHidden);

	ShowDir(m_strCurDir, m_strFilter, FALSE, FALSE);
}

void CFileExplorer::LayoutWnd( CWnd* pWnd, int nLeft, int nRight, int nTop, int nBottom)
{
	CRect rcWnd;
	pWnd->GetWindowRect(&rcWnd);
	ScreenToClient(&rcWnd);

	if ( nLeft != -1 )
		rcWnd.left = nLeft;
	if ( nRight != -1 )
		rcWnd.right = nRight;
	if ( nTop != -1 )
		rcWnd.top = nTop;
	if ( nBottom != -1 )
		rcWnd.bottom = nBottom;

	pWnd->MoveWindow(rcWnd);
}

void CFileExplorer::OnBnClickedButtonQuit()
{
	CString strLayout = m_pDiskFileManager->m_layoutManager.SelectLayoutString(m_pDiskFileManager->GetExplorerCount()-1, FALSE);
	if ( strLayout.IsEmpty())
		return;

	m_pDiskFileManager->OnDeleteCell(this, strLayout);
}

void CFileExplorer::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	//CRect rc;
	//GetClientRect(&rc);

	//if ( (rc.bottom-point.y<13) && (rc.right-point.x<13) )
	//	m_pDiskFileManager->OnChangeLayout(FALSE);
}

void CFileExplorer::AdjustListColumn()
{
	CRect rcList;
	m_listFile.GetClientRect(&rcList);

	int nWidth = 0;
	for ( int i = 1; i < m_listFile.m_ctrlHeader.GetItemCount(); i ++ )
	{
		nWidth += m_listFile.m_ctrlHeader.m_gridColumnsList[i].nWidth;
	}

	int nNameWidth = rcList.Width()-nWidth;
	if ( nNameWidth < 70 )
		nNameWidth = 70;

	m_listFile.SetColumnWidth(0, nNameWidth);
}

void CFileExplorer::DisableAllCtrls()
{
	m_bUpdateIconInProgress = TRUE;
	m_btShowBackward.EnableWindow(FALSE);
	m_btShowForward.EnableWindow(FALSE);
	m_btShowUpward.EnableWindow(FALSE);
	m_btRefresh.EnableWindow(FALSE);
	m_btShowHidden.EnableWindow(FALSE);
	m_btClose.EnableWindow(FALSE);

	m_comboDir.EnableWindow(FALSE);
	m_editSearch.EnableWindow(FALSE);
}

void CFileExplorer::EnableAllCtrls()
{
	m_btShowUpward.EnableWindow(m_strCurDir.GetLength()>3 && m_strCurDir != _M("Desktop") && m_strCurDir != _M("My Documents") );
	m_btShowBackward.EnableWindow(m_pDiskFileManager->HasPrevDir(&m_listFile));
	m_btShowForward.EnableWindow(m_pDiskFileManager->HasNextDir(&m_listFile));

	m_btRefresh.EnableWindow(TRUE);
	m_btShowHidden.EnableWindow(TRUE);
	m_btClose.EnableWindow(TRUE);

	m_comboDir.EnableWindow(TRUE);
	m_editSearch.EnableWindow(TRUE);

	m_bUpdateIconInProgress = FALSE;
}

HBRUSH CFileExplorer::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	//if ( nCtlColor == CTLCOLOR_DLG )
	//	return ( HBRUSH ) CreateSolidBrush(RGB(111,111,111));

	return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

}

BOOL CFileExplorer::OnEraseBkgnd(CDC* pDC) 
{
	CDialog::OnEraseBkgnd(pDC);	

	//CRect rect;
	//GetClientRect(&rect);

	//CBrush brGray(RGB(111,111,111));
	//pDC->FillRect(rect, &brGray);

	return TRUE;
}

CString CFileExplorer::GetCurDir()
{
	CString strAcutalDir = m_strCurDir;

	if ( strAcutalDir == _M("Desktop"))
		strAcutalDir = m_pDiskFileManager->m_defaultDir.strDesktopDir;
	else if ( strAcutalDir == _M("My Documents"))
		strAcutalDir = m_pDiskFileManager->m_defaultDir.strMyDocumentsDir;

	return strAcutalDir;
}

void CFileExplorer::FormatDirName( CString& strDir )
{
	if ( strDir.GetAt(strDir.GetLength()-1) != _T('\\') && strDir != _M("Desktop") && strDir != _M("My Documents"))
	{
		strDir += _T("\\");
	}
}

void CFileExplorer::OnLanguageChanged()
{
	m_listFile.m_ctrlHeader.Invalidate(TRUE);
	SelectChanged();
	SetUIText();
}

void CFileExplorer::SetUIText()
{
	m_btShowBackward.SetTooltipText(_M("Backward"));
	m_btShowForward.SetTooltipText(_M("Forward"));
	m_btShowUpward.SetTooltipText(_M("Upward"));
	m_btShowHidden.SetTooltipText(_M("Show hidden files"));
	m_btClose.SetTooltipText(_M("Close"));
}

void CFileExplorer::EnableClose( BOOL bEnable )
{
	m_btClose.EnableWindow(bEnable);
}
