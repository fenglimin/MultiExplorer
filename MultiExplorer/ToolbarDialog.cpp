// ToolbarDialog.cpp : implementation file
//

#include "stdafx.h"
#include "MultiExplorer.h"
#include "ToolbarDialog.h"
#include "CommonTool.h"
#include "DiskFileManager.h"
// CToolbarDialog dialog

IMPLEMENT_DYNAMIC(CToolbarDialog, CDialog)

CToolbarDialog::CToolbarDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CToolbarDialog::IDD, pParent)
{
	m_nButtonSize = 36;
	m_pDiskFileManager = NULL;
}

CToolbarDialog::~CToolbarDialog()
{
	DeleteQuickLaunchButtons();
	DeleteDriverButtons();
}

void CToolbarDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CToolbarDialog, CDialog)
	ON_COMMAND_RANGE(BUTTON_COMMAND_ID_BEGIN,BUTTON_COMMAND_ID_BEGIN+MAX_BUTTON_COUNT,OnButtonClick) 
	ON_WM_RBUTTONDBLCLK()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CToolbarDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	HICON hIcon;
	CRect rc;
	list<CToolbarButton*>::iterator it;
	for ( it = m_listQuickLaunchButtons.begin(); it != m_listQuickLaunchButtons.end(); it ++)
	{
		CToolbarButton* pToolbarButton = *it;

		GetButtonRect(pToolbarButton->nButtonID, rc);
		pToolbarButton->pButton = new CButtonST;
		pToolbarButton->pButton->Create(_T(""), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
			rc,	this, BUTTON_COMMAND_ID_BEGIN+pToolbarButton->nButtonID);

		hIcon = GetLargeIconFromFile(pToolbarButton->strButtonFile);
		pToolbarButton->pButton->SetIcon(hIcon,hIcon);
		pToolbarButton->pButton->SetTooltipText(pToolbarButton->strButtonFile);
	}

	m_nNewButtonID = m_listQuickLaunchButtons.size()+1;

	SetTimer(1, 1000, NULL);

	return TRUE;
}

void CToolbarDialog::SetPosition( const CRect& rcWnd )
{
	SetWindowPos(NULL, rcWnd.left, rcWnd.top, rcWnd.Width(), rcWnd.Height(), SWP_SHOWWINDOW);

	list<CToolbarButton*>::iterator it;
	CRect rc;
	CToolbarButton* pToolbarButton;
	int nPos = 0;
	for ( it = m_listQuickLaunchButtons.begin(); it != m_listQuickLaunchButtons.end(); it ++)
	{
		pToolbarButton = *it;

		GetButtonRect(nPos, rc);
		pToolbarButton->pButton->MoveWindow(rc);

		nPos++;
	}

	RefreshDriverButtons();
}

void CToolbarDialog::OnButtonClick( UINT uID )
{
	uID -= BUTTON_COMMAND_ID_BEGIN;

	CheckClick(m_listQuickLaunchButtons, uID);
	CheckClick(m_listDriverButtons, uID);

}

HICON CToolbarDialog::GetLargeIconFromFile( const CString& strFile )
{
	SHFILEINFO    sfi;
	HIMAGELIST hImagelist;
	hImagelist=(HIMAGELIST)SHGetFileInfo(_T(""), 
		0,
		&sfi, 
		sizeof(SHFILEINFO), 
		SHGFI_SYSICONINDEX | SHGFI_LARGEICON ) ;

	CImageList* pImageList = CImageList::FromHandle(hImagelist);

	SHFILEINFO sInfo;
	SHGetFileInfo(strFile, FILE_ATTRIBUTE_NORMAL, &sInfo, sizeof(sInfo),
		SHGFI_USEFILEATTRIBUTES|SHGFI_LARGEICON|SHGFI_SYSICONINDEX);

	HICON hIcon = pImageList->ExtractIcon(sInfo.iIcon);
	DeleteObject(hImagelist);

	return hIcon;
}

BOOL CToolbarDialog::PreTranslateMessage( MSG* pMsg )
{
	BOOL bRetVal=FALSE;

	if ( pMsg->message == WM_RBUTTONDBLCLK )
	{
		HandleRButtonDblClick(pMsg->pt);
		bRetVal = TRUE;
	}
	else if ( pMsg->message == WM_RBUTTONDOWN )
	{
		HandleRButtonClick(pMsg->pt);
		bRetVal = TRUE;
	}

	if (!bRetVal)
		bRetVal=CDialog::PreTranslateMessage(pMsg);

	return bRetVal;
}

void CToolbarDialog::GetButtonRect( int nButtonPos, CRect& rc )
{
	rc.top = 0;
	rc.bottom = m_nButtonSize;
	if ( nButtonPos >= 0 )
	{
		rc.left = m_nButtonSize * nButtonPos;
	}
	else
	{
		CRect rcClient;
		GetWindowRect(&rcClient);
		ScreenToClient(rcClient);
		rc.left = rcClient.right+m_nButtonSize*nButtonPos-2;
	}

	rc.right = rc.left + m_nButtonSize;

}

void CToolbarDialog::HandleRButtonDblClick( POINT pt )
{
	CToolbarButton* pToolbarButton = FindQuickLaunchButton(pt);

	if ( pToolbarButton == NULL )
	{
		int nCount = m_vecDeletedButtonFile.size();
		if (  nCount > 0 )
		{
			CString strButtonFile = m_vecDeletedButtonFile[nCount-1];
			AppendQuickLaunchButton(strButtonFile, TRUE);
			m_vecDeletedButtonFile.pop_back();
		}
	}
	else
	{
		DeleteButton(pToolbarButton);
	}
}

void CToolbarDialog::AppendQuickLaunchButton( const CString& strButtonFile, BOOL bNeedSave )
{
	CRect rc;
	GetButtonRect(m_listQuickLaunchButtons.size(), rc);

	CToolbarButton* pToolbarButton = new CToolbarButton;
	pToolbarButton->bDriverButton = FALSE;
	pToolbarButton->bNeedSave = bNeedSave;
	pToolbarButton->strButtonFile = strButtonFile;
	pToolbarButton->strTooltip = strButtonFile;
	pToolbarButton->nButtonID = m_nNewButtonID;
	pToolbarButton->pButton = new CButtonST;
	pToolbarButton->pButton->Create(_T(""), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
		rc,	this, BUTTON_COMMAND_ID_BEGIN+pToolbarButton->nButtonID);

	HICON hIcon = GetLargeIconFromFile(pToolbarButton->strButtonFile);
	pToolbarButton->pButton->SetIcon(hIcon,hIcon);
	pToolbarButton->pButton->SetTooltipText(pToolbarButton->strTooltip);

	m_listQuickLaunchButtons.insert(m_listQuickLaunchButtons.end(), pToolbarButton);

	m_nNewButtonID++;
}

void CToolbarDialog::DeleteAllButtons(list<CToolbarButton*>& listButton)
{
	while ( listButton.size() > 0 )
	{
		CToolbarButton* pToolbarButton = *listButton.begin();

		pToolbarButton->pButton->DestroyWindow();
		delete pToolbarButton->pButton;
		delete pToolbarButton;

		listButton.remove(pToolbarButton);
	}
}

void CToolbarDialog::AppendDriverButtons(const CDiskDriverInfo& di)
{
	CRect rc;
	int nID = (int)m_listDriverButtons.size()+1;
	GetButtonRect(-nID, rc);

	CToolbarButton* pToolbarButton = new CToolbarButton;
	pToolbarButton->bDriverButton = TRUE;
	pToolbarButton->strButtonFile = di.strDriverLetter;
	pToolbarButton->nButtonID = MAX_BUTTON_COUNT-nID;
	pToolbarButton->pButton = new CButtonST;
	pToolbarButton->pButton->Create(_T(""), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 
		rc,	this, BUTTON_COMMAND_ID_BEGIN+pToolbarButton->nButtonID);

	pToolbarButton->pButton->SetIcon(GetPieIcon(di));
	pToolbarButton->pButton->SetTooltipText(GetDriverTip(di));

	m_listDriverButtons.insert(m_listDriverButtons.end(), pToolbarButton);
}

void CToolbarDialog::CheckClick( list<CToolbarButton*>& listButton, UINT uID )
{
	list<CToolbarButton*>::iterator it;
	for ( it = listButton.begin(); it != listButton.end(); it ++)
	{
		CToolbarButton* pToolbarButton = *it;
		if ( pToolbarButton->nButtonID == uID )
		{
			int nToken = pToolbarButton->strButtonFile.ReverseFind(_T('\\'));
			CString strWorkDir = pToolbarButton->strButtonFile.Left(nToken+1);

			ShellExecute(NULL, _T("Open"), pToolbarButton->strButtonFile, _T(""), strWorkDir, SW_SHOW );
			return;
		}
	}
}

void CToolbarDialog::RefreshDriverButtons()
{
	vector<CDiskDriverInfo> vecDiskDriverInfo = m_pDiskFileManager->m_vecDiskDriverInfoLast;
	if ( vecDiskDriverInfo.size() == 0 )
		return;

	list<CToolbarButton*>::iterator it;
	int nID = 0;
	for ( it = m_listDriverButtons.begin(); it != m_listDriverButtons.end(); it ++)
	{
		nID ++;
		CToolbarButton* pToolbarButton = *it;

		int i;
		for ( i = vecDiskDriverInfo.size()-1; i >= 0; i -- )
		{
			if ( vecDiskDriverInfo[i].strDriverLetter == pToolbarButton->strButtonFile )
			{
				CRect rc;
				GetButtonRect(-nID, rc);
				pToolbarButton->pButton->MoveWindow(rc);

				HICON hIcon = GetPieIcon(vecDiskDriverInfo[i]);
				pToolbarButton->pButton->SetIcon(hIcon);
				pToolbarButton->pButton->SetTooltipText(GetDriverTip(vecDiskDriverInfo[i]));
				break;
			}
		}

		if ( i < 0 )
		{
			it--;

			pToolbarButton->pButton->DestroyWindow();
			delete pToolbarButton->pButton;
			delete pToolbarButton;
			
			m_listDriverButtons.remove(pToolbarButton);
		}
	}

	BOOL bFind;
	for ( int i = 0; i < (int)vecDiskDriverInfo.size(); i ++ )
	{
		bFind = FALSE;
		list<CToolbarButton*>::iterator it;
		for ( it = m_listDriverButtons.begin(); it != m_listDriverButtons.end(); it ++)
		{
			CToolbarButton* pToolbarButton = *it;

			if ( pToolbarButton->strButtonFile == vecDiskDriverInfo[i].strDriverLetter)
			{
				bFind = TRUE;
				break;
			}
		}

		if ( !bFind )
		{
			AppendDriverButtons(vecDiskDriverInfo[i]);
		}
	}
}

HICON CToolbarDialog::GetPieIcon(const CDiskDriverInfo& di)
{
	CCommonTool commomTool;
	vector<CPieChart> vecPieChart;
	vecPieChart.push_back(CPieChart(_T(""), (int)(di.nUsedSpace/1000)));
	if ( di.bAlive)
		vecPieChart.push_back(CPieChart(_T(""), (int)((di.nTotalSpace-di.nUsedSpace)/1000)));
	CBitmap bitmap;
	CRect rcIcon(0, 0, m_nButtonSize-6, m_nButtonSize-6);
	if ( !commomTool.CreatePieChart(bitmap, rcIcon,GetSysColor(CTLCOLOR_DLG), vecPieChart, di.strDriverLetter.Left(2)) )
		return NULL;

	HICON hIcon = commomTool.HICONFromCBitmap(bitmap);

	return hIcon;
}

CString CToolbarDialog::GetDriverTip(const CDiskDriverInfo& di)
{
	CCommonTool commomTool;
	CString strTooltip;

	if ( di.bAlive )
		strTooltip.Format(_M("%s(%s), Total space %s M, Free space %s M"), di.strLabel, di.strDriverLetter.Left(2), 
			commomTool.ConvertToFileSize(di.nTotalSpace/1024/1024), commomTool.ConvertToFileSize((di.nTotalSpace-di.nUsedSpace)/1024/1024));
	else
		strTooltip.Format(_M("%s(%s), Error getting capacity!"), di.strLabel, di.strDriverLetter.Left(2) );

	return strTooltip;
}

CToolbarButton* CToolbarDialog::FindQuickLaunchButton( const POINT& pt )
{
	return FindButton(m_listQuickLaunchButtons, pt);
}

CToolbarButton* CToolbarDialog::FindQuickLaunchButton( const CString& strButtonFile)
{
	list<CToolbarButton*>::iterator it;
	CRect rc;
	CToolbarButton* pToolbarButton;
	for ( it = m_listQuickLaunchButtons.begin(); it != m_listQuickLaunchButtons.end(); it ++)
	{
		pToolbarButton = *it;

		if ( pToolbarButton->strButtonFile == strButtonFile)
			break;
	}

	if ( it == m_listQuickLaunchButtons.end() )
		return NULL;
	else
		return pToolbarButton;
}

void CToolbarDialog::DeleteButton( CToolbarButton* pToolbarButton )
{
	list<CToolbarButton*>::iterator it;
	int nPos = 0;
	for ( it = m_listQuickLaunchButtons.begin(); it != m_listQuickLaunchButtons.end(); it ++)
	{
		if ( pToolbarButton == *it )
			break;

		nPos ++;
	}

	it++;

	// Delete the button
	m_vecDeletedButtonFile.push_back(pToolbarButton->strButtonFile);
	pToolbarButton->pButton->DestroyWindow();
	delete pToolbarButton->pButton;
	delete pToolbarButton;
	m_listQuickLaunchButtons.remove(pToolbarButton);

	// Move the rest button forward
	CRect rc;
	for ( ; it != m_listQuickLaunchButtons.end(); it ++ )
	{
		pToolbarButton = *it;

		GetButtonRect(nPos,rc);
		pToolbarButton->pButton->MoveWindow(rc);

		nPos++;
	}

}

void CToolbarDialog::DeleteButton( const CString& strButtonFile )
{
	CToolbarButton* pToolbarButton = FindQuickLaunchButton(strButtonFile);
	if ( pToolbarButton )
	{
		DeleteButton(pToolbarButton);
	}
}

CToolbarButton* CToolbarDialog::FindButton( list<CToolbarButton*>& listButtons, const POINT& pt )
{
	list<CToolbarButton*>::iterator it;
	CRect rc;
	CToolbarButton* pToolbarButton;
	for ( it = listButtons.begin(); it != listButtons.end(); it ++)
	{
		pToolbarButton = *it;

		pToolbarButton->pButton->GetWindowRect(&rc);
		if ( rc.PtInRect(pt) )
			break;
	}

	if ( it == listButtons.end() )
		return NULL;
	else
		return pToolbarButton;
}

void CToolbarDialog::HandleRButtonClick( POINT pt )
{
	CToolbarButton* pToolbarButton = FindButton(m_listDriverButtons, pt);
	if ( pToolbarButton )
	{
		m_pDiskFileManager->StartNewMultiExplorer(pToolbarButton->strButtonFile);
	}
}

void CToolbarDialog::OnTimer( UINT_PTR nTimerID )
{
	if ( m_pDiskFileManager->IsRefreshDiskDriverThreadActive() )
		return;

	RefreshDriverButtons();

	m_pDiskFileManager->StartRefreshDiskDriverThread();	
}

// CToolbarDialog message handlers

