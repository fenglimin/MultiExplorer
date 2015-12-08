// CNetworkOperationDialog.cpp : implementation file
//

#include "stdafx.h"
#include "MultiExplorer.h"
#include "NetworkOperationDialog.h"
#include "afxdialogex.h"
#include "DiskFileManager.h"

// CNetworkOperationDialog dialog

static UINT GetClipboardDataThreadFunc(LPVOID pParam)
{
	CNetworkOperationDialog* pDlg = (CNetworkOperationDialog*)pParam;

	CMenu* mnu = pDlg->GetSystemMenu(FALSE);
	mnu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
	pDlg->GetDlgItem(IDOK)->EnableWindow(FALSE);
	pDlg->GetDlgItem(IDC_BUTTON_GET_DATA)->EnableWindow(FALSE);

	CString strIp = pDlg->m_listMachine.GetItemText(pDlg->m_nRow, 0);
	CString strPort = pDlg->m_listMachine.GetItemText(pDlg->m_nRow, 1);
	int nPort = atoi(strPort);

	CString strMsg;
	strMsg.Format(_M("%s Getting clipboard from %s ......\r\n\r\n"), pDlg->GetCurrentFormattedTime(FALSE), strIp);
	pDlg->SetDlgItemText(IDC_EDIT_CLIPBOARD, strMsg);

	CString strData;
	if (!pDlg->m_pDiskFileManager->m_workTool.Request_GetClipboardData(strIp, nPort, strData))
		strData = pDlg->GetCurrentFormattedTime(FALSE) + " " + strData;

	pDlg->SetDlgItemText(IDC_EDIT_CLIPBOARD, strMsg + strData + "\r\n\r\n" + pDlg->GetCurrentFormattedTime(FALSE) + " " + _M("Get clipboard ended!"));

	mnu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND);
	pDlg->GetDlgItem(IDOK)->EnableWindow(TRUE);
	pDlg->GetDlgItem(IDC_BUTTON_GET_DATA)->EnableWindow(TRUE);

	return 0;
}


IMPLEMENT_DYNAMIC(CNetworkOperationDialog, CDialog)

CNetworkOperationDialog::CNetworkOperationDialog(CDiskFileManager* pDiskFileManager, CWnd* pParent /*=NULL*/)
	: CDialog(CNetworkOperationDialog::IDD, pParent)
{
	m_pDiskFileManager = pDiskFileManager;
}

CNetworkOperationDialog::~CNetworkOperationDialog()
{
}

void CNetworkOperationDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_MACHINE, m_listMachine);
}


BEGIN_MESSAGE_MAP(CNetworkOperationDialog, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_GET_DATA, &CNetworkOperationDialog::OnBnClickedButtonGetData)
END_MESSAGE_MAP()


// CNetworkOperationDialog message handlers
BOOL CNetworkOperationDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString strText;

	CustomColumn		gridColumn;
	CustomColumnList	gridColumnsList;

	gridColumn.Reset();
	gridColumn.strHeaderCaption = _M("Ip Address");
	gridColumn.nWidth = 110;
	gridColumnsList.push_back(gridColumn);

	gridColumn.Reset();
	gridColumn.strHeaderCaption = _M("Port");
	gridColumn.nWidth = 50;
	gridColumnsList.push_back(gridColumn);

	CString strColumnOrder = "0;1;";
	m_listMachine.SetColumnInfo(gridColumnsList, strColumnOrder);

	// Set Row Bk Color
	m_listMachine.AddDefaultRowColor(RGB(239, 239, 239));
	m_listMachine.AddDefaultRowColor(RGB(229, 229, 229));

	// Height
	m_listMachine.SetHeaderHeight(24);
	m_listMachine.SetRowHeigt(24);

	m_listMachine.EnableFilter(FALSE);
	m_listMachine.EnableSort(FALSE);
	m_listMachine.SetMultipleSelection(FALSE);
	m_listMachine.SetShowSelection(TRUE);
	m_listMachine.SetHighlightChangedCellText(TRUE);
	m_listMachine.SetSelectThenEdit(TRUE);
	m_listMachine.SetEditNextCellAfterReturnIsHit(TRUE);
	m_listMachine.SetEditFirstCellAfterNewRowIsAdded(TRUE);
	m_listMachine.SetEnablePopupMenu(TRUE);

	m_listMachine.SetUser(this);


	m_listMachine.m_defaultListFormat.cellType = cellTextEdit;
	
	int nRow = m_listMachine.AppendEmptyRow();
	m_listMachine.SetCell(nRow, 0, "127.0.0.1");
	m_listMachine.SetCell(nRow, 1, "1229");

	SetUIText();

	return TRUE;
}

BOOL CNetworkOperationDialog::OnRowLDblClicked(CListCtrl* pListCtrl, int nRow, int nCol, UINT nFlags, CPoint point)
{

	return TRUE;
}

void CNetworkOperationDialog::SetUIText()
{
	SetDlgItemText(IDOK, _M("OK"));
	SetDlgItemText(IDCANCEL, _M("Cancel"));
	SetDlgItemText(IDC_BUTTON_GET_DATA, _M("Get Clipboard Data"));
	SetWindowText(_M("Network Clipboard"));
}

void CNetworkOperationDialog::OnBnClickedButtonGetData()
{
	POSITION pos = m_listMachine.GetFirstSelectedItemPosition();
	int nRow = m_listMachine.GetNextSelectedItem(pos);

	if (nRow == -1)
	{
		SetDlgItemText(IDC_EDIT_CLIPBOARD, GetCurrentFormattedTime(FALSE) + " " + _M("Please select a machine!"));
		return;
	}

	m_nRow = nRow;

	AfxBeginThread(GetClipboardDataThreadFunc, (void*)this);
}


CString CNetworkOperationDialog::GetCurrentFormattedTime(BOOL bForFileName)
{
	CString strFormat = "%04d-%02d-%02d %02d-%02d-%02d";
	if (!bForFileName)
	{
		strFormat = "%04d-%02d-%02d %02d:%02d:%02d";
	}

	COleDateTime dt = COleDateTime::GetCurrentTime();
	CString strRet;
	strRet.Format(strFormat, dt.GetYear(), dt.GetMonth(), dt.GetDay(),
		dt.GetHour(), dt.GetMinute(), dt.GetSecond());

	return strRet;
}