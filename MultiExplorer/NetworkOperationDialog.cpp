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
	ON_BN_CLICKED(IDC_BUTTON_POST_ACTION, &CNetworkOperationDialog::OnBnClickedButtonPostAction)
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

	m_pDiskFileManager->SetWorkToolClientUser(this);

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
	CString strButtonText;
	GetDlgItemText(IDC_BUTTON_GET_DATA, strButtonText);

	if (strButtonText == _M("Get Clipboard Data"))
	{
		m_pDiskFileManager->m_userOption.bClipboardUnicode = IsDlgButtonChecked(IDC_CHECK_UNICODE);
		m_pDiskFileManager->m_userOption.bAppendMessage = IsDlgButtonChecked(IDC_CHECK_APPEND_MESSAGE);

		POSITION pos = m_listMachine.GetFirstSelectedItemPosition();
		int nRow = m_listMachine.GetNextSelectedItem(pos);

		if (nRow == -1)
		{
			AppendMessage(_M("Please select a machine!"), TRUE, TRUE);
			return;
		}

		m_nRow = nRow;
		AfxBeginThread(GetClipboardDataThreadFunc, (void*)this);
	}
	else
	{
		OnCancelGetClipboardData();
	}

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

	m_pDiskFileManager->SetWorkToolClientUser(NULL);
}

void CNetworkOperationDialog::AppendMessage(CString strMessage, BOOL bAddTimeStamp, BOOL bAppendEndline)
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CLIPBOARD);
	
	int nLineCount = 0;
	CString strExsitingMessage;
	pEdit->GetWindowText(strExsitingMessage);
	nLineCount = pEdit->GetLineCount();


	if (strExsitingMessage.GetLength() != 0)
		strExsitingMessage += "\r\n";

	if (bAddTimeStamp)
	{
		strExsitingMessage += m_pDiskFileManager->m_pWorkTool->GetCurrentFormattedTime(FALSE);
		strExsitingMessage += " ";
	}

	strExsitingMessage += strMessage;

	if (bAppendEndline)
		strExsitingMessage += "\r\n";

	pEdit->SetWindowText(strExsitingMessage);
	pEdit->LineScroll(nLineCount+1);
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

	if (m_pDiskFileManager->m_userOption.bAppendMessage)
		AppendMessage("------------------------------------------------------------------------------------------", FALSE, FALSE);
	else
		SetDlgItemText(IDC_EDIT_CLIPBOARD, "");

	CString strMsg;
	strMsg.Format(_M("Getting clipboard from %s ......"), strIp);
	AppendMessage(strMsg, TRUE, FALSE);

	CString strErrorMsg;
	int nFormat = m_pDiskFileManager->m_userOption.bClipboardUnicode ? CF_UNICODETEXT : CF_TEXT;
	BOOL bOk = m_pDiskFileManager->m_pWorkTool->Request_GetClipboardData(strIp, nPort, nFormat, strErrorMsg);
	if (!bOk)
	{
		AppendMessage(strErrorMsg, TRUE, FALSE);
	}

	AppendMessage(_M("Get clipboard ended!"), TRUE, FALSE);
	if (m_pDiskFileManager->m_userOption.bAppendMessage)
		AppendMessage("------------------------------------------------------------------------------------------", FALSE, TRUE);

	mnu->EnableMenuItem(SC_CLOSE, MF_BYCOMMAND);
	GetDlgItem(IDOK)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_GET_DATA)->EnableWindow(TRUE);
}

void CNetworkOperationDialog::OnCancelGetClipboardData()
{
	m_pDiskFileManager->m_pWorkTool->m_bContinueTransfer = AfxMessageBox(_M("Are you sure to stop transfer?"), MB_YESNO) == IDNO;;
}

BOOL CNetworkOperationDialog::OnNewMessage(const CString& strMessage, BOOL bAddTimeStamp)
{
	AppendMessage(strMessage, bAddTimeStamp, FALSE);

	return TRUE;
}

BOOL CNetworkOperationDialog::OnEmptyDirReceived(CString strTargetDir, CString strDir)
{
	AppendMessage(strDir, FALSE, FALSE);

	return TRUE;
}

BOOL CNetworkOperationDialog::OnStart(BOOL bIsText)
{
	CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_GET_DATA);
	if (!bIsText)
	{
		pButton->EnableWindow(TRUE);
		pButton->SetWindowText("Stop");
	}

	return TRUE;
}

BOOL CNetworkOperationDialog::OnComplete(BOOL bIsText, CString strData)
{
	CString strButtonText = bIsText ? _M("Copy to Clipboard") : _M("Open directory");

	CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_POST_ACTION);
	pButton->SetWindowText(strButtonText);
	pButton->ShowWindow(SW_SHOW);

	m_bLastOpIsText = bIsText;
	m_strLastOpData = strData;

	SetDlgItemText(IDC_BUTTON_GET_DATA, _M("Get Clipboard Data"));

	return TRUE;
}

void CNetworkOperationDialog::OnBnClickedButtonPostAction()
{
	if (m_bLastOpIsText)
	{
		OpenClipboard();

		int buff_size = m_strLastOpData.GetLength();
		CStringW strWide = CStringW(m_strLastOpData);
		int nLen = strWide.GetLength();

		HANDLE clipbuffer;
		char* buffer;
		clipbuffer = ::GlobalAlloc(GMEM_DDESHARE, (nLen + 1) * 2);
		buffer = (char*)::GlobalLock(clipbuffer);
		memset(buffer, 0, (nLen + 1) * 2);
		memcpy_s(buffer, nLen * 2, strWide.GetBuffer(0), nLen * 2);
		strWide.ReleaseBuffer();
		GlobalUnlock(clipbuffer);

		SetClipboardData(CF_UNICODETEXT, clipbuffer);
		CloseClipboard();
	}
	else
	{
		m_pDiskFileManager->StartNewMultiExplorer(m_strLastOpData);
	}
}
