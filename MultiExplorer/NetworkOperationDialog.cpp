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
	pDlg->OnGetClipboardData();

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

	m_pDiskFileManager->SetWorkToolUser(this);

	return TRUE;
}

BOOL CNetworkOperationDialog::OnRowLDblClicked(CListCtrl* pListCtrl, int nRow, int nCol, UINT nFlags, CPoint point)
{

	return TRUE;
}

void CNetworkOperationDialog::SetUIText()
{
	SetDlgItemText(IDOK, _M("Exit"));
	SetDlgItemText(IDCANCEL, _M("Cancel"));
	SetDlgItemText(IDC_BUTTON_GET_DATA, _M("Get Clipboard Data"));
	SetDlgItemText(IDC_STATIC_LOCAL_PORT, _M("Listening Port"));
	SetDlgItemText(IDC_STATIC_REMOTE_MACHINE, _M("Remote Machine"));
	SetDlgItemText(IDC_CHECK_UNICODE, _M("Use Unicode"));
	SetDlgItemText(IDC_CHECK_APPEND_MESSAGE, _M("Append Message"));

	SetWindowText(_M("Network Clipboard"));
}

void CNetworkOperationDialog::OnBnClickedButtonGetData()
{
	m_pDiskFileManager->m_userOption.bClipboardUnicode = IsDlgButtonChecked(IDC_CHECK_UNICODE);
	m_pDiskFileManager->m_userOption.bAppendMessage = IsDlgButtonChecked(IDC_CHECK_APPEND_MESSAGE);

	POSITION pos = m_listMachine.GetFirstSelectedItemPosition();
	int nRow = m_listMachine.GetNextSelectedItem(pos);

	if (nRow == -1)
	{
		AppendMessage(_M("Please select a machine!"), !m_pDiskFileManager->m_userOption.bAppendMessage, TRUE, TRUE);
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
	((CButton*)GetDlgItem(IDC_CHECK_UNICODE))->SetCheck(m_pDiskFileManager->m_userOption.bClipboardUnicode);
	((CButton*)GetDlgItem(IDC_CHECK_APPEND_MESSAGE))->SetCheck(m_pDiskFileManager->m_userOption.bAppendMessage);
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

	m_pDiskFileManager->m_userOption.bClipboardUnicode = IsDlgButtonChecked(IDC_CHECK_UNICODE);
	m_pDiskFileManager->m_userOption.bAppendMessage = IsDlgButtonChecked(IDC_CHECK_APPEND_MESSAGE);

	m_pDiskFileManager->SetWorkToolUser(NULL);
}

void CNetworkOperationDialog::AppendMessage(CString strMessage, BOOL bCleanFirst, BOOL bAddTimeStamp, BOOL bAppendEndline)
{
	CString strExsitingMessage;
	if (!bCleanFirst)
		GetDlgItemText(IDC_EDIT_CLIPBOARD, strExsitingMessage);

	if (strExsitingMessage.GetLength() != 0)
		strExsitingMessage += "\r\n";

	if (bAddTimeStamp)
	{
		strExsitingMessage += GetCurrentFormattedTime(FALSE);
		strExsitingMessage += " ";
	}

	strExsitingMessage += strMessage;

	if (bAppendEndline)
		strExsitingMessage += "\r\n";

	SetDlgItemText(IDC_EDIT_CLIPBOARD, strExsitingMessage);
}

void CNetworkOperationDialog::OnGetClipboardData()
{
	CMenu* mnu = GetSystemMenu(FALSE);
	mnu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
	GetDlgItem(IDOK)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_GET_DATA)->EnableWindow(FALSE);

	CString strIp = m_listMachine.GetItemText(m_nRow, 0);
	CString strPort = m_listMachine.GetItemText(m_nRow, 1);
	int nPort = atoi(strPort);

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CLIPBOARD);
	int nLineCount = m_pDiskFileManager->m_userOption.bAppendMessage ? pEdit->GetLineCount() : 0;

	if (m_pDiskFileManager->m_userOption.bAppendMessage)
		AppendMessage("------------------------------------------------------------------------------------------", FALSE, FALSE, FALSE);
	CString strMsg;
	strMsg.Format(_M("Getting clipboard from %s ......"), strIp);
	AppendMessage(strMsg, !m_pDiskFileManager->m_userOption.bAppendMessage, TRUE, FALSE);

	CString strData;
	int nFormat = m_pDiskFileManager->m_userOption.bClipboardUnicode ? CF_UNICODETEXT : CF_TEXT;
	BOOL bOk = m_pDiskFileManager->m_pWorkTool->Request_GetClipboardData(strIp, nPort, nFormat, strData);
	if (strData.GetLength() != 0)
		AppendMessage(strData, FALSE, TRUE, bOk);

	AppendMessage(_M("Get clipboard ended!"), FALSE, TRUE, FALSE);
	if (m_pDiskFileManager->m_userOption.bAppendMessage)
		AppendMessage("------------------------------------------------------------------------------------------", FALSE, FALSE, TRUE);

	pEdit->LineScroll(nLineCount);

	mnu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND);
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_GET_DATA)->EnableWindow(TRUE);
}

BOOL CNetworkOperationDialog::OnNewMessage(const CString& strMessage, BOOL bAddTimeStamp)
{
	AppendMessage(strMessage, FALSE, bAddTimeStamp, FALSE);

	return TRUE;
}

BOOL CNetworkOperationDialog::OnEmptyDirReceived(CString strDir)
{
	AppendMessage(strDir, FALSE, FALSE, FALSE);

	return TRUE;
}

BOOL CNetworkOperationDialog::OnGetAllDirFilesFromClipboard(CDiskFile& diskFile)
{
	BOOL bCut;

	if (!m_pDiskFileManager->CheckCopyFromClipboard(diskFile, bCut))
		return FALSE;

	vector<CDirectoryInfo> vecDir;
	vector<CFileInfo> vecFile;

	CString strDirName;
	for (int i = 0; i < (int)diskFile.vecDirectory.size(); i++)
	{
		m_pDiskFileManager->GetDirFileListRecursive(diskFile.strWorkDir, TRUE, diskFile.vecDirectory[i]+_T("\\"), vecDir, vecFile, TRUE, _T(""), _T(""));
	}

	for (int i = 0; i < (int)vecDir.size(); i++)
	{
		diskFile.vecDirectory.push_back(vecDir[i].strName);
	}

	for (int i = 0; i < (int)vecFile.size(); i++)
	{
		diskFile.vecFile.push_back(vecFile[i].strName);
	}
   
	return TRUE;
}