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
	m_bPortChanged = FALSE;
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
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDOK, &CNetworkOperationDialog::OnBnClickedOk)
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
	m_listMachine.AddDefaultRowColor(m_pDiskFileManager->m_userOption.strEvenLineBKColorActive);
	m_listMachine.AddDefaultRowColor(m_pDiskFileManager->m_userOption.strOddLineBKColorActive);

	// Height
	m_listMachine.SetHeaderHeight(m_pDiskFileManager->m_userOption.nRowHeight);
	m_listMachine.SetRowHeigt(m_pDiskFileManager->m_userOption.nRowHeight);

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
	
	LoadConfig();
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
	SetDlgItemText(IDC_STATIC_LOCAL_PORT, _M("Listening Port"));
	SetDlgItemText(IDC_STATIC_REMOTE_MACHINE, _M("Remote Machine"));

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

void CNetworkOperationDialog::OnClose()
{
	SaveConfig();
	__super::OnClose();
}


void CNetworkOperationDialog::OnBnClickedOk()
{
	SaveConfig();
	__super::OnOK();
}

void CNetworkOperationDialog::LoadConfig()
{
	CString strPort;
	for (int i = 0; i < (int)m_pDiskFileManager->m_userOption.vecRemoteMachine.size(); i++)
	{
		RemoteMachine remoteMachine = m_pDiskFileManager->m_userOption.vecRemoteMachine[i];
		int nRow = m_listMachine.AppendEmptyRow();
		m_listMachine.SetCell(nRow, 0, remoteMachine.strIpAddress);

		strPort.Format("%d", remoteMachine.nPort);
		m_listMachine.SetCell(nRow, 1, strPort);
	}

	SetDlgItemInt(IDC_EDIT_LOCAL_PORT, m_pDiskFileManager->m_userOption.nLocalListeningPort);
}

void CNetworkOperationDialog::SaveConfig()
{
	CString strPort;
	m_pDiskFileManager->m_userOption.vecRemoteMachine.clear();

	for (int i = 0; i < m_listMachine.GetItemCount(); i++)
	{
		RemoteMachine remoteMachine;

		remoteMachine.strIpAddress = m_listMachine.GetItemText(i, 0);
		remoteMachine.nPort = atoi(m_listMachine.GetItemText(i, 1));

		m_pDiskFileManager->m_userOption.vecRemoteMachine.push_back(remoteMachine);
	}

	int nPort = GetDlgItemInt(IDC_EDIT_LOCAL_PORT);
	if (nPort != m_pDiskFileManager->m_userOption.nLocalListeningPort)
	{
		m_pDiskFileManager->m_userOption.nLocalListeningPort = nPort;
		m_bPortChanged = TRUE;
	}
}