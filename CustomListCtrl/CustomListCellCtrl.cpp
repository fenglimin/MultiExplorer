#include "stdafx.h"
#include "CustomListCellCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CFilterEdit

CFilterDateTime::CFilterDateTime()
{
	m_bEditTime = FALSE;
	m_bNotifySent = FALSE;
	m_cellType = cellTextDate;
}

CFilterDateTime::~CFilterDateTime()
{
}


BEGIN_MESSAGE_MAP(CFilterDateTime, CDateTimeCtrl)
	//{{AFX_MSG_MAP(CFilterSortHeaderCtrl::CFilterEdit)
	ON_WM_KILLFOCUS()
	ON_NOTIFY_REFLECT(DTN_DATETIMECHANGE,OnDateTimeChanged)
	ON_NOTIFY_REFLECT(DTN_CLOSEUP, OnCloseup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilterEdit message handlers

void CFilterDateTime::OnKillFocus(CWnd* pNewWnd) 
{
	CDateTimeCtrl::OnKillFocus(pNewWnd);

	
	if ( !m_bNotifySent && CDateTimeCtrl::GetMonthCalCtrl() == NULL )
	{
		(GetParent())->SendMessage(FLM_DATETIMECHANGED,TRUE,0);
	}
}

void CFilterDateTime::PostNcDestroy()
{
	delete this;
}

void CFilterDateTime::OnDateTimeChanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	*pResult = 0;
}

void CFilterDateTime::OnCloseup(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
}

BOOL CFilterDateTime::PreTranslateMessage(MSG* pMsg) 
{
	BOOL bRetVal=FALSE;

	if (pMsg->message==WM_KEYDOWN)
	{
		if (pMsg->wParam==VK_RETURN) // Enter - input is ok
		{
			m_bNotifySent=TRUE;

			(GetParent())->SendMessage(FLM_DATETIMECHANGED,TRUE,1);

			bRetVal=TRUE;
		}
		else if (pMsg->wParam==VK_ESCAPE) // ESCAPE - ignore input
		{
			m_bNotifySent=TRUE;

			(GetParent())->SendMessage(FLM_DATETIMECHANGED,FALSE,0);

			bRetVal=TRUE;
		}
	}

	if (!bRetVal)
		bRetVal=CDateTimeCtrl::PreTranslateMessage(pMsg);

	return bRetVal;
}

/////////////////////////////////////////////////////////////////////////////
// CFilterEdit

CFilterEdit::CFilterEdit()
{
	m_bNotifySent=FALSE;
	m_strEditableChars = _T("~!@#$%^&*()_+{}|:\"<>?`-=[]\\;',./1234567890abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
}

CFilterEdit::~CFilterEdit()
{
}


BEGIN_MESSAGE_MAP(CFilterEdit, CEdit)
	//{{AFX_MSG_MAP(CFilterSortHeaderCtrl::CFilterEdit)
	ON_WM_KILLFOCUS()
	ON_WM_CHAR()
	ON_WM_KEYDOWN()
	ON_WM_RBUTTONUP()
	ON_MESSAGE(WM_PASTE, OnPaste)
	ON_MESSAGE(WM_COPY, OnCopy)
	ON_MESSAGE(WM_CUT, OnCut)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilterEdit message handlers

void CFilterEdit::OnRButtonUp(UINT nFlags, CPoint point)
{
	return CEdit::OnRButtonUp(nFlags, point);
}

void CFilterEdit::OnKillFocus(CWnd* pNewWnd) 
{
	CEdit::OnKillFocus(pNewWnd);

	if (!m_bNotifySent)
	{
		m_bNotifySent=TRUE;

		(GetParent())->SendMessage(FLM_EDITTEXTCHANGED,TRUE,0);
	}
}

void CFilterEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default

	if ((m_strValidChars.IsEmpty()) || ((-1 != m_strValidChars.Find(static_cast<TCHAR> (nChar))) || 
		(nChar == VK_BACK) || (nChar == VK_DELETE) || (nChar == CTRL_C) || (nChar == CTRL_V) || (nChar == CTRL_X)))
	{
		CEdit::OnKeyDown(nChar, nRepCnt, nFlags);

		if ( nChar == VK_DELETE )
		{
			(GetParent())->SendMessage(FLM_EDITTEXTCHANGING,FALSE,0);
		}

		
	}
	else
	{
		MessageBeep(MB_ICONEXCLAMATION);
		return;
	}
}

void CFilterEdit::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default

	if ((m_strValidChars.IsEmpty()) || ((-1 != m_strValidChars.Find(static_cast<TCHAR> (nChar))) || 
		(nChar == VK_BACK) || (nChar == VK_DELETE) || (nChar == CTRL_C) || (nChar == CTRL_V) || (nChar == CTRL_X)))
	{
		CEdit::OnChar(nChar, nRepCnt, nFlags);

		if ( nChar != CTRL_C )
		{
			(GetParent())->SendMessage(FLM_EDITTEXTCHANGING,FALSE,0);
		}


	}
	else
	{
		MessageBeep(MB_ICONEXCLAMATION);
		return;
	}
}

void CFilterEdit::SetValidChars(CString& rstrValidChars)
{
	m_strValidChars = rstrValidChars;	
}

void CFilterEdit::PostNcDestroy()
{
	delete this;
}



BOOL CFilterEdit::PreTranslateMessage(MSG* pMsg) 
{
	BOOL bRetVal=FALSE;

	if (pMsg->message==WM_KEYDOWN)
	{
		CString strMsg;
		strMsg.Format("message is %d\n", pMsg->wParam);
		TRACE(strMsg);

		if (pMsg->wParam==VK_RETURN) // Enter - input is ok
		{
			m_bNotifySent=TRUE;

			(GetParent())->SendMessage(FLM_EDITTEXTCHANGED,TRUE,1);

			bRetVal=TRUE;
		}
		else if (pMsg->wParam==VK_ESCAPE) // ESCAPE - ignore input
		{
			m_bNotifySent=TRUE;

			(GetParent())->SendMessage(FLM_EDITTEXTCHANGED,FALSE,0);

			bRetVal=TRUE;
		}
		//else if (pMsg->wParam==VK_HOME) 
		//{
		//	SetSel(0,0);
		//	bRetVal=TRUE;
		//}
		//else if (pMsg->wParam==VK_END) 
		//{
		//	int nLen = GetWindowTextLength();
		//	SetSel(nLen,nLen);
		//	bRetVal=TRUE;
		//}
		//else if (pMsg->wParam==VK_DELETE)
		//{
		//	int nStartChar, nEndChar;
		//	GetSel(nStartChar,nEndChar);
		//	CString strText;
		//	GetWindowText(strText);
		//	strText = strText.Left(nStartChar) + strText.Right(strText.GetLength()-nEndChar-1);
		//	SetWindowText(strText);
		//	SetSel(nStartChar,nStartChar);
		//	(GetParent())->SendMessage(FLM_EDITTEXTCHANGING,FALSE,0);
		//	bRetVal=TRUE;
		//}
	}

	if (!bRetVal)
		bRetVal=CEdit::PreTranslateMessage(pMsg);

	return bRetVal;
}

LRESULT CFilterEdit::OnPaste( WPARAM wParam, LPARAM lParam )
{
	OpenClipboard();

	HGLOBAL hMem = NULL;
	LPCTSTR lpStr = NULL;

	hMem = GetClipboardData(CF_UNICODETEXT);
	LPCWSTR pData = (LPCWSTR)::GlobalLock(hMem);

	DWORD dwMinSize = WideCharToMultiByte(CP_OEMCP,NULL,pData,-1,NULL,0,NULL,FALSE);
	char* pChar = new char[dwMinSize];
	WideCharToMultiByte(CP_OEMCP,NULL,pData,-1,pChar,dwMinSize,NULL,FALSE);

	::GlobalUnlock(hMem);
	CloseClipboard();

	CString strClipboardText = pChar;
	delete pChar;

	int nStartChar, nEndChar;
	GetSel(nStartChar,nEndChar);
	CString strText;
	GetWindowText(strText);
	strText = strText.Left(nStartChar) + strClipboardText + strText.Right(strText.GetLength()-nEndChar-1);
	SetWindowText(strText);

	int nCursor = nStartChar + strClipboardText.GetLength();
	SetSel(nCursor,nCursor);

	return 0;
}

LRESULT CFilterEdit::OnCopy( WPARAM wParam, LPARAM lParam )
{
	int nStartChar, nEndChar;
	GetSel(nStartChar,nEndChar);
	CString strText;
	GetWindowText(strText);

	CString strCopyText = strText.Mid(nStartChar, nEndChar-nStartChar);
	CopyToClipboard(strCopyText);

	return 0;
}

LRESULT CFilterEdit::OnCut( WPARAM wParam, LPARAM lParam )
{
	int nStartChar, nEndChar;
	GetSel(nStartChar,nEndChar);
	CString strText;
	GetWindowText(strText);
	CString strLeftText = strText.Left(nStartChar) + strText.Right(strText.GetLength()-nEndChar);
	SetWindowText(strLeftText);
	SetSel(nStartChar,nStartChar);

	CString strCopyText = strText.Mid(nStartChar, nEndChar-nStartChar);
	CopyToClipboard(strCopyText);

	return 0;
}

void CFilterEdit::CopyToClipboard( const CString& strText )
{
	OpenClipboard();
	EmptyClipboard();

	int buff_size = strText.GetLength() ;
	CStringW strWide = CStringW(strText);
	int nLen = strWide.GetLength();

	HANDLE clipbuffer ;
	char* buffer;
	clipbuffer = ::GlobalAlloc( GMEM_DDESHARE, (nLen + 1) * 2 ) ;
	buffer = (char*)::GlobalLock(clipbuffer) ;
	memset(buffer, 0, (nLen + 1) * 2);
	memcpy_s(buffer, nLen * 2, strWide.GetBuffer(0), nLen* 2 );
	strWide.ReleaseBuffer();
	GlobalUnlock(clipbuffer) ;

	SetClipboardData(CF_UNICODETEXT, clipbuffer) ;
	CloseClipboard() ;
}

// End of CFilterEdit
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// CFilterCombo

CFilterCombo::CFilterCombo()
{
	m_bNotifySent=FALSE;
	m_bDropdown = FALSE;
}

CFilterCombo::~CFilterCombo()
{
}


BEGIN_MESSAGE_MAP(CFilterCombo, CComboBox)
	//{{AFX_MSG_MAP(CFilterSortHeaderCtrl::CFilterCombo)
	ON_WM_KILLFOCUS()
	ON_CONTROL_REFLECT(CBN_CLOSEUP, CFilterCombo::OnCloseup)
	ON_CONTROL_REFLECT(CBN_SELCHANGE, &CFilterCombo::OnSelectChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
////CFilterCombo message handlers

void CFilterCombo::OnCloseup() 
{
	// Set the focus to the parent list control
	if (!m_bNotifySent)
	{
		m_bNotifySent=TRUE;

		(GetParent())->SendMessage(FLM_COMBOSELCHANGED,TRUE,0);
	}

	//	GetParent()->SetFocus();
	//	int i = 0;123456

}

void CFilterCombo::OnSelectChanged() 
{
	// Set the focus to the parent list control
	if (!m_bNotifySent)
	{
		m_bNotifySent=TRUE;

		(GetParent())->SendMessage(FLM_COMBOSELCHANGED,TRUE,0);
	}

	//	GetParent()->SetFocus();
	//	int i = 0;123456

}

void CFilterCombo::OnKillFocus(CWnd* pNewWnd) 
{
	CComboBox::OnKillFocus(pNewWnd);
	if (!m_bNotifySent)
	{
		m_bNotifySent=TRUE;

		(GetParent())->SendMessage(FLM_COMBOSELCHANGED,TRUE,0);
	}
}

void CFilterCombo::PostNcDestroy()
{
	delete this;
}

BOOL CFilterCombo::PreTranslateMessage(MSG* pMsg)
{
	BOOL bRetVal=FALSE;

	if (pMsg->message==WM_KEYDOWN)
	{
		if (pMsg->wParam==VK_RETURN) // Enter - input is ok
		{
			m_bNotifySent=TRUE;

			(GetParent())->SendMessage(FLM_COMBOSELCHANGED,TRUE,1);

			bRetVal=TRUE;
		}
		else if (pMsg->wParam==VK_ESCAPE) // ESCAPE - ignore input
		{
			m_bNotifySent=TRUE;

			(GetParent())->SendMessage(FLM_COMBOSELCHANGED,FALSE,0);

			bRetVal=TRUE;
		}
		else if (pMsg->wParam==VK_DOWN) // ESCAPE - ignore input
		{
			SetCurSel(GetCurSel()+1);
			(GetParent())->SendMessage(FLM_COMBOSELCHANGING,FALSE,0);
			bRetVal=TRUE;
		}
		else if (pMsg->wParam==VK_UP) // ESCAPE - ignore input
		{
			int nCur = GetCurSel();
			if ( nCur > 0 )
				SetCurSel(nCur-1);

			(GetParent())->SendMessage(FLM_COMBOSELCHANGING,FALSE,0);
			bRetVal=TRUE;
		}
//		VK_LEFT
	}

	if (!bRetVal)
		bRetVal=CComboBox::PreTranslateMessage(pMsg);

	return bRetVal;
}