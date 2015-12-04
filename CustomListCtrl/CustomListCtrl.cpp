
#include "stdafx.h"
#include "CustomListCtrl.h"
#include "DynDialogEx.h"

IMPLEMENT_DYNAMIC(CCustomListCtrl, CListCtrl)

int CALLBACK BrowseCallbackProc(HWND hwnd,UINT uMsg,LPARAM lParam,LPARAM lpData)   
{
	if(uMsg == BFFM_INITIALIZED)
	{   
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}

	return 0;   
} 


CCustomListCtrl::CCustomListCtrl()
{
	m_nFontHeight = 16;
	m_nRowHeight = 36;
	m_nNumColumns = 0;

	m_pDateTime = NULL;
	m_pEdit = NULL;
	m_pCombo = NULL;
	m_bEndEditSent = TRUE;

	m_nEditRow = -1;
	m_nEditCol = -1;

	m_bSortAscending = TRUE;
	m_nSortColumn = -1;

	

	m_strEmptyMarkupText = _T("There is NO item in the list!");

	m_bEditNextCellAfterReturnIsHit = FALSE;
	m_bCellButtonClicked = FALSE;
	m_bShowSelection = TRUE;
	m_bIgnoreMouse = FALSE;
	m_bAllowMultlSelection = FALSE;
	m_bHighlightChangedCellText = TRUE;
	m_bSelectThenEdit = FALSE;
	m_bDoubleClickEmptyAreaToAddRow = FALSE;
	m_bEnablePopupMenu = TRUE;

	m_hCellTextFont = NULL;

	m_bReadOnly = FALSE;
	m_bFilterInCase = FALSE;
	
	m_bShowTimeCtrlForDateTimeCell = FALSE;
	m_bChangeCellTextRejected = FALSE;

	SetGridLineColor(RGB(121,121,121), RGB(201,201,201));

	m_p2DCache = NULL;
	m_pUser = NULL;

	CustomListContextMenu contextMenu;
	contextMenu.CreateDefaultMenu();
	SetContextMenu(contextMenu);

	LoadDefaultIcon();

	m_strHighlightText = _T("");
}

CCustomListCtrl::~CCustomListCtrl()
{
	if ( m_penHorLine.m_hObject )
	{
		m_penHorLine.DeleteObject();
	}

	if ( m_penVerLine.m_hObject )
	{
		m_penVerLine.DeleteObject();
	}

	if ( m_hCellTextFont )
	{
		DeleteObject(m_hCellTextFont);
	}
}


BEGIN_MESSAGE_MAP(CCustomListCtrl, CListCtrl)
	ON_WM_MEASUREITEM()
	ON_WM_MEASUREITEM_REFLECT()
//	ON_NOTIFY_REFLECT_EX(NM_DBLCLK, OnDoubleClick)
	ON_NOTIFY_REFLECT(LVN_COLUMNCLICK, OnColumnClick)
	ON_MESSAGE(FLM_DATETIMECHANGED,OnDateTimeChanged)
	ON_MESSAGE(FLM_EDITTEXTCHANGED,OnEditTextChanged)
	ON_MESSAGE(FLM_COMBOSELCHANGED,OnComboSelChanged)
	ON_WM_RBUTTONDOWN()
	ON_WM_MBUTTONDOWN()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEWHEEL()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_CHAR()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEHWHEEL()
END_MESSAGE_MAP()



// CCustomListCtrl 消息处理程序
void CCustomListCtrl::PostNcDestroy()
{

}

void CCustomListCtrl::OnDestroy()
{
	DeleteAllItems(FALSE);
	CListCtrl::OnDestroy();
}

namespace {
	LRESULT EnableWindowTheme(HWND hwnd, LPCWSTR classList, LPCWSTR subApp, LPCWSTR idlist)
	{
		//HMODULE hinstDll;
		//HRESULT (__stdcall *pSetWindowTheme)(HWND hwnd, LPCWSTR pszSubAppName, LPCWSTR pszSubIdList);
		//HANDLE (__stdcall *pOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
		//HRESULT (__stdcall *pCloseThemeData)(HANDLE hTheme);

		//hinstDll = ::LoadLibrary("UxTheme.dll");
		//if (hinstDll)
		//{
		//	(FARPROC&)pOpenThemeData = ::GetProcAddress(hinstDll, TEXT("OpenThemeData"));
		//	(FARPROC&)pCloseThemeData = ::GetProcAddress(hinstDll, TEXT("CloseThemeData"));
		//	(FARPROC&)pSetWindowTheme = ::GetProcAddress(hinstDll, TEXT("SetWindowTheme"));
		//	::FreeLibrary(hinstDll);
		//	if (pSetWindowTheme && pOpenThemeData && pCloseThemeData)
		//	{
		//		HANDLE theme = pOpenThemeData(hwnd,classList);
		//		if (theme!=NULL)
		//		{
		//			VERIFY(pCloseThemeData(theme)==S_OK);
		//			return pSetWindowTheme(hwnd, subApp, idlist);
		//		}
		//	}
		//}
		return S_FALSE;
	}
}

void CCustomListCtrl::PreSubclassWindow()
{
	SetMultipleSelection(FALSE);

	// TODO: 在此添加专用代码和/或调用基类
	ModifyStyle(0,LVS_OWNERDRAWFIXED );

//	SetWindowLong(m_hWnd ,GWL_EXSTYLE,WS_EX_CLIENTEDGE);
//	SetExtendedStyle(LVS_EX_GRIDLINES);                     //设置扩展风格为网格
	::SendMessage(m_hWnd, LVM_SETEXTENDEDLISTVIEWSTYLE,
		LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

	

	CListCtrl::PreSubclassWindow();
	CHeaderCtrl *pHeader = GetHeaderCtrl();
	m_ctrlHeader.SubclassWindow(pHeader->GetSafeHwnd());

	// Enable Vista-look if possible
	EnableWindowTheme(GetSafeHwnd(), L"ListView", L"Explorer", NULL);
}

void CCustomListCtrl::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CListCtrl::OnMouseHWheel(nFlags,zDelta,pt);
	HideAllCellCtrl(FALSE);
}

BOOL CCustomListCtrl::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	BOOL bOk = CListCtrl::OnMouseWheel(nFlags,zDelta,pt);
	HideAllCellCtrl(FALSE);

	return bOk;
}

void CCustomListCtrl::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	CListCtrl::OnChar(nChar,nRepCnt,nFlags);

	if ( (nChar == CTRL_C) || (nChar == CTRL_V) || (nChar == CTRL_X) || (nChar == CTRL_A) )
	{
		if ( m_pUser )
		{
			m_pUser->OnKeyHitted(this, nChar);
		}
	}
}

BOOL CCustomListCtrl::PreTranslateMessage(MSG* pMsg) 
{
	BOOL bRetVal=FALSE;

	if (pMsg->message==WM_KEYDOWN )
	{
		if (m_pEdit == NULL && m_pCombo == NULL && m_pDateTime == NULL && 
			m_ctrlHeader.m_pEdit == NULL && m_ctrlHeader.m_pCombo == NULL)
		{
			if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_DELETE || 
				pMsg->wParam == VK_F5 || pMsg->wParam == VK_BACK )
			{
				if ( m_pUser )
				{
					m_pUser->OnVirtualKeyHitted(this, pMsg->wParam);
				}
				bRetVal=TRUE;
			}
		}
	}

	if (!bRetVal)
		bRetVal=CListCtrl::PreTranslateMessage(pMsg);

	return bRetVal;
}

void CCustomListCtrl::SetGridLineColor(COLORREF colorHorLine, COLORREF colorVerLine)
{
	if ( m_penHorLine.m_hObject )
	{
		m_penHorLine.DeleteObject();
	}
	m_penHorLine.CreatePen(PS_SOLID, 1, colorHorLine);

	if ( m_penVerLine.m_hObject )
	{
		m_penVerLine.DeleteObject();
	}
	m_penVerLine.CreatePen(PS_SOLID, 1, colorVerLine);
}

BOOL CCustomListCtrl::SetCellText(int nRow, int nCol, const CString& strCellText, BOOL bUpdateCache, BOOL bMarkChanged)
{
	CString strOldText = CListCtrl::GetItemText(nRow,nCol);
	if ( !CListCtrl::SetItemText(nRow,nCol,strCellText))
		return FALSE;

	RowItemData* pRowItemData = reinterpret_cast<RowItemData*>(GetItemData(nRow));
	ASSERT(pRowItemData);
	pRowItemData->SetCellText(nCol,strCellText);

	if ( bMarkChanged && strOldText.Compare(strCellText) != 0 )
	{
		CellFormat* pCellFormat = GetCellFormat(nRow,nCol, TRUE);
		pCellFormat->bCellTextChange = TRUE;
	}

	if ( bUpdateCache && m_p2DCache)
	{
		m_p2DCache->CacheSetItemText(pRowItemData->nCacheIndex, nCol, strCellText);
	}

	return TRUE;
}

void CCustomListCtrl::SetCell(int nRow, int nCol, CString strCellText, CellFormat* pCellFormat)
{
	RowItemData* pRowItemData = reinterpret_cast<RowItemData*>(GetItemData(nRow));
	ASSERT(pRowItemData);

	if ( pCellFormat )
	{
		pRowItemData->SetCellFormat(nCol,pCellFormat);
	}

	SetCellText(nRow, nCol, strCellText, TRUE, FALSE);
}

BOOL CCustomListCtrl::SetItemText(int nRow, int nCol, CString strCellText)
{
	ASSERT(FALSE);
	return FALSE;
}

HFONT CCustomListCtrl::SetTextFontWeight(CDC* pDC, LONG lfWeight)
{
	LOGFONT lf = { 0 };
	LOGFONT prev_lf = { 0 };

	CFont* cf = GetFont();
	if (cf != NULL)
	{
		VERIFY(cf->GetObject(sizeof(lf), &lf));
	}
	else
	{
		HFONT hFont = (HFONT)::GetCurrentObject(pDC->m_hDC, OBJ_FONT);	//+++1.1
		if (hFont)
			GetObject(hFont, sizeof(lf), &lf);
		else
			GetObject(GetStockObject(SYSTEM_FONT), sizeof(lf), &lf);
	}

	lf.lfHeight = m_nFontHeight;
	lf.lfWeight = lfWeight;

	if ( m_hCellTextFont )
	{
		DeleteObject(m_hCellTextFont);
	}
	m_hCellTextFont = CreateFontIndirect(&lf);
	ASSERT(m_hCellTextFont);
	HFONT hOldFont = (HFONT) pDC->SelectObject( m_hCellTextFont);

	return hOldFont;
}

void CCustomListCtrl::DrawCellText(CDC* pDC, CellFormat* pCellFormat, CString strCellText, CRect rcItem, BOOL bSelected )
{
	COLORREF colorText = pCellFormat->colorText;
	if ( pCellFormat->bCellTextChange && m_bHighlightChangedCellText)
		colorText = COLOR_CHANGED_CELL_TEXT;

	HFONT hOldFont = SetTextFontWeight(pDC, pCellFormat->lfWeight);

	pCellFormat->rcCellText = rcItem;

	if ( pCellFormat->bEnabled )
	{
		DrawCellText(pDC, strCellText, rcItem, pCellFormat->uTextAlign, colorText, bSelected, TRUE, pCellFormat->bShowHighlight );
	}
	else
	{
		OffsetRect(&rcItem, 1, 1);
		DrawCellText(pDC, strCellText, rcItem, pCellFormat->uTextAlign, GetSysColor (COLOR_3DHILIGHT), bSelected, FALSE, pCellFormat->bShowHighlight );

		OffsetRect(&rcItem, -1, -1);
		DrawCellText(pDC, strCellText, rcItem, pCellFormat->uTextAlign, GetSysColor (COLOR_GRAYTEXT), bSelected, FALSE, pCellFormat->bShowHighlight );
	}

	pDC->SelectObject(hOldFont);
}

void CCustomListCtrl::DrawCellText(CDC* pDC, CString strCellText, CRect rcItem, UINT uTextAlign, COLORREF colorText, BOOL bSelected, BOOL bCheckSelection, BOOL bShowHighlight )
{
	// Restore Font
//	pDC->SelectObject(GetStockObject(DEFAULT_GUI_FONT));

	// Calculate the text position
	UINT   uFormat    = uTextAlign; ;
	TEXTMETRIC metric;
	pDC->GetTextMetrics(&metric);

	int ofst;
	ofst = rcItem.Height() - metric.tmHeight;
	rcItem.OffsetRect(4,ofst/2);
	rcItem.right -= 6;

	COLORREF crTextColor;
	if ( bSelected && bCheckSelection && m_bShowSelection)
	{
		crTextColor = GetSysColor(COLOR_HIGHLIGHTTEXT);
	}
	else
	{
		crTextColor = colorText;
	}	

	pDC->SetBkMode(TRANSPARENT);

	// Draw Text
	CString strShowText = strCellText;
	strShowText.Replace(_T("&"), _T("&&"));

	if ( bShowHighlight && !m_strHighlightText.IsEmpty() )
	{
		CString strLowerText = strShowText;
		strLowerText.MakeLower();
		int nStartToken = strLowerText.Find(m_strHighlightText);
		if ( nStartToken != -1 )
		{
			int nEndToken = nStartToken + m_strHighlightText.GetLength();

			CString strBefore = strShowText.Left(nStartToken);
			CSize szBefore;
			GetTextExtentPoint32(pDC->GetSafeHdc(), strBefore,strBefore.GetLength(),&szBefore);
			CRect rcBefore = rcItem;
			rcBefore.right = rcBefore.left + szBefore.cx;
			pDC->SetTextColor(crTextColor);
			pDC->DrawText(strBefore, strBefore.GetLength(), &rcBefore, uFormat);

			CString strToken = strShowText.Mid(nStartToken, nEndToken-nStartToken);
			CSize szToken;
			GetTextExtentPoint32(pDC->GetSafeHdc(), strToken,strToken.GetLength(),&szToken);
			CRect rcToken = rcBefore;
			rcToken.left = rcBefore.right;
			rcToken.right = rcToken.left + szToken.cx;
			pDC->SetTextColor(RGB(255,0,0));
			pDC->DrawText(strToken, strToken.GetLength(), &rcToken, uFormat);

			CString strAfter = strShowText.Right(strShowText.GetLength()-nEndToken);
			CSize szAfter;
			GetTextExtentPoint32(pDC->GetSafeHdc(), strAfter,strAfter.GetLength(),&szAfter);
			CRect rcAfter = rcToken;
			rcAfter.left = rcToken.right;
			rcAfter.right = rcAfter.left + szAfter.cx;
			pDC->SetTextColor(crTextColor);
			pDC->DrawText(strAfter, strAfter.GetLength(), &rcAfter, uFormat);

			return;
		}
	}

	pDC->DrawText(strShowText, strShowText.GetLength(), &rcItem, uFormat);
}

CellFormat*	CCustomListCtrl::GetRowFormat(int nRow, BOOL bGetForUpdate)
{
	RowItemData* pRowItemData = reinterpret_cast<RowItemData*>(GetItemData(nRow));
	ASSERT(pRowItemData != NULL);
	CellFormat* pRowFormat = pRowItemData->pDefaultRowFormat;

	// If the row format do NOT set, use the default format of the whole list.
	if ( pRowFormat == NULL )
	{
		if ( bGetForUpdate )
		{
			pRowFormat = m_defaultListFormat.CopyFormat(levelRow);
			pRowItemData->pDefaultRowFormat = pRowFormat;
		}
		else
		{
			pRowFormat = &m_defaultListFormat;
		}
	}
	else
	{
		ASSERT(pRowFormat->formatLevel == levelRow);
	}

	return pRowFormat;
}

CellFormat*	CCustomListCtrl::GetCellFormat(int nRow, int nCol, BOOL bGetForUpdate)
{
	RowItemData* pRowItemData = reinterpret_cast<RowItemData*>(GetItemData(nRow));
	ASSERT(pRowItemData != NULL);
	CellFormat* pCellFormat = pRowItemData->FindCellFormat(nCol, FALSE);

	// If the cell format do NOT set, use the default format of the row.
	if ( pCellFormat == NULL )
	{
		CellFormat* pRowFormat = GetRowFormat(nRow);
		if ( bGetForUpdate )
		{
			pCellFormat = pRowFormat->CopyFormat(levelCell);
			pRowItemData->SetCellFormat(nCol, pCellFormat);
		}
		else
		{
			pCellFormat = pRowFormat;
		}
	}
	else
	{
		ASSERT(pCellFormat->formatLevel == levelCell);
	}

	return pCellFormat;
}

void CCustomListCtrl::SetCellFormat(int nRow, int nCol, CellFormat* pCellFormat)
{
	RowItemData* pRowItemData = reinterpret_cast<RowItemData*>(GetItemData(nRow));
	ASSERT(pRowItemData);

	pRowItemData->SetCellFormat(nCol,pCellFormat);
}

void CCustomListCtrl::DrawCellButton(CDC* pDC, CellFormat* pCellFormat, BOOL bPushed)
{
	UINT uFrameCtrl = DFCS_BUTTONPUSH;
	if ( bPushed )
	{
		uFrameCtrl |= DFCS_PUSHED;
	}

	CRect rcCtrl = pCellFormat->rcCellCtrl;
	pDC->DrawFrameControl (rcCtrl, DFC_BUTTON , uFrameCtrl);

	CRect rcButtonText = rcCtrl;
	rcButtonText.DeflateRect(2,2,2,2);

	if ( pCellFormat->bEnabled )
	{
		DrawCellText(pDC, pCellFormat->strCellCtrlText, rcButtonText, DT_CENTER, RGB(0,0,0), FALSE, FALSE, FALSE);
	}
	else
	{
		OffsetRect(&rcButtonText, 1, 1);
		DrawCellText(pDC, pCellFormat->strCellCtrlText, rcButtonText, DT_CENTER, GetSysColor (COLOR_3DHILIGHT), FALSE, FALSE, FALSE );

		OffsetRect(&rcButtonText, -1, -1);
		DrawCellText(pDC, pCellFormat->strCellCtrlText, rcButtonText, DT_CENTER, GetSysColor (COLOR_GRAYTEXT), FALSE, FALSE, FALSE );
	}
}

void CCustomListCtrl::DrawCellCtrl(CDC* pDC, CellFormat* pCellFormat, int nRow, int nCol, CRect rcCtrl, LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	UINT uFrameCtrl = 0;
	pCellFormat->rcCellCtrl = rcCtrl;

	if ( pCellFormat->bEnabled == FALSE )
	{
		uFrameCtrl |= DFCS_INACTIVE;
	}

	if ( pCellFormat->cellType == cellCtrlRadioButton )
	{
		uFrameCtrl |= DFCS_BUTTONRADIO;
	}
	else if ( pCellFormat->cellType == cellCtrlCheckBox)
	{
		uFrameCtrl |= DFCS_BUTTONCHECK;
	}
	else
	{
		return;
	}

	if ( pCellFormat->bChecked && pCellFormat->bEnabled )
	{
		uFrameCtrl |= DFCS_CHECKED;
	}
	pDC->DrawFrameControl (rcCtrl, DFC_BUTTON , uFrameCtrl);

}

void CCustomListCtrl::DrawCellBMP(CDC* pDC, CellFormat* pCellFormat, CRect& rcCtrl, BOOL bSetCtrlRect)
{
	POINT p;
	CBitmap* bmpIcon;		
	HBITMAP hBmp = (HBITMAP)pCellFormat->hPicture;
	if ( hBmp == NULL )
	{
		hBmp = m_ctrlHeader.m_hPicLoadFailed;
	}
	bmpIcon = CBitmap::FromHandle(hBmp);	

	//CBitmap bmpIcon;
	//bmpIcon.Attach(pCellFormat->hBitmap);

	BITMAP bmp;
	bmpIcon->GetBitmap( &bmp );
	p.x = rcCtrl.left + 4;

	int nDrawWidth, nDrawHeight;
	if ( rcCtrl.Height() > bmp.bmHeight)
	{
		p.y = rcCtrl.top + (rcCtrl.Height() - bmp.bmHeight)/2;
		nDrawHeight = bmp.bmHeight;
	}
	else
	{
		p.y = rcCtrl.top + 2;
		nDrawHeight = rcCtrl.Height() - 4;
	}

	nDrawWidth = (int) (nDrawHeight * bmp.bmWidth / bmp.bmHeight);
	if ( nDrawWidth > rcCtrl.Width() - 8 )
	{
		nDrawWidth = rcCtrl.Width() - 8;
		nDrawHeight = (int) ( nDrawWidth * bmp.bmHeight / bmp.bmWidth);
	}

	CDC tempDc;
	tempDc.CreateCompatibleDC(pDC);
	tempDc.SelectObject( bmpIcon );

	pDC->SetStretchBltMode(COLORONCOLOR);
	pDC->StretchBlt(p.x, p.y, nDrawWidth, nDrawHeight, &tempDc, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY );

	tempDc.DeleteDC();

	rcCtrl.left = p.x;
	rcCtrl.top = p.y;
	rcCtrl.right = rcCtrl.left + nDrawWidth;
	rcCtrl.bottom = rcCtrl.top + nDrawHeight;

	if ( bSetCtrlRect )
	{
		pCellFormat->rcCellCtrl = rcCtrl;
	}
}

void CCustomListCtrl::DrawCellICON(CDC* pDC, CellFormat* pCellFormat, CRect& rcCtrl, BOOL bSetCtrlRect)
{
	if ( pCellFormat->hPicture )
	{
		int nIconSize = rcCtrl.Height() - 4;

		rcCtrl.left = rcCtrl.left + 4;
		rcCtrl.top = rcCtrl.top + 2;
		rcCtrl.bottom = rcCtrl.top + nIconSize;
		rcCtrl.right = rcCtrl.left + nIconSize;

		DrawIconEx(pDC->m_hDC, rcCtrl.left,rcCtrl.top, (HICON)pCellFormat->hPicture, 
			nIconSize, nIconSize, 0, NULL, DI_NORMAL);

		if ( bSetCtrlRect )
		{
			pCellFormat->rcCellCtrl = rcCtrl;
		}
	}
	else
	{
		DrawCellBMP(pDC,pCellFormat,rcCtrl, bSetCtrlRect);
	}



	//ICONINFO   iconinfo; 
	////   obtain   the   Icon   somehow 
	//::GetIconInfo((HICON)pCellFormat->hPicture,   &iconinfo); 

	//BITMAP   bmpInfo;
	//GetObject(iconinfo.hbmColor,   sizeof(BITMAP),   &bmpInfo); 
}

void CCustomListCtrl::DrawCellBorder(CDC* pDC, CRect rcCell, BOOL bRightMost)
{
	CPen* pOldPen = pDC->SelectObject(&m_penHorLine);
	pDC->MoveTo(rcCell.left,rcCell.top);
	pDC->LineTo(rcCell.right-1,rcCell.top);

	if ( !bRightMost )
	{
		pDC->SelectObject(&m_penVerLine);
	}
	pDC->MoveTo(rcCell.right-1,rcCell.top);
	pDC->LineTo(rcCell.right-1,rcCell.bottom);

	pDC->SelectObject(pOldPen);
}

void CCustomListCtrl::DrawCellBackground(CDC* pDC, CRect rcCell, int nRow)
{
	COLORREF bgColor;
	if ( m_vecRowColors.size() != 0 )
	{
		int nColorIndex = nRow % m_vecRowColors.size();
		bgColor = m_vecRowColors[nColorIndex];
	}
	else
	{
		bgColor = m_defaultListFormat.colorCellBackground;
	}

	pDC->FillSolidRect(rcCell,bgColor);
}

void CCustomListCtrl::DrawAllBelowEmptyCells(CDC* pDC, CRect rcCell, int nRow, int nBottom)
{
	int nEmptyRow = (nBottom-rcCell.bottom) / rcCell.Height() + 1;
	CRect rcBottom = rcCell;
	for ( int i = 1; i <= nEmptyRow; i++)
	{
		rcBottom.top = rcBottom.bottom;
		rcBottom.bottom = rcBottom.top + rcCell.Height();
		DrawCellBackground(pDC,rcBottom, nRow+i);
		DrawCellBorder(pDC, rcBottom);
	}
}

void CCustomListCtrl::DrawCellBorder(CDC* pDC, int nRow, int nCol)
{
	CRect rcCell;
	
	if ( nRow == -1 )
	{
		rcCell.top = m_ctrlHeader.m_Height;
		rcCell.bottom = rcCell.top + m_nRowHeight;
		rcCell.left = 0 - GetScrollPos(SB_HORZ);

		int nColumnCount = m_ctrlHeader.GetItemCount();
		for ( int i = 0; i < nCol; i++)
		{
			rcCell.left += GetColumnWidth(i);
		}
		rcCell.right = rcCell.left + GetColumnWidth(nCol);
		DrawCellBackground(pDC,rcCell,0);
	}
	else
	{
		if (!GetSubItemRect(nRow, nCol ,LVIR_LABEL, rcCell)) 
			return;

		if ( nCol == 0 )
			rcCell.left -= 2;
	}

	

	CRect rcClient;
	GetClientRect(&rcClient);

	CRect rcRight = rcCell;
	rcRight.left = rcCell.right;
	rcRight.right = rcClient.right;

	BOOL bRightMost = (nCol == m_ctrlHeader.GetItemCount()-1) && ( rcCell.right < rcClient.right );
	BOOL bBottomMost = ( nRow == GetItemCount()-1 ) && ( rcCell.bottom < rcClient.bottom );

	DrawCellBorder(pDC, rcCell, bRightMost);

	if ( nRow == -1 )
	{
		nRow = 0;
	}

	if ( bRightMost )
	{
		DrawCellBackground(pDC,rcRight, nRow);
		DrawCellBorder(pDC, rcRight);
	}

	if ( bBottomMost )
	{
		DrawAllBelowEmptyCells(pDC,rcCell,nRow,rcClient.bottom);
	}

	if ( bRightMost && bBottomMost )
	{
		DrawAllBelowEmptyCells(pDC,rcRight,nRow,rcClient.bottom);
	}
}

void CCustomListCtrl::DrawCellPictureButton(CDC* pDC, CellFormat* pCellFormat, BOOL bPushed)
{
	UINT uFrameCtrl = DFCS_BUTTONPUSH;
	if ( bPushed )
	{
		uFrameCtrl |= DFCS_PUSHED;
	}

	CRect rcCtrl = pCellFormat->rcCellCtrl;
	pDC->DrawFrameControl (rcCtrl, DFC_BUTTON , uFrameCtrl);


	CRect rcBmp = pCellFormat->rcCellCtrl;
	DrawCellBMP(pDC, pCellFormat, rcBmp, FALSE);

	CRect rcButtonText = pCellFormat->rcCellCtrl;
	rcButtonText.left = rcBmp.right+ 4;
	rcButtonText.DeflateRect(2,2,2,2);

	if ( pCellFormat->bEnabled )
	{
		DrawCellText(pDC, pCellFormat->strCellCtrlText, rcButtonText, DT_LEFT, RGB(0,0,0), FALSE, FALSE, FALSE);
	}
	else
	{
		OffsetRect(&rcButtonText, 1, 1);
		DrawCellText(pDC, pCellFormat->strCellCtrlText, rcButtonText, DT_LEFT, GetSysColor (COLOR_3DHILIGHT), FALSE, FALSE, FALSE );

		OffsetRect(&rcButtonText, -1, -1);
		DrawCellText(pDC, pCellFormat->strCellCtrlText, rcButtonText, DT_LEFT, GetSysColor (COLOR_GRAYTEXT), FALSE, FALSE, FALSE );
	}
}

void CCustomListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	CDC* pDC;
	pDC = CDC::FromHandle(lpDrawItemStruct->hDC);

	int nRow = lpDrawItemStruct->itemID;

	// The column header is hide
	if ( nRow == 0 && m_ctrlHeader.m_Height == 0)
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		rcClient.bottom = rcClient.top + 2;

		DrawCellBackground(pDC, rcClient, -1);
		
	}

	BOOL bSelected = ConsistentRowSelection(nRow);
	COLORREF bgColor;

	for ( int nCol=0; nCol < GetHeaderCtrl()->GetItemCount(); nCol++)
	{
		CRect rcCell;
		if (!GetSubItemRect(nRow, nCol ,LVIR_LABEL, rcCell))   
			continue;   

		CellFormat* pCellFormat = GetCellFormat(nRow, nCol);
		ASSERT(pCellFormat);

		CRect rcTemp;
		rcTemp = rcCell;

		if (nCol==0)
			rcTemp.left -=2;
		
		if ( bSelected && m_bShowSelection && pCellFormat->bEnabled )
		{
			bgColor = GetSysColor(COLOR_HIGHLIGHT);
		}
		else
		{			
			// If No background color was set, use the default color
			if ( m_vecRowColors.size() != 0 )
			{
				int nColorIndex = nRow % m_vecRowColors.size();
				bgColor = m_vecRowColors[nColorIndex];
			}
			else
			{
				bgColor = pCellFormat->colorCellBackground;
			}

		}
		pDC->FillSolidRect(rcTemp,bgColor);

		CString strCellText = GetItemText(nRow, nCol);
		if ( pCellFormat->cellType == cellTextReadOnly ||
			 pCellFormat->cellType == cellTextEdit ||
			 pCellFormat->cellType == cellTextNotes ||
			 pCellFormat->cellType == cellTextCombo ||
			 pCellFormat->cellType == cellTextTime ||
			 pCellFormat->cellType == cellTextDate  )
		{
			rcTemp.right -= 2;
			DrawCellText(pDC, pCellFormat, strCellText, rcTemp, bSelected );
		}
		else if ( pCellFormat->cellType == cellTextDateTime )
		{
			rcTemp.right -= 2;
			int nPos = strCellText.Find(_T(" "));
			CString strDate = strCellText.Left(nPos);
			CString strTime = strCellText.Right(strCellText.GetLength()-nPos-1);

			CRect rcDate, rcTime;

			rcDate = rcTemp;
			rcDate.right = rcDate.left + rcDate.Width()/2;
			DrawCellText(pDC, pCellFormat, strDate, rcDate, bSelected );

			rcTime = rcTemp;
			rcTime.left = rcTime.right - rcTime.Width()/2;
			DrawCellText(pDC, pCellFormat, strTime, rcTime, bSelected );
		}
		else if ( pCellFormat->cellType == cellCtrlRadioButton || 
				  pCellFormat->cellType == cellCtrlCheckBox )
		{
			CRect rcCtrl = rcTemp;
			
			if ( pCellFormat->bShowCtrl )
			{
				rcCtrl.top += 3;
				rcCtrl.bottom -= 3;
				rcCtrl.left += 3;
				rcCtrl.right = rcCtrl.left + rcCtrl.Height();
				DrawCellCtrl(pDC, pCellFormat, nRow, nCol, rcCtrl, lpDrawItemStruct);
			}

			if ( pCellFormat->bShowText)
			{
				CRect rcText = rcTemp;
				rcText.left = rcCtrl.right + 2;
				DrawCellText(pDC, pCellFormat, strCellText, rcText, bSelected );
			}
		}
		else if ( pCellFormat->cellType == cellCtrlButton )
		{
			CRect rcCtrl = rcTemp;

			if ( pCellFormat->bShowCtrl )
			{
				rcCtrl.top += 2;
				rcCtrl.bottom -= 2;
				rcCtrl.right -= 2;
				rcCtrl.left = rcCtrl.left+2;
				pCellFormat->rcCellCtrl = rcCtrl;
				DrawCellButton(pDC, pCellFormat, FALSE);
			}

			if ( pCellFormat->bShowText )
			{
				CRect rcText = rcTemp;
				rcText.right = rcCtrl.left - 2;
				DrawCellText(pDC, pCellFormat, strCellText, rcText, bSelected );
			}

		}
		else if ( pCellFormat->cellType == cellCtrlPictureButton )
		{
			CRect rcCtrl = rcTemp;
			rcCtrl.DeflateRect(2,2);
			pCellFormat->rcCellCtrl = rcCtrl;

			DrawCellPictureButton(pDC,pCellFormat,FALSE);
		}
		else if ( pCellFormat->cellType == cellPictureBMP )
		{
			CRect rcCtrl = rcTemp;
			DrawCellBMP(pDC, pCellFormat, rcCtrl, TRUE);

			if ( pCellFormat->bShowText)
			{
				CRect rcText = rcTemp;
				rcText.left = pCellFormat->rcCellCtrl.right + 8;
				DrawCellText(pDC, pCellFormat, strCellText, rcText, bSelected );
			}
		}
		else if ( pCellFormat->cellType == cellPictureICON )
		{
			CRect rcCtrl = rcTemp;
			DrawCellICON(pDC, pCellFormat, rcCtrl, TRUE);

			if ( pCellFormat->bShowText)
			{
				CRect rcText = rcTemp;
				rcText.left = pCellFormat->rcCellCtrl.right + 8;
				DrawCellText(pDC, pCellFormat, strCellText, rcText, bSelected );
			}
		}
		else if ( pCellFormat->cellType == cellCtrlDirectory ||
				  pCellFormat->cellType == cellCtrlFile )
		{
			CRect rcIcon = rcTemp;
			if ( pCellFormat->cellType == cellCtrlDirectory )
			{
				pCellFormat->hPicture = m_hFolder;
			}

			DrawCellICON(pDC, pCellFormat, rcIcon, FALSE);

			CRect rcButton = rcTemp;
			rcButton.left = rcButton.right;
			if ( pCellFormat->bShowCtrl )
			{
				rcButton.top += 2;
				rcButton.bottom -= 2;
				rcButton.right -= 2;
				rcButton.left = rcButton.right-30;

				pCellFormat->rcCellCtrl = rcButton;
				pCellFormat->strCellCtrlText = _T("...");
				DrawCellButton(pDC, pCellFormat, FALSE);
			}

			CRect rcText = rcTemp;
			rcText.left = rcIcon.right;
			rcText.right = rcButton.left;
			DrawCellText(pDC, pCellFormat, strCellText, rcText, bSelected );
		}
		else if ( pCellFormat->cellType == cellCtrlColor )
		{
			CRect rcIcon = rcTemp;
			rcIcon.left += 4;
			rcIcon.top += 2;
			rcIcon.bottom -= 2;
			rcIcon.right = rcIcon.left + (int)(rcIcon.Height()*1.5);

			pDC->FillSolidRect(rcIcon, StringToColorRef(strCellText) );

			//edge of color frame
			CBrush br;
			br.CreateSolidBrush(RGB(0, 0, 0));
			pDC->FrameRect(&rcIcon, &br);

			CRect rcButton = rcTemp;
			rcButton.left = rcButton.right;
			if ( pCellFormat->bShowCtrl )
			{
				rcButton.top += 2;
				rcButton.bottom -= 2;
				rcButton.right -= 2;
				rcButton.left = rcButton.right-30;

				pCellFormat->rcCellCtrl = rcButton;
				pCellFormat->strCellCtrlText = _T("...");
				DrawCellButton(pDC, pCellFormat, FALSE);
			}

			CRect rcText = rcTemp;
			rcText.left = rcIcon.right;
			rcText.right = rcButton.left;
			DrawCellText(pDC, pCellFormat, strCellText, rcText, bSelected );
		}

		DrawCellBorder(pDC, nRow, nCol);

	}
}

void CCustomListCtrl::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	CListCtrl::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}

void CCustomListCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
	if (m_nRowHeight>0)
	{
		lpMeasureItemStruct->itemHeight = m_nRowHeight;
	}
}


void CCustomListCtrl::SetRowHeigt(int nHeight) //高置行高
{
	m_nRowHeight = nHeight;

	CRect rcWin;
	GetWindowRect(&rcWin);
	WINDOWPOS wp;
	wp.hwnd = m_hWnd;
	wp.cx = rcWin.Width();
	wp.cy = rcWin.Height();
	wp.flags = SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER;
	SendMessage(WM_WINDOWPOSCHANGED, 0, (LPARAM)&wp);


	m_nFontHeight = nHeight-9;
	//if ( m_FilterFont.m_hObject)
	//	m_FilterFont.DeleteObject();

	//LOGFONT logFont;

	//ZeroMemory(&logFont, sizeof(logFont));
	//strcpy_s(logFont.lfFaceName, "Arial");
	//logFont.lfHeight = nHeight-6;
	//logFont.lfCharSet = DEFAULT_CHARSET;
	//logFont.lfWeight = FW_NORMAL;

	//m_FilterFont.CreateFontIndirect(&logFont);
}

//void CCustomListCtrl::SetHeaderFontHW(int nHeight,int nWith) //设置头部字体宽和高
//{
//	m_Header.m_fontHeight = nHeight;
//	m_Header.m_fontWith = nWith;
//}
//void CCustomListCtrl::SetHeaderTextColor(COLORREF color) //设置头部字体颜色
//{
//	m_Header.m_color = color;
//}

//void CCustomListCtrl::SetFontHW(int nHeight,int nWith) //设置字体高和宽
//{
//	m_fontHeight = nHeight;
//	m_fontWith = nWith;
//}

void CCustomListCtrl::FreeItemMemory( const int nItem, BOOL bDeleteFromCache)
{
	RowItemData* pRowItemData = reinterpret_cast<RowItemData*>(GetItemData(nItem));
	if ( pRowItemData )
	{
		if ( bDeleteFromCache && m_p2DCache)
		{
			m_p2DCache->CacheDeleteRow(pRowItemData->nCacheIndex);
		}
		
		delete pRowItemData;
		pRowItemData = NULL;
	}
}


BOOL CCustomListCtrl::DeleteItem( int nItem, BOOL bDeleteFromCache )
{
	CRect rc;
	GetItemRect(nItem, rc, LVIR_BOUNDS);

	CRect rcClient;
	GetClientRect(rcClient);

	rc.bottom = rcClient.bottom;
	
	FreeItemMemory( nItem, bDeleteFromCache );
	BOOL bOk = CListCtrl::DeleteItem( nItem );

	InvalidateRect(rc);
	return bOk;
}

void CCustomListCtrl::DeleteAllSelectedItems(BOOL bDeleteFromCache, BOOL bNotifyUser)
{
	int i = 0;
	RowItemData* pRowItemData;
	while ( i < GetItemCount() )
	{
		pRowItemData = reinterpret_cast<RowItemData*>(GetItemData(i));
		if ( !pRowItemData->bSelected )
		{
			i++;
			continue;
		}

		if ( bNotifyUser && m_pUser )
		{
			if ( !m_pUser->OnRowDeleting(this, i) )
				return;
		}
		DeleteItem(i, bDeleteFromCache);
		if ( bNotifyUser && m_pUser )
		{
			m_pUser->OnRowDeleted(this, i);
		}
	}
}

BOOL CCustomListCtrl::DeleteAllItems(BOOL bDeleteFromCache)
{
	for( int nItem = 0; nItem < GetItemCount(); nItem ++ )
		FreeItemMemory( nItem, bDeleteFromCache );
	return CListCtrl::DeleteAllItems();
}

void CCustomListCtrl::SetColumnOrder( CString strColumnOrder )
{
	int nColCount = GetHeaderCtrl()->GetItemCount();
	int* pOrderArray = new int[nColCount];
	int iStart = 0;
	for( int i =  0; i < nColCount; i++)
	{
		int iSemiColon = strColumnOrder.Find( _T(';'), iStart );

		if( iSemiColon == -1 )
			iSemiColon = strColumnOrder.GetLength();

		int iOrder = atoi( strColumnOrder.Mid( iStart, iSemiColon - iStart ) );

		iStart = iSemiColon + 1;

		pOrderArray[i] = iOrder;
		m_ColumnStates[i].m_nOrgPosition = iOrder;

	}
	VERIFY( SetColumnOrderArray(nColCount, pOrderArray) );
	delete [] pOrderArray;
}

void CCustomListCtrl::SetColumnInfo(CString strColumnInfo, CString strColumnOrder)
{
	// you must call this after setting the column headings.
	SetHeadings(strColumnInfo);
	SetColumnOrder(strColumnOrder);
}

void CCustomListCtrl::SetColumnInfo( CustomColumnList& headerList, CString strColumnOrder )
{
	CustomColumnList::iterator it;
	CString strColumnInfo = "";
	for (it=headerList.begin(); it!=headerList.end(); it++)
	{
		CustomColumn* column =&(*it);
		CString strTemp = strColumnInfo;
		strColumnInfo.Format(_T("%s%s,%d,%d,%d;"), strTemp, column->strHeaderCaption, column->bVisible, column->nWidth, column->nFormat);
	}
	m_ctrlHeader.SetColumnsList(headerList);
	SetColumnInfo(strColumnInfo, strColumnOrder);
}

// the heading text is in the format column 1 text,column 1 width;column 2 text,column 3 width;etc.
BOOL CCustomListCtrl::SetHeadings( const CString& strHeadings )
{
	int nStart = 0;
	m_nNumColumns = 0;
	InsertColumn( m_nNumColumns++, _T(""), 0, 0 );

	for( ;; )
	{
		// Get heading
		int iComma = strHeadings.Find( _T(','), nStart );
		if( iComma == -1 )
			break;
		const CString strHeading = strHeadings.Mid( nStart, iComma - nStart );

		// Get Visible
		nStart = iComma + 1;
		iComma = strHeadings.Find( _T(','), nStart );
		if( iComma == -1 )
			break;
		const int nVisible = atoi( strHeadings.Mid( nStart, iComma - nStart ) );

		// Get Width
		nStart = iComma + 1;
		iComma = strHeadings.Find( _T(','), nStart );
		if( iComma == -1 )
			break;
		const int iOrgWidth = atoi( strHeadings.Mid( nStart, iComma - nStart ) );


		// Get Format
		nStart = iComma + 1;
		int iSemiColon = strHeadings.Find( _T(';'), nStart );
		if( iSemiColon == -1 )
			iSemiColon = strHeadings.GetLength();
		const int iFormat = atoi( strHeadings.Mid( nStart, iSemiColon - nStart ) );

		nStart = iSemiColon + 1;


		int iWidth = iOrgWidth;
		bool bVisible = true;
		if(nVisible==0)
		{
			iWidth = 0;
			bVisible = false;
		}

		CString		strHeadMark = strHeading;
		int nRetVal = InsertColumn( m_nNumColumns++, strHeadMark, iFormat, iWidth );
		if(  nRetVal == -1 )
			return FALSE;
		///////////////

		InsertColumnState(m_nNumColumns-2, bVisible, iOrgWidth);

		HDITEM hdr;
		memset(&hdr,0,sizeof(HDITEM));

		hdr.mask=HDI_FORMAT;
		m_ctrlHeader.GetItem(nRetVal,&hdr);
		hdr.fmt|= HDF_OWNERDRAW;

		m_ctrlHeader.SetItem(nRetVal,&hdr);
		///////////////////

	}
	m_ctrlHeader.SetDisableStatus();

	DeleteColumn(0);
	m_nNumColumns--;

	return TRUE;
}

void CCustomListCtrl::InsertColumnState(int nCol, bool bVisible, int nOrgWidth)
{
	VERIFY( nCol >=0 && nCol <= m_ColumnStates.GetSize() );

	ListCtrlColumnState columnState;
	columnState.m_nOrgWidth = nOrgWidth;
	columnState.m_bVisible = bVisible;

	if (nCol == m_ColumnStates.GetSize())
	{
		// Append column picker to the end of the array
		m_ColumnStates.Add(columnState);
	}
	else
	{
		// Insert column in the middle of the array
		CSimpleArray<ListCtrlColumnState> newArray;
		for(int i=0 ; i < m_ColumnStates.GetSize(); ++i)
		{
			if (i == nCol)
				newArray.Add(columnState);
			newArray.Add(m_ColumnStates[i]);
		}
		m_ColumnStates = newArray;
	}
}

BOOL CCustomListCtrl::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{

	HD_NOTIFY       *pHDN = (HD_NOTIFY*)lParam;

	// This code is for using bitmap in the background
	// Invalidate the right side of the control when a column is resized
	//if(pHDN->hdr.code == HDN_ITEMCHANGINGW || pHDN->hdr.code == HDN_ITEMCHANGEDW)
	//{
	//	int i=0;
	//	m_ctrlHeader.SetFilterRect();
	//	Invalidate(FALSE); 
	//} 


	BOOL bDefault=TRUE;

	if (GetHeaderCtrl())
	{
		if ((DWORD)wParam==(DWORD)GetHeaderCtrl()->m_hWnd)
		{
			NMHDR* pNM=(NMHDR*)lParam;

			if (//pNM->code==FLCN_FILTERCHANGING ||
				//pNM->code==FLCN_ENDFILTEREDIT ||
				//pNM->code==FLCN_BEGINFILTEREDIT ||
				pNM->code==MSG_CUSTOM_LIST_FILTER_TEXT_CHANGED )
			{
				if ( m_p2DCache )
				{
					FilterDataFromCache();
				}
				
				NMCUSTOMHDR* pNMF=(NMCUSTOMHDR*)lParam;
				if ( m_pUser )
				{
					m_pUser->OnFilterTextChanged(this, pNMF->idFrom, pNMF->szNewText);
				}
				bDefault=FALSE;

				//NMCUSTOMHDR nmh;

				//nmh.code=pNMF->code;
				//nmh.hwndFrom=m_hWnd;
				//nmh.idFrom=pNMF->idFrom;
				//nmh.szNewText=pNMF->szNewText;

				//if (GetParent())
				//{
				//	*pResult=GetParent()->SendMessage(WM_NOTIFY,(WPARAM)m_hWnd,(LPARAM)&nmh);
				//	bDefault=FALSE;
				//}
			}

			if (pNM->code==FLCN_SHOWINGEDIT)
			{
				CRect rc,rcWin;
				GetHeaderCtrl()->GetItemRect(pNM->idFrom,&rc);
				GetClientRect(rcWin);

				// scroll the window

				int nScrollBarGap=GetSystemMetrics(SM_CXVSCROLL);

				int nHScroll=GetScrollPos(SB_HORZ);

				if (rc.left<=nHScroll)
					Scroll(CSize(rc.left-nHScroll,0));
				else
				{
					if (rc.right>nHScroll+rcWin.Width()-nScrollBarGap)
					{
						if (rc.Width()>rcWin.Width()-nScrollBarGap)
							Scroll(CSize(rc.left-nHScroll,0));
						else
							Scroll(CSize(rc.right-(nHScroll+rcWin.Width())+nScrollBarGap*0,0));
					}
				}

				*pResult=0; // 0 continue editing, 1 stop editing

				bDefault=FALSE;
			}
			if ( pNM->code == FLCN_FILTERCHANGING )
			{
				int i = 0;
			}
		}
	}

	return (bDefault ? CListCtrl::OnNotify(wParam, lParam, pResult) : TRUE);
}

void CCustomListCtrl::ShowDateTime(int nRow, int nCol, BOOL bShowTimePart)
{
	if ( m_pDateTime == NULL )
	{
		CRect rc;
		GetSubItemRect(nRow,nCol,LVIR_LABEL,rc);

		UINT uStyle = WS_VISIBLE | WS_CHILD;
		CString strText = GetItemText(nRow,nCol);

		CellFormat* pCellFormat = GetCellFormat(nRow,nCol,FALSE);

		if ( pCellFormat->cellType == cellTextDate )
		{
			uStyle |= DTS_SHORTDATEFORMAT;
		}
		else if ( pCellFormat->cellType == cellTextTime )
		{
			uStyle |= DTS_TIMEFORMAT;
		}
		else if ( pCellFormat->cellType == cellTextDateTime )
		{
			if ( bShowTimePart )
			{
				uStyle |= DTS_TIMEFORMAT;
				rc.left = rc.right - rc.Width()/2;
			}
			else
			{
				uStyle |= DTS_SHORTDATEFORMAT;
				rc.right = rc.left + rc.Width()/2;
			}

			m_bShowTimeCtrlForDateTimeCell = !bShowTimePart;
		}
		else
		{
			ASSERT(FALSE);
		}


		m_pDateTime = new CFilterDateTime();
		m_pDateTime->Create(uStyle, rc, this, 1000);
		m_pDateTime->m_cellType = pCellFormat->cellType;
		m_pDateTime->m_bEditTime = bShowTimePart;

		COleDateTime tm;
		m_ctrlHeader.GetDateTime(strText, tm);
		m_pDateTime->SetTime(tm);

		SetFocus();
		m_pDateTime->SetFocus();

		m_nEditRow = nRow;
		m_nEditCol = nCol;
	}
}

void CCustomListCtrl::HideDateTime(BOOL bValidate)
{
	if (m_pDateTime == NULL )
		return;

	COleDateTime t;
	m_pDateTime->GetTime(t);

	CString strNewValue, strDate, strTime;

	strDate.Format("%04d-%02d-%02d", t.GetYear(), t.GetMonth(), t.GetDay());
	strTime.Format("%02d:%02d:%02d", t.GetHour(), t.GetMinute(), t.GetSecond());

	if ( m_pDateTime->m_cellType == cellTextDate )
	{
		strNewValue = strDate;	
	}
	else if ( m_pDateTime->m_cellType == cellTextTime )
	{
		strNewValue = strTime;	
	}
	else if ( m_pDateTime->m_cellType == cellTextDateTime )
	{
		CString strOldValue = GetItemText(m_nEditRow, m_nEditCol);
		int nPos = strOldValue.Find(_T(" "));
		if ( m_pDateTime->m_bEditTime )
		{
			strNewValue = strOldValue.Left(nPos) + _T(" ") + strTime; 
		}
		else
		{
			strNewValue = strDate + _T(" ") + strOldValue.Right(strOldValue.GetLength()-nPos-1);
		}
	}
	else
	{
		ASSERT(FALSE);
	}
	

	m_pDateTime->ShowWindow(SW_HIDE);
	m_pDateTime->DestroyWindow();
	m_pDateTime=NULL;

	OnCellCtrlHiding(bValidate,strNewValue);
}

void CCustomListCtrl::ShowEdit(int nRow, int nCol, UINT nMaxLen, CString& rStrValidChars)
{
	if (m_pEdit==NULL)
	{
		CRect rc;
		GetEditRect(nRow, nCol, rc);

		m_pEdit=new CFilterEdit();
		m_pEdit->CreateEx(WS_EX_CLIENTEDGE | WS_EX_NOPARENTNOTIFY ,"EDIT",NULL,ES_WANTRETURN | WS_VISIBLE | WS_CHILD | WS_GROUP | ES_AUTOHSCROLL,rc,this,IDC_ED_EDIT,NULL);

		if ( nMaxLen > 0 )
			m_pEdit->SetLimitText(nMaxLen);
		m_pEdit->SetFont(GetFont());
		m_pEdit->SetValidChars(rStrValidChars);
		m_pEdit->SetWindowText(GetItemText(nRow,nCol));
		m_pEdit->SetSel(0,m_pEdit->GetWindowTextLength());
		m_pEdit->SetFocus();

		m_nEditRow = nRow;
		m_nEditCol = nCol;
	}
}

void CCustomListCtrl::HitTest(LV_HITTESTINFO&  ht)
{
	CPoint pt(GetMessagePos());
	ScreenToClient(&pt);
	ht.pt      = pt;

	ListView_SubItemHitTest(m_hWnd, &ht);
}

BOOL CCustomListCtrl::OnDoubleClick( NMHDR* pNMHDR, LRESULT* pResult )
{
	return TRUE;
}

void CCustomListCtrl::OnCellCtrlHiding(BOOL bValidate, CString& strNewValue)
{
	m_bChangeCellTextRejected = FALSE;
	CString strOldValue = GetItemText(m_nEditRow, m_nEditCol);
	if ( strOldValue == strNewValue)
		return;

	if (!bValidate)
	{
		if ( m_pUser )
		{
			m_pUser->OnCellChangeCancelled(this, m_nEditRow,m_nEditCol, GetCellFormat(m_nEditRow,m_nEditCol),
				strOldValue, strNewValue);
		}
		return;
	}

	BOOL bChangeCellTextAllowed = TRUE;
	if (m_pUser )
	{
		bChangeCellTextAllowed = m_pUser->OnCellTextChanging(this, m_nEditRow,m_nEditCol, GetCellFormat(m_nEditRow,m_nEditCol),
			strOldValue, strNewValue);

		m_bChangeCellTextRejected = !bChangeCellTextAllowed;
	}

	if ( bChangeCellTextAllowed )
	{
		CellFormat* pCellFormat = GetCellFormat(m_nEditRow,m_nEditCol, TRUE);
		if ( pCellFormat->cellType == cellCtrlFile && strNewValue.Find(_T("\\")) != -1 )
		{
			// This is a file with full path, need to update the icon
			pCellFormat->SetPicture(strNewValue);
		}

		SetCellText(m_nEditRow, m_nEditCol, strNewValue, TRUE, TRUE);
	}

	Invalidate();
	UpdateWindow();
}

void CCustomListCtrl::OnCellCtrlHidden(BOOL bReturnIsHit)
{
	// This function handles when the edit ctrl or the combo box ctrl is hidden

	//if (GetParent())
	//	GetParent()->SetFocus();

	if (m_pUser && !m_bChangeCellTextRejected)
	{
		m_pUser->OnCellTextChanged(this,m_nEditRow,m_nEditCol);
	}

	// Return is hit
	if (  bReturnIsHit && m_bEditNextCellAfterReturnIsHit )
	{
		if ( m_bChangeCellTextRejected )
		{
			// We do NOT changed this cell, so continue to edit this cell
			EditNextEditableCell(m_nEditRow,m_nEditCol-1);
		}
		else
		{
			// We have changed this cell, so start to edit next cell
			EditNextEditableCell(m_nEditRow,m_nEditCol);
		}
	}
}

// This function will be called when the edit ctrl lose focus( Mouse click on other ctrl, user hit return and escape)
LRESULT CCustomListCtrl::OnEditTextChanged(WPARAM wParam, LPARAM lParam)
{
	TRACE("OnEditTextChanged\r\n");
	HideEdit(wParam);

	OnCellCtrlHidden(lParam == 1);

	return 0;
}

LRESULT CCustomListCtrl::OnDateTimeChanged(WPARAM wParam, LPARAM lParam)
{
	TRACE("OnDateTimeChanged\r\n");
	HideDateTime(wParam);

	OnCellCtrlHidden(lParam == 1);

	return 0;
}

void CCustomListCtrl::HideEdit(BOOL bValidate)
{
	if (m_pEdit == NULL )
		return;

	CString strNewValue;
	m_pEdit->GetWindowText(strNewValue);
	m_pEdit->DestroyWindow();
	m_pEdit=NULL;

	OnCellCtrlHiding(bValidate,strNewValue);
}

void CCustomListCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	CListCtrl::OnLButtonUp(nFlags,point);
}

void CCustomListCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	int nRow, nCol;
	CellFormat* pCellFormat = NULL;

	if ( !CheckClick(nRow, nCol, &pCellFormat ))
	{
		// Clicked a disabled cell
		if ( pCellFormat != NULL && !pCellFormat->bEnabled)
		{
			return;
		}

		if ( m_bDoubleClickEmptyAreaToAddRow )
		{
			AppendDefaultRow();
		}
	}

	CListCtrl::OnLButtonDblClk(nFlags,point);

	if ( m_pUser )
	{
		m_pUser->OnRowLDblClicked(this, nRow,nCol,nFlags,point);
	}
}

void CCustomListCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CListCtrl::OnLButtonDown(nFlags,point);

	int nRow, nCol;
	CellFormat* pCellFormat = NULL;
	int nCount = GetItemCount();

	if ( CheckClick(nRow, nCol, &pCellFormat) )
	{
		RowItemData* pRowItemData = reinterpret_cast<RowItemData*>(GetItemData(nRow));
		ASSERT(pRowItemData != NULL);

		if ( m_bAllowMultlSelection && !pRowItemData->bSelected)
		{
			pRowItemData->bSelected = TRUE;
			CRect rc;
			GetItemRect(nRow, rc, LVIR_BOUNDS);
			InvalidateRect(rc);
		}
	}

	if ( m_bEnablePopupMenu )
	{
		EnableContextMenuItem(nRow,nCol);
		ClientToScreen ( &point );
		int nResult = m_menuContext.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, point.x, point.y, this);
		return OnContextMenuAction(nResult, nRow, nCol);
	}

	if ( m_pUser )
	{
		m_pUser->OnRowRClicked(this,nRow,nCol,nFlags,point);
	}
	
	//CListCtrl::OnRButtonDown(nFlags,point);
}

void CCustomListCtrl::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	CListCtrl::OnRButtonDblClk(nFlags,point);
}


BOOL CCustomListCtrl::CheckClick(int& nRow, int& nCol, CellFormat** pCellFormat)
{
	if ( m_bIgnoreMouse )
		return FALSE;

	SetFocus();

	LV_HITTESTINFO  ht;
	HitTest(ht);

	nRow = ht.iItem;
	nCol = ht.iSubItem;

	if ( nRow < 0 || nCol < 0 )
		return FALSE;

	*pCellFormat = GetCellFormat(nRow,nCol);
	return (*pCellFormat)->bEnabled;
}

void CCustomListCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if ( m_bIgnoreMouse )
		return;

	int nRow, nCol;
	CellFormat* pCellFormat = NULL;

	if ( !CheckClick(nRow, nCol, &pCellFormat ))
		return;

	RowItemData* pRowItemData = reinterpret_cast<RowItemData*>(GetItemData(nRow));
	ASSERT(pRowItemData != NULL);

	if ( m_bAllowMultlSelection )
	{
		pRowItemData->bSelected = !pRowItemData->bSelected;
		CRect rc;
		GetItemRect(nRow, rc, LVIR_BOUNDS);
		InvalidateRect(rc);
	}

	if ( pCellFormat->cellType==cellTextEdit || pCellFormat->cellType==cellTextCombo || pCellFormat->cellType==cellTextDate || 
		 pCellFormat->cellType==cellTextTime || pCellFormat->cellType==cellTextDateTime || pCellFormat->cellType==cellTextNotes ||
		 ((pCellFormat->cellType == cellCtrlDirectory || pCellFormat->cellType == cellCtrlFile ) && pCellFormat->rcCellText.PtInRect(point) ) )
	{
		BOOL bSelected = GetItemState(nRow, LVIS_SELECTED ) & LVIS_SELECTED;
		if ( !m_bSelectThenEdit || ( m_bSelectThenEdit&&bSelected ) )
		{
			CRect rc;
			GetSubItemRect(nRow,nCol,LVIR_LABEL,rc);

			if ( pCellFormat->cellType == cellTextDateTime && point.x > (rc.left+rc.Width()/2) )
			{
				ShowDateTime(nRow, nCol, TRUE);
			}
			else
			{
				StartCellEdit(nRow, nCol);
			}
			
			return;
		}

	}

	CListCtrl::OnLButtonDown(nFlags,point);

	CRect rcCellCtrl = pCellFormat->rcCellCtrl;
	m_bCellButtonClicked = rcCellCtrl.PtInRect(point);

	if ( pCellFormat->cellType == cellCtrlCheckBox || pCellFormat->cellType == cellCtrlRadioButton )
	{
		pCellFormat->bChecked = !pCellFormat->bChecked;
		InvalidateRect(rcCellCtrl);
	}
	else if (  m_bCellButtonClicked && (pCellFormat->cellType == cellCtrlDirectory || pCellFormat->cellType == cellCtrlFile) )
	{
		HDC hDC = ::GetDC(m_hWnd);
		CDC* pDC = CDC::FromHandle(hDC);
		DrawCellButton(pDC, pCellFormat, TRUE );
		Sleep(50);
		DrawCellButton(pDC, pCellFormat, FALSE );
		::ReleaseDC(m_hWnd, hDC);

		if ( pCellFormat->cellType == cellCtrlFile )
		{
			CFileDialog dlg (TRUE, NULL, NULL, OFN_HIDEREADONLY,
				"All Files (*.*)|*.*||" );
			if ( dlg.DoModal() != IDOK )
				return;

			CString strCellText = dlg.GetPathName();
			pCellFormat->SetPicture(strCellText);
			SetCellText(nRow,nCol,strCellText, FALSE, TRUE );
		}
		else
		{
			CString strText = GetItemText(nRow,nCol);
			CString strNewDir = BrowserForFolder(NULL, _M("Choose Directory"), strText);
			if ( !strNewDir.IsEmpty() )
			{
				SetCellText(nRow,nCol, strNewDir, FALSE, TRUE);
			}
		}
	}
	else if ( m_bCellButtonClicked && pCellFormat->cellType == cellCtrlColor )
	{
		HDC hDC = ::GetDC(m_hWnd);
		CDC* pDC = CDC::FromHandle(hDC);
		DrawCellButton(pDC, pCellFormat, TRUE );
		Sleep(50);
		DrawCellButton(pDC, pCellFormat, FALSE );
		::ReleaseDC(m_hWnd, hDC);

		CColorDialog colorDlg ( StringToColorRef(GetItemText(nRow, nCol)) );

		if (IDOK == colorDlg.DoModal())
		{
			COLORREF crColor = colorDlg.GetColor();
			SetCellText(nRow,nCol, ColorRefToString(crColor), FALSE, TRUE);
		}
	}

	if ( m_pUser )
	{
		if ( m_bCellButtonClicked && pCellFormat->cellType == cellCtrlButton )
		{
			HDC hDC = ::GetDC(m_hWnd);
			CDC* pDC = CDC::FromHandle(hDC);
			DrawCellButton(pDC, pCellFormat, TRUE );
			Sleep(50);
			DrawCellButton(pDC, pCellFormat, FALSE );
			m_pUser->OnCellCtrlClicked(this,nRow,nCol,pCellFormat);

			::ReleaseDC(m_hWnd, hDC);
		}
		else if ( m_bCellButtonClicked && pCellFormat->cellType == cellCtrlPictureButton )
		{
			HDC hDC = ::GetDC(m_hWnd);
			CDC* pDC = CDC::FromHandle(hDC);
			DrawCellPictureButton(pDC, pCellFormat, TRUE );
			Sleep(50);
			DrawCellPictureButton(pDC, pCellFormat, FALSE );
			m_pUser->OnCellCtrlClicked(this,nRow,nCol,pCellFormat);

			::ReleaseDC(m_hWnd, hDC);
		}
		else if ( pCellFormat->cellType == cellCtrlCheckBox || pCellFormat->cellType == cellCtrlRadioButton )
		{
			m_pUser->OnCellCtrlClicked(this,nRow,nCol,pCellFormat);
		}
		else
		{
			m_pUser->OnRowLClicked(this, nRow,nCol,nFlags,point);
		}
	}	
}

void CCustomListCtrl::OnMButtonDown(UINT nFlags, CPoint point) 
{
	CListCtrl::OnMButtonDown(nFlags,point);

	int nRow, nCol;
	CellFormat* pCellFormat = NULL;

	if ( !CheckClick(nRow, nCol, &pCellFormat ))
		return;

	if ( m_bSelectThenEdit )
		return;

	StartCellEdit(nRow, nCol);
}

BOOL CCustomListCtrl::EditNextEditableCell(int nRow, int nCol)
{
	CellFormat* pCellFormat = GetCellFormat(nRow,nCol);
	if ( pCellFormat == NULL )
		return FALSE;

	if ( pCellFormat->cellType == cellTextDateTime )
	{
		if ( m_bShowTimeCtrlForDateTimeCell )
		{
			ShowDateTime(nRow,nCol,TRUE);
//			m_bShowTimeCtrlForDateTimeCell = FALSE;
			return TRUE;
		}
		else
		{
			m_bShowTimeCtrlForDateTimeCell = TRUE;
		}
	}

	for ( int i = nCol+1; i < m_ctrlHeader.GetItemCount(); i ++ )
	{
		if ( StartCellEdit(nRow,i) )
			return TRUE;
	}

	return FALSE;
}

BOOL CCustomListCtrl::StartCellEdit(int nRow, int nCol)
{
	if ( m_bReadOnly )
		return FALSE;

	CellFormat* pCellFormat = GetCellFormat(nRow,nCol);
	if ( pCellFormat == NULL )
		return FALSE;

	if ( pCellFormat->cellType == cellTextEdit ||
		 pCellFormat->cellType == cellCtrlDirectory ||
		 pCellFormat->cellType == cellCtrlFile )
	{
		int nMaxLen = 0;
		CString strValidChars = _T("");
		if ( m_pUser )
		{
			m_pUser->OnCellEditorDisplayed(this, nRow,nCol,pCellFormat,nMaxLen, strValidChars);
		}
		ShowEdit(nRow, nCol, nMaxLen, strValidChars);
		return TRUE;
	}
	else if ( pCellFormat->cellType == cellTextCombo )
	{
		CStringList slComboItem;
		
		CustomColumn column = m_ctrlHeader.m_gridColumnsList[nCol];
		int nCount = column.dropdownValueList.size();
		for ( int i = 0; i < nCount; i ++ )
		{
			slComboItem.AddTail(column.dropdownValueList[i].strText);
		}
		
		if ( m_pUser )
		{
			m_pUser->OnCellComboDisplayed(this,nRow,nCol,pCellFormat,slComboItem);
		}

		ShowCombo(nRow,nCol,slComboItem);
		return TRUE;
	}
	else if ( pCellFormat->cellType == cellTextDate || pCellFormat->cellType == cellTextTime || pCellFormat->cellType == cellTextDateTime)
	{
		ShowDateTime(nRow,nCol, FALSE);
		return TRUE;
	}
	else if ( pCellFormat->cellType == cellTextNotes )
	{
		ShowNotes(nRow,nCol);
		return TRUE;
	}

	return FALSE;
}

void CCustomListCtrl::HideAllCellCtrl(BOOL bValidate)
{
	if( GetFocus() != this )
		SetFocus();	// Force focus to finish editing

	HideDateTime(bValidate);
}

void CCustomListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	HideAllCellCtrl(FALSE);

	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CCustomListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	HideAllCellCtrl(FALSE);

	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL CCustomListCtrl::OnEraseBkgnd( CDC* pDC )
{
	//if(!(GetStyle()   &   WS_VSCROLL)) 
	//{ 
	//	ModifyStyle(0,   WS_VSCROLL,   SWP_DRAWFRAME); 
	//	EnableScrollBar(  SB_VERT,   ESB_ENABLE_BOTH); 
	//} 
	//if(!(GetStyle()   &   WS_HSCROLL)) 
	//{ 
	//	ModifyStyle(0,   WS_HSCROLL,   SWP_DRAWFRAME); 
	//	EnableScrollBar( SB_HORZ,   ESB_ENABLE_BOTH); 
	//} 

	int nItemCount = GetItemCount();
	if ( nItemCount == 0)
		return TRUE;

	int nFirstShowRow = GetScrollPos(SB_VERT);
	int nLastShowRow = nFirstShowRow + GetCountPerPage() + 2;
	if ( nLastShowRow >= nItemCount)
	{
		nLastShowRow = nItemCount-1;
	}


	int nColumnCount = m_ctrlHeader.GetItemCount();
	CRect rcClient, rcRightCell;
	GetClientRect(rcClient);
	GetSubItemRect(0, nColumnCount-1, LVIR_BOUNDS, rcRightCell);
	if ( rcClient.right > rcRightCell.right)
	{
		for ( int i = nFirstShowRow; i <= nLastShowRow; i++ )
		{
			DrawCellBorder(pDC, i,nColumnCount-1);
		}
	}

	for ( int i = 0; i < nColumnCount; i++ )
	{
		DrawCellBorder(pDC, nLastShowRow,i);
	}

	return TRUE;//CListCtrl::OnEraseBkgnd(pDC);
}

BOOL CCustomListCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// Not sure why the CBN_CLOSEUP message is NOT sent to the ctrl itself
	if  ( m_pCombo!= NULL && (HWND)lParam == m_pCombo->m_hWnd )
	{
		if (HIWORD(wParam) == CBN_CLOSEUP )
		{
			HideCombo(TRUE);
			return TRUE;
		}
	}

	return TRUE;
}

void CCustomListCtrl::ShowCombo(int nRow, int nCol,CStringList& rComboItemsList)
{
	if(NULL == m_pCombo)
	{
		CRect rc, rcClient;
		GetSubItemRect(nRow,nCol,LVIR_LABEL,rc);
		GetWindowRect(rcClient);

		rc.bottom = rcClient.bottom;

		DWORD dwStyle = WS_TABSTOP | WS_CHILD | WS_VISIBLE | CBS_AUTOHSCROLL | WS_VSCROLL | CBS_DROPDOWNLIST;
		m_pCombo = new CFilterCombo();
		m_pCombo->Create(dwStyle, rc, this, 0);
		m_pCombo->SetFont(GetFont());

		int iCurSel = -1;
		int itmp = 0;
		CString strCellText = GetItemText(nRow,nCol);

		for (POSITION Pos_ = rComboItemsList.GetHeadPosition(); Pos_ != NULL;)
		{
			CString strList = (LPCTSTR) (rComboItemsList.GetNext(Pos_));
			m_pCombo->AddString(strList);
			if(strCellText.Compare(strList)==0)
			{
				iCurSel = itmp;	
			}
			itmp++;
		}

		if ( iCurSel == -1 )
		{
			m_pCombo->AddString(strCellText);
			iCurSel = itmp;
		}

		m_pCombo->SetItemHeight(-1, m_nRowHeight-6);
		m_pCombo->SetCurSel(iCurSel);
		m_pCombo->SetFocus();

		m_nEditRow = nRow;
		m_nEditCol = nCol;
	}
}

void CCustomListCtrl::HideCombo(BOOL bValidate)
{
	if (m_pCombo == NULL)
		return;

	CString strNewValue;
	m_pCombo->GetWindowText(strNewValue);
	m_pCombo->m_bNotifySent = TRUE;
	m_pCombo->ShowWindow(SW_HIDE);
//	m_pCombo->DestroyWindow();
	//delete m_pCombo;
	m_pCombo=NULL;

	OnCellCtrlHiding(bValidate,strNewValue);
}

// This function will be called only when user hit return or escape, 
// user select the combo item will NOT trigger this function
LRESULT CCustomListCtrl::OnComboSelChanged(WPARAM wParam, LPARAM lParam)
{
	TRACE("OnComboSelChanged\r\n");
	HideCombo(wParam);

	OnCellCtrlHidden(lParam == 1);

	return 0;
}

void CCustomListCtrl::OnColumnClick( NMHDR* pNMHDR, LRESULT* pResult )
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	const int iColumn = pNMListView->iSubItem;

	// if it's a second click on the same column then reverse the sort order,
	// otherwise sort the new column in ascending order.

	if ( m_ctrlHeader.m_bEnableSort)
	{
		Sort( iColumn, iColumn == m_nSortColumn ? !m_bSortAscending : TRUE );
	}
	
	*pResult = 0;
}


void CCustomListCtrl::Sort( int nColumn, BOOL bAscending )
{
	m_nSortColumn = nColumn;
	m_bSortAscending = bAscending;

	// show the appropriate arrow in the header control.
	m_ctrlHeader.SetSortArrow( m_nSortColumn, m_bSortAscending );

	VERIFY( SortItems( CompareFunction, reinterpret_cast<DWORD>( this ) ) );
}

void CCustomListCtrl::SortPrevious()
{
	if(m_nSortColumn>-1)
	{
		// show the appropriate arrow in the header control.
		m_ctrlHeader.SetSortArrow( m_nSortColumn, m_bSortAscending );

		VERIFY( SortItems( CompareFunction, reinterpret_cast<DWORD>( this ) ) );
	}
}


int CALLBACK CCustomListCtrl::CompareFunction( LPARAM lParam1, LPARAM lParam2, LPARAM lParamData )
{
	CCustomListCtrl* pListCtrl = reinterpret_cast<CCustomListCtrl*>( lParamData );
	ASSERT( pListCtrl->IsKindOf( RUNTIME_CLASS( CListCtrl ) ) );

	RowItemData* pid1 = reinterpret_cast<RowItemData*>( lParam1 );
	RowItemData* pid2 = reinterpret_cast<RowItemData*>( lParam2 );


	ASSERT( pid1 );
	ASSERT( pid2 );

	int nCompareResult = 0;
	if ( pListCtrl->m_pUser )
	{
		if ( pListCtrl->m_pUser->OnSort(pListCtrl, pid1, pid2, nCompareResult) )
		{
			return nCompareResult;
		}
	}

	CString strText1 = pid1->GetCellText(pListCtrl->m_nSortColumn);
	CString strText2 = pid2->GetCellText(pListCtrl->m_nSortColumn);

	ASSERT(pListCtrl->m_nSortColumn!=-1);
	CustomColumn column = pListCtrl->m_ctrlHeader.m_gridColumnsList[pListCtrl->m_nSortColumn];


	if ( column.compareType == compareAsNumber )
	{
		return pListCtrl->m_bSortAscending ? pListCtrl->NumberCompare( strText1, strText2 ) : pListCtrl->NumberCompare( strText2, strText1 );
	}
	else if( column.compareType == compareAsDate || column.compareType == compareAsTime || column.compareType == compareAsDateTime)
	{
		return pListCtrl->m_bSortAscending ? pListCtrl->DateCompare( strText1, strText2, column.compareType) : pListCtrl->DateCompare( strText2, strText1, column.compareType);
	}
	else
	{
		return pListCtrl->m_bSortAscending ? lstrcmp( strText1, strText2 ) : lstrcmp( strText2, strText1 );
	}
}

ColumnCompareType CCustomListCtrl::IsDateTime( LPCTSTR pszText )
{
	ASSERT_VALID_STRING( pszText );

	COleDateTime oleDate;

	if(oleDate.ParseDateTime(pszText, VAR_DATEVALUEONLY))
	{
		if(oleDate.m_dt>0)
		{
			if(oleDate.ParseDateTime(pszText, VAR_TIMEVALUEONLY))
			{
				if(oleDate.m_dt>0)//if 0:0:0 go out
					return compareAsDateTime;
			}
			return compareAsDate;
		}
		else
		{
			return compareAsTime;
		}
	}
	return compareAsText;
}


int CCustomListCtrl::DateCompare( const CString& strDate1, const CString& strDate2, ColumnCompareType iType)
{
	DWORD dwDateTime;
	COleDateTime oleDate1, oleDate2;
	if(iType==compareAsDate)
	{
		dwDateTime = VAR_DATEVALUEONLY;
	}
	else if(iType==compareAsTime)
	{
		dwDateTime = VAR_TIMEVALUEONLY;
	}
	else
	{
		dwDateTime = 0;
	}
	BOOL bRet1 = oleDate1.ParseDateTime(strDate1, dwDateTime);
	BOOL bRet2 = oleDate2.ParseDateTime(strDate2, dwDateTime);
	if(bRet1 || bRet2)
	{
		if(oleDate1.GetStatus()!=COleDateTime::valid||strDate1.IsEmpty())
			return -1;
		else if(oleDate2.GetStatus()!=COleDateTime::valid||strDate2.IsEmpty())
			return 1;
		if(oleDate1<oleDate2)
		{
			return -1;
		}
		else if(oleDate1>oleDate2)
		{
			return 1;
		}
	}

	if(iType==compareAsDateTime)
	{
		dwDateTime = VAR_TIMEVALUEONLY;
		bRet1 = oleDate1.ParseDateTime(strDate1, dwDateTime);
		bRet2 = oleDate2.ParseDateTime(strDate2, dwDateTime);
		if(bRet1 || bRet2)
		{
			if(oleDate1.GetStatus()!=COleDateTime::valid||strDate1.IsEmpty())
				return -1;
			else if(oleDate2.GetStatus()!=COleDateTime::valid||strDate2.IsEmpty())
				return 1;
			if(oleDate1<oleDate2)
			{
				return -1;
			}
			else if(oleDate1>oleDate2)
			{
				return 1;
			}
		}
	}

	return 0;
}

int CCustomListCtrl::NumberCompare( LPCTSTR pszNumber1, LPCTSTR pszNumber2 )
{
	ASSERT_VALID_STRING( pszNumber1 );
	ASSERT_VALID_STRING( pszNumber2 );

	TCHAR buf1[1024],buf2[1024];
	_tcscpy_s(buf1, pszNumber1);
	_tcscpy_s(buf2, pszNumber2);
	if (buf1[_tcslen(buf1)-1]==_T('%'))
		buf1[_tcslen(buf1)-1]='\0';
	if (buf2[_tcslen(buf2)-1]==_T('%'))
		buf2[_tcslen(buf2)-1]='\0';

	const double fNumber1 = atof( pszNumber1 );
	const double fNumber2 = atof( pszNumber2 );

	if( fNumber1 < fNumber2 )
		return -1;

	if( fNumber1 > fNumber2 )
		return 1;

	return 0;
}


void CCustomListCtrl::SetMultipleSelection(BOOL bMultiple)
{
	m_bAllowMultlSelection = bMultiple;
	if ( bMultiple )
	{
		ModifyStyle(LVS_SINGLESEL, 0 );
	}
	//else
	//{	
	//	ModifyStyle(0, LVS_SINGLESEL );
	//}
}

void CCustomListCtrl::OnPaint()
{
	if (GetItemCount()==0)// && !m_EmptyMarkupText.IsEmpty())
	{
////		m_InvalidateMarkupText = true;
//
//		// Show text string when list is empty
//		CPaintDC dc(this);

//		int nSavedDC = dc.SaveDC();
//
//		//Set up variables
//		COLORREF clrText = ::GetSysColor(COLOR_WINDOWTEXT);	//system text color
//		COLORREF clrBack = GetBkColor();
//		CRect rc;
//		GetClientRect(&rc);	//get client area of the ListCtrl
//
//		//Now we actually display the text
//		dc.SetTextColor(clrText);	//set the text color
//		dc.SetBkColor(clrBack);	//set the background color
//		CBrush backBrush(clrBack);
//		dc.FillRect(rc, &backBrush);	//fill the client area rect
////		CFont* pOldFont = dc.SelectObject(GetCellFont());	//select a font
//		dc.DrawText(m_strEmptyMarkupText, -1, rc, 
//			DT_CENTER | DT_WORDBREAK | DT_NOPREFIX |
//			DT_NOCLIP | DT_VCENTER | DT_SINGLELINE); //and draw the text
////		dc.SelectObject(pOldFont);
		//// Restore dc
		//dc.RestoreDC(nSavedDC);

		
		CPaintDC dc(this);
		int nColumnCount = m_ctrlHeader.GetItemCount();
		for ( int i = 0; i < nColumnCount; i ++)
		{
			DrawCellBorder(&dc, -1, i);
		}
	}
	else
	{
		CListCtrl::OnPaint();	// default




/*		CPaintDC dc(this);  

		CRect rcClient,rcClip,rcHeader;  
		GetClientRect(&rcClient);  

		GetHeaderCtrl()->GetWindowRect(&rcHeader);  
		dc.GetClipBox(&rcClip);  

		CDC MemDC;  
		CBitmap MemBitmap, *pOldBitmap;  

		MemDC.CreateCompatibleDC(&dc);  
		int nMode = MemDC.SetMapMode(dc.GetMapMode());  

		MemBitmap.CreateCompatibleBitmap(&dc,rcClient.Width(),rcClient.Height());  
		pOldBitmap = MemDC.SelectObject(&MemBitmap);  
		MemDC.FillSolidRect(&rcClient,GetBkColor());  

		CRgn rgn;  
		rgn.CreateRectRgn(rcClip.left,rcClip.top,rcClip.right,rcClip.bottom);  
		MemDC.SelectClipRgn(&rgn);  
		DefWindowProc(WM_PAINT,(WPARAM)MemDC.m_hDC,(LPARAM)0);  
		MemDC.SelectClipRgn(NULL);  

		if (rcClip.top < rcHeader.Height())  
			rcClip.top = rcHeader.Height();  

		dc.BitBlt(rcClip.left,rcClip.top,rcClip.Width(),rcClip.Height(),&MemDC,rcClip.left,rcClip.top,SRCCOPY);  

		MemDC.SetMapMode(nMode);  
		MemDC.SelectObject(pOldBitmap);  
		MemDC.DeleteDC();  
		MemBitmap.DeleteObject();  
		rgn.DeleteObject();*/ 
	}



//	DeleteItem(0);


	//// Draw the lines only for LVS_REPORT mode
	//if( (GetStyle() & LVS_TYPEMASK) == LVS_REPORT )
	//{
	//	// Get the number of columns
	//	CClientDC dc(this );
	//	CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
	//	int nColumnCount = pHeader->GetItemCount();

	//	// The bottom of the header corresponds to the top of the line 
	//	RECT rcHeader;
	//	pHeader->GetClientRect( &rcHeader );

	//	// Now get the client rect so we know the line length and
	//	// when to stop
	//	CRect rcBody;
	//	GetClientRect( &rcBody );

	//	// Define a Rect record the entire ClientRect
	//	CRect rcEntire(rcBody);

	//	// Change the rcBody top, just choose the data row area
	//	rcBody.top = rcHeader.bottom + 1;		


	//	CDC memDC;
	//	CBitmap bitmap;
	//	memDC.CreateCompatibleDC(&dc);
	//	bitmap.CreateCompatibleBitmap(&dc, rcEntire.Width(), rcEntire.Height());


	//	CBitmap* pOldBitmap = memDC.SelectObject(&bitmap); 

	//	memDC.BitBlt(0,0,rcEntire.Width(), rcEntire.Height(), &dc, 0,0,SRCCOPY);




	//	///////////////////////////////////////////////////////////////////
	//	// Draw the horizontal grid lines
	//	///////////////////////////////////////////////////////////////////	

	//	CPen horPen1, horPen2;
	//	horPen1.CreatePen(PS_SOLID, 1, RGB(121,121,121));
	//	horPen2.CreatePen(PS_SOLID, 1, RGB(239,239,239));
	//	memDC.SelectObject(&horPen1);

	//	// Draw header row
	//	memDC.MoveTo(rcHeader.left, rcHeader.top);
	//	memDC.LineTo(rcHeader.right, rcHeader.top);

	//	memDC.SelectObject(&horPen2);
	//	memDC.MoveTo(rcHeader.left, rcHeader.bottom);
	//	memDC.LineTo(rcHeader.right, rcHeader.bottom);

	//	// Draw data row
	//	memDC.SelectObject(&horPen1);
	//	for( int i = 0; i <= GetCountPerPage(); i++ )
	//	{
	//		memDC.MoveTo( rcBody.left,  rcBody.top + m_nRowHeight*i);
	//		memDC.LineTo( rcBody.right, rcBody.top + m_nRowHeight*i);
	//	}
	//	horPen1.DeleteObject();
	//	horPen2.DeleteObject();


	//	///////////////////////////////////////////////////////////////////
	//	// Draw the vertical grid lines
	//	///////////////////////////////////////////////////////////////////		
	//	// The border of the column is offset by the horizontal scroll
	//	int borderx = 0 - GetScrollPos( SB_HORZ );

	//	CPen myVerticalPen(PS_SOLID, 1, RGB(201,201,201));
	//	myVerticalPen.CreateObject();
	//	memDC.SelectObject(&myVerticalPen);

	//	int* pOrderArray = new int[nColumnCount];
	//	VERIFY( GetColumnOrderArray(pOrderArray, nColumnCount) );

	//	for( int i = 0; i < nColumnCount; i++ )
	//	{
	//		// Get the next border
	//		borderx += GetColumnWidth( pOrderArray[i] );

	//		// if next border is outside client area, break out
	//		if( borderx >= rcBody.right ) break;

	//		// Draw the line.
	//		memDC.MoveTo( borderx-1, rcHeader.top);
	//		memDC.LineTo( borderx-1, rcHeader.bottom-1);

	//		memDC.MoveTo( borderx-1, rcBody.top+1);
	//		memDC.LineTo( borderx-1, rcBody.bottom );
	//	}		
	//	myVerticalPen.DeleteObject();


	//	dc.BitBlt(0, 0, rcEntire.Width(), rcEntire.Height(), &memDC, 0,0, SRCCOPY);
	//	memDC.SelectObject(pOldBitmap);
	//	memDC.DeleteDC();
	//	bitmap.DeleteObject();
	//}
}

void CCustomListCtrl::SetRowStatus(int nRow, BOOL bEnable, BOOL bRepaint)
{
	if ( nRow < 0 || nRow >= GetItemCount() )
	{
		return;
	}

	CellFormat* pRowFormat = GetRowFormat(nRow, TRUE);
	pRowFormat->bEnabled = bEnable;

	for ( int i = 0; i < m_ctrlHeader.GetItemCount(); i++)
	{
		CellFormat* pCellFormat = GetCellFormat(nRow, i, TRUE);
		pCellFormat->bEnabled = bEnable;
	}

	// If disabled, de-select it if it is selected;
	if ( !bEnable )
	{
		RowItemData* pRowItemData = reinterpret_cast<RowItemData*>(GetItemData(nRow));
		ASSERT(pRowItemData != NULL);

		pRowItemData->bSelected = FALSE;
		SetItemState(nRow, ~LVIS_SELECTED, LVIS_SELECTED );
	}

	if ( bRepaint )
	{
		CRect rcRow;
		GetItemRect(nRow, &rcRow, LVIR_BOUNDS);
		InvalidateRect(rcRow);
	}
}

BOOL CCustomListCtrl::ConsistentRowSelection(int nRow)
{
	BOOL bSelected;
	RowItemData* pRowItemData = reinterpret_cast<RowItemData*>(GetItemData(nRow));
	ASSERT(pRowItemData != NULL);
	if ( m_bAllowMultlSelection )
	{
		bSelected = pRowItemData->bSelected;
		if ( bSelected )
		{
			SetItemState(nRow, LVIS_SELECTED, LVIS_SELECTED );
		}
		else
		{
			SetItemState(nRow, ~LVIS_SELECTED, LVIS_SELECTED );
		}
	}
	else
	{
		bSelected = GetItemState(nRow, LVIS_SELECTED ) & LVIS_SELECTED;
		pRowItemData->bSelected = bSelected;
	}

	return bSelected;
}

void  CCustomListCtrl::SetAdditionalRowData(int nRow, CString strKey, CString strData)
{
	RowItemData* pRowItemData = reinterpret_cast<RowItemData*>(GetItemData(nRow));
	ASSERT(pRowItemData != NULL);

	pRowItemData->mapAdditionalData[strKey] = strData;
}

void  CCustomListCtrl::SetAdditionalRowData(int nRow, CString strKey, int nData)
{
	CString strData;
	strData.Format(_T("%d"), nData);
	
	SetAdditionalRowData(nRow, strKey, strData);
}

CString  CCustomListCtrl::GetAdditionalRowData(int nRow, CString strKey)
{
	RowItemData* pRowItemData = reinterpret_cast<RowItemData*>(GetItemData(nRow));
	ASSERT(pRowItemData != NULL);

	map<CString,CString>::iterator it = pRowItemData->mapAdditionalData.find(strKey);
	if ( it != pRowItemData->mapAdditionalData.end() )
	{
		return it->second;
	}

	return _T("");
}

BOOL CCustomListCtrl::SwapRow(int nRow1, int nRow2)
{
	if ( nRow1 < 0 || nRow1 >= GetItemCount() )
		return FALSE;
	if ( nRow2 < 0 || nRow2 >= GetItemCount() )
		return FALSE;

	if ( nRow1 == nRow2 )
		return TRUE;

	int nColCount = m_ctrlHeader.GetItemCount();
	for ( int i = 0; i < nColCount; i++)
	{
		CString strCellText1 = GetItemText(nRow1,i);
		CString strCellText2 = GetItemText(nRow2,i);

		SetCellText(nRow1, i, strCellText2, TRUE, FALSE);
		SetCellText(nRow2, i, strCellText1, TRUE, FALSE);
	}

	RowItemData* pRowItemData1 = reinterpret_cast<RowItemData*>(GetItemData(nRow1));
	ASSERT(pRowItemData1 != NULL);
	RowItemData* pRowItemData2 = reinterpret_cast<RowItemData*>(GetItemData(nRow2));
	ASSERT(pRowItemData2 != NULL);

	if ( !m_bAllowMultlSelection )
	{
		BOOL bSelected1 = GetItemState(nRow1, LVIS_SELECTED ) == LVIS_SELECTED;
		BOOL bSelected2 = GetItemState(nRow2, LVIS_SELECTED ) == LVIS_SELECTED;

		SetItemState(nRow2, bSelected1?LVIS_SELECTED:~LVIS_SELECTED, LVIS_SELECTED);
		SetItemState(nRow1, bSelected2?LVIS_SELECTED:~LVIS_SELECTED, LVIS_SELECTED);
	}

	int nCacheIndex1 = pRowItemData1->nCacheIndex;
	int nCacheIndex2 = pRowItemData2->nCacheIndex;
	pRowItemData1->nCacheIndex = nCacheIndex2;
	pRowItemData2->nCacheIndex = nCacheIndex1;

	SetItemData(nRow1, (DWORD_PTR)pRowItemData2);
	SetItemData(nRow2, (DWORD_PTR)pRowItemData1);


	if ( m_pUser )
	{
		m_pUser->OnRowSwapped(this, nRow1, nRow2);
	}

	EnsureVisible(nRow2, FALSE);

	return TRUE;
}

vector<int>	CCustomListCtrl::FindRowByCellText(int nCol, CString strMatchText, BOOL bMatchCase, BOOL bMatchWholeWord)
{
	vector<int> vecMatch;
	int nCount = GetItemCount();

	for ( int i = 0; i < nCount; i++ )
	{
		CString strText = GetItemText(i, nCol);
		BOOL bMatch;
		if ( bMatchWholeWord )
		{
			if ( bMatchCase )
				bMatch = ( strText.Compare(strMatchText) == 0 );
			else
				bMatch = ( strText.CompareNoCase(strMatchText) == 0 );
		}
		else
		{
			CString strTemp = strMatchText;
			if ( !bMatchCase )
			{
				strText.MakeLower();
				strTemp.MakeLower();
			}
			bMatch = ( strText.Find(strTemp) != -1 );
		}

		if ( bMatch )
		{
			vecMatch.push_back(i);
		}
	}

	return vecMatch;
}

vector<int> CCustomListCtrl::FindSelectedRow()
{
	vector<int> vecSelectedRow;


	int nCount = GetItemCount();
	RowItemData* pRowItemData;
	for ( int i = 0; i < nCount; i ++ )
	{
		ConsistentRowSelection(i);
		pRowItemData = reinterpret_cast<RowItemData*>(GetItemData(i));
		if ( pRowItemData->bSelected )
		{
			vecSelectedRow.push_back(i);
		}
	}

	return vecSelectedRow;
}

int CCustomListCtrl::AppendEmptyRow(int nCacheIndex, CellFormat* pDefaultRowFormat)
{
	int nItemCount = GetItemCount();
	return InsertEmptyRow(nItemCount, nCacheIndex, pDefaultRowFormat);
}

int CCustomListCtrl::InsertEmptyRow(int nInsertAfter, int nCacheIndex, CellFormat* pDefaultRowFormat)
{
	RowItemData* pRowItemData = new RowItemData();
	pRowItemData->pDefaultRowFormat = pDefaultRowFormat;
	if ( nCacheIndex == -1 && m_p2DCache)
		pRowItemData->nCacheIndex = m_p2DCache->CacheAppendEmptyRow(rowNew);
	else
		pRowItemData->nCacheIndex = nCacheIndex;

	if ( nInsertAfter < 0 )
		nInsertAfter = 0;
	int nItem = InsertItem(nInsertAfter, _T(""));
	SetItemData(nItem, (DWORD_PTR)pRowItemData);

	return nItem;
}

int CCustomListCtrl::AppendDefaultRow(int nCacheIndex)
{
	int nRow = AppendEmptyRow(nCacheIndex);
	SetDefaultFormatAndValue(nRow);

	return nRow;
}

int CCustomListCtrl::InsertDefaultRow(int nInsertAfter, int nCacheIndex)
{
	int nRow = InsertEmptyRow(nInsertAfter, nCacheIndex);
	SetDefaultFormatAndValue(nRow);

	return nRow;
}

void CCustomListCtrl::SetDefaultFormatAndValue(int nItem)
{
	int nColumnCount = m_ctrlHeader.GetItemCount();
	CellFormat* pCellFormat;
	for ( int i = 0; i < nColumnCount; i ++ )
	{
		pCellFormat = m_defaultListFormat.CopyFormat(levelCell);
		pCellFormat->cellType = m_ctrlHeader.m_gridColumnsList[i].ctrlType;

		if ( m_ctrlHeader.m_gridColumnsList[i].nFormat == LVCFMT_LEFT )
			pCellFormat->uTextAlign = DT_LEFT;
		else if ( m_ctrlHeader.m_gridColumnsList[i].nFormat == LVCFMT_CENTER )
			pCellFormat->uTextAlign = DT_CENTER;
		else
			pCellFormat->uTextAlign = DT_RIGHT;

		SetCell(nItem, i, m_ctrlHeader.m_gridColumnsList[i].strDefaultValue, pCellFormat);
	}
}

void CCustomListCtrl::FilterDataFromCache(const vector<CString>& vecFilter)
{
	ASSERT(m_p2DCache);

	m_p2DCache->CacheStartProtect();

	DeleteAllItems(FALSE);

	int nDataCount = m_p2DCache->CacheGetRowCount();
	int nColumnCount = m_p2DCache->CacheGetColCount();
	ASSERT( m_ctrlHeader.GetItemCount() == nColumnCount);
	ASSERT( vecFilter.size() == nColumnCount);

	int nItem, nPos;
	BOOL bNotMatch;
	CString strCellText, strFilterText;
	CString strStartDateTime, strEndDateTime;
	for ( int i = 0; i < nDataCount; i ++ )
	{
		RowStatus rowstatus = m_p2DCache->CacheGetRowstatus(i);
		if ( rowstatus == rowDeleted || rowstatus == rowHidden )
			continue;

		bNotMatch = FALSE;
		for ( int j = 0; j < nColumnCount; j ++ )
		{
			strFilterText = vecFilter[j];
			if ( strFilterText.GetLength() > 0 )
			{

				strCellText = m_p2DCache->CacheGetItemText(i,j);
				if ( !m_bFilterInCase )
				{
					strFilterText.MakeLower();
					strCellText.MakeLower();
				}
				
				if ( m_ctrlHeader.m_gridColumnsList[j].ctrlType == cellTextDate ||
					 m_ctrlHeader.m_gridColumnsList[j].ctrlType == cellTextTime ||
					 m_ctrlHeader.m_gridColumnsList[j].ctrlType == cellTextDateTime )
				{
					nPos = strFilterText.Find(_T(" ~ "));
					ASSERT(nPos!=-1);
					strStartDateTime = strFilterText.Left(nPos);
					strEndDateTime = strFilterText.Right(strFilterText.GetLength()-nPos-3);

					if ( DateCompare(strStartDateTime, strCellText, m_ctrlHeader.m_gridColumnsList[j].compareType ) == 1 ||
						 DateCompare(strEndDateTime, strCellText, m_ctrlHeader.m_gridColumnsList[j].compareType ) == -1 )
					{
						bNotMatch = TRUE;
						break;
					}
				}
				else
				{
					if ( strCellText.Find(strFilterText) == -1 )
					{
						bNotMatch = TRUE;
						break;
					}
				}
			}
		}

		if ( bNotMatch )
		{
			continue;
		}

		nItem = AppendDefaultRow(i);
		for ( int k = 0; k < nColumnCount; k ++ )
		{
			SetCellText(nItem, k, m_p2DCache->CacheGetItemText(i,k), FALSE, FALSE);
		}
	}

	m_p2DCache->CacheEndProtect();

	SortPrevious();
}

void CCustomListCtrl::FilterDataFromCache()
{
	vector<CString> vecFilter;

	int nColCount = m_ctrlHeader.GetItemCount();

	CString strFilterText;
	for ( int i = 0; i < nColCount; i++)
	{
		strFilterText = m_ctrlHeader.GetFilterText(i);
		if ( m_ctrlHeader.m_gridColumnsList[i].ctrlType == cellTextCombo && strFilterText == _M("ALL"))
		{
			strFilterText = _T("");
		}

		vecFilter.push_back(strFilterText);
	}

	FilterDataFromCache(vecFilter);
}

void CCustomListCtrl::UpdateTableSytle(CustomListStyle& listStyle)
{
	listStyle.gridColumnsList = m_ctrlHeader.m_gridColumnsList;
}

void CCustomListCtrl::SetTableStyle(CustomListStyle& listStyle)
{
	SetClickEmptyAreaToAddRow(listStyle.bClickEmptyAreaToAddRow);
	SetShowSelection(listStyle.bShowSelection);
	SetIgnoreMouse(listStyle.bIgnoreMouse);
	SetMultipleSelection(listStyle.bAllowMultlSelection);
	SetHighlightChangedCellText(listStyle.bHighlightChangedCellText);
	SetSelectThenEdit(listStyle.bClickSelectedCellToEdit);
	EnableFilter(listStyle.bEnableFilter);
	SetRowHeigt(listStyle.nRowHeight);
	SetHeaderHeight(listStyle.nHeaderHeight);
	SetEditNextCellAfterReturnIsHit(listStyle.bEditNextCellAfterReturnIsHit);
	SetEditFirstCellAfterNewRowIsAdded(listStyle.bEditFirstCellAfterNewRowIsAdded);

	m_vecRowColors.clear();
	for ( int i = 0; i < (int)listStyle.vecRowColors.size(); i ++ )
	{
		AddDefaultRowColor(listStyle.vecRowColors[i]);
	}

	SetEmptyTable(FALSE);

	SetColumnInfo(listStyle.gridColumnsList, listStyle.strColumnOrder);
	SetContextMenu(listStyle.contextMenu);

	m_nSortColumn = -1;
	m_ctrlHeader.SetSortArrow(-1,FALSE);
}

void CCustomListCtrl::SetCacheData(I2DCache<CString>* p2DCache)
{
	m_p2DCache = p2DCache;
}

void CCustomListCtrl::SetEmptyTable(BOOL bEmptyDataCache)
{
	DeleteAllItems(bEmptyDataCache);
	while ( DeleteColumn(0) );
	m_ColumnStates.RemoveAll();
	m_ctrlHeader.Invalidate(TRUE);
}

BOOL CCustomListCtrl::OnRowAddedInternal(int nRow)
{
	EnsureVisible(nRow, FALSE);
	if ( m_pUser )
	{
		m_pUser->OnRowAdded(this, nRow);
	}
	if ( m_bEditFirstCellAfterNewRowIsAdded )
	{
		EditNextEditableCell(nRow,-1);
	}

	return TRUE;
}

void CCustomListCtrl::SetContextMenu(const CustomListContextMenu& contextMenu)
{
	m_menuContext.DestroyMenu();
	m_menuContext.CreatePopupMenu();

	int nCount = contextMenu.vecContextMenu.size();
	m_bEnablePopupMenu = ( nCount > 0) ;
		
	for ( int i = 0; i < nCount; i ++ )
	{
		if ( contextMenu.vecContextMenu[i].nMenuID == menuSeperator )
		{
			m_menuContext.AppendMenu(MF_SEPARATOR);
		}
		else
		{
			m_menuContext.AppendMenu(MF_STRING|MF_ENABLED, contextMenu.vecContextMenu[i].nMenuID, _M(contextMenu.vecContextMenu[i].strMenuText));
		}
	}
}

void CCustomListCtrl::EnableContextMenuItem(int nRow, int nCol)
{
	int nCount = GetItemCount();
	BOOL bSelected = FALSE;
	if ( nRow != -1 )
	{
		RowItemData* pRowItemData = reinterpret_cast<RowItemData*>(GetItemData(nRow));
		ASSERT(pRowItemData != NULL);
		bSelected = pRowItemData->bSelected;
	}

	if ( nRow == -1 || nCol == -1 )
	{
		m_menuContext.EnableMenuItem(menuCopy, MF_DISABLED | MF_GRAYED);
	}
	else
	{
		m_menuContext.EnableMenuItem(menuCopy, MF_ENABLED);
	}

	if ( !bSelected || m_bReadOnly)
	{
		m_menuContext.EnableMenuItem(menuDeleteAllSelectedItem, MF_DISABLED | MF_GRAYED);
	}
	else
	{
		m_menuContext.EnableMenuItem(menuDeleteAllSelectedItem, MF_ENABLED);
	}

	if ( nRow == -1 || m_bReadOnly)
	{
		m_menuContext.EnableMenuItem(menuInsertItemBeforeThisRow, MF_DISABLED | MF_GRAYED);
	}
	else
	{
		m_menuContext.EnableMenuItem(menuInsertItemBeforeThisRow, MF_ENABLED);
	}

	if ( m_bReadOnly)
	{
		m_menuContext.EnableMenuItem(menuAppendItemToBottom, MF_DISABLED | MF_GRAYED);
	}
	else
	{
		m_menuContext.EnableMenuItem(menuAppendItemToBottom, MF_ENABLED);
	}
	
	if ( nRow < 1  || m_bReadOnly)
	{
		m_menuContext.EnableMenuItem(menuMoveItemTop, MF_DISABLED | MF_GRAYED);
		m_menuContext.EnableMenuItem(menuMoveItemUp, MF_DISABLED | MF_GRAYED);
	}
	else
	{
		m_menuContext.EnableMenuItem(menuMoveItemTop, MF_ENABLED);
		m_menuContext.EnableMenuItem(menuMoveItemUp, MF_ENABLED);
	}


	if ( nRow == -1 || nRow == nCount - 1 || m_bReadOnly)
	{
		m_menuContext.EnableMenuItem(menuMoveItemDown, MF_DISABLED | MF_GRAYED);
		m_menuContext.EnableMenuItem(menuMoveItemBottom, MF_DISABLED | MF_GRAYED);
	}
	else
	{
		m_menuContext.EnableMenuItem(menuMoveItemDown, MF_ENABLED);
		m_menuContext.EnableMenuItem(menuMoveItemBottom, MF_ENABLED);
	}

	if ( nCount == 0 || !m_bAllowMultlSelection)
	{
		m_menuContext.EnableMenuItem(menuSelectAll, MF_DISABLED | MF_GRAYED);
	}
	else
	{
		m_menuContext.EnableMenuItem(menuSelectAll, MF_ENABLED);
	}

	if ( !GetFirstSelectedItemPosition() || !m_bAllowMultlSelection )
	{
		m_menuContext.EnableMenuItem(menuSelectNone, MF_DISABLED | MF_GRAYED);
	}
	else
	{
		m_menuContext.EnableMenuItem(menuSelectNone, MF_ENABLED);
	}
}

void CCustomListCtrl::OnContextMenuAction(int nMenuID, int nRow, int nCol)
{
	int nNewRow = -1;
	switch ( (CustomListMenuID)nMenuID)
	{
	case menuDeleteAllSelectedItem:	
		DeleteAllSelectedItems(TRUE, TRUE);
		break;
	case menuInsertItemBeforeThisRow: 
		nNewRow = InsertDefaultRow(nRow);
		OnRowAddedInternal(nNewRow);
		break;
	case menuAppendItemToBottom: 
		nNewRow = AppendDefaultRow();
		OnRowAddedInternal(nNewRow);
		break;
	case menuMoveItemTop:
		for ( int i = nRow; i > 0; i -- )
			SwapRow(i, i-1);
		break;
	case menuMoveItemUp: 
		SwapRow(nRow,nRow-1);
		break;
	case menuMoveItemDown: 
		SwapRow(nRow,nRow+1);
		break;
	case menuMoveItemBottom:
		for ( int i = nRow; i < GetItemCount()-1; i ++ )
			SwapRow(i, i+1);
		break;
	case menuCopy:
		CopyCellTextToClipboard(nRow,nCol);
		break;
	case menuSelectAll:
		SetAllItemSelected(TRUE);
		break;
	case menuSelectNone:
		SetAllItemSelected(FALSE);
		break;
	default:
		break;
	}
}

void CCustomListCtrl::SetAllItemSelected(BOOL bSelected)
{
	int nCount = GetItemCount();

	if ( m_bAllowMultlSelection )
	{
		RowItemData* pRowItemData;
		for ( int i = 0; i < nCount; i ++ )
		{
			pRowItemData = reinterpret_cast<RowItemData*>(GetItemData(i));
			ASSERT(pRowItemData != NULL);
			pRowItemData->bSelected = bSelected;
		}

		Invalidate(TRUE);
	}
	else
	{
		for ( int i = 0; i < nCount; i ++ )
		{
			SetItemState(i, LVIS_SELECTED, LVIS_SELECTED );
		}
	}
}

void CCustomListCtrl::CopyCellTextToClipboard(int nRow, int nCol)
{
	CString strText = GetItemText(nRow,nCol);
	CopyTextToClipboard(strText);
}


void CCustomListCtrl::PasteCellTextFromClipboard( int nRow, int nCol )
{
	OpenClipboard();

	HGLOBAL hMem = NULL;
	LPCTSTR lpStr = NULL;

	hMem = GetClipboardData(CF_UNICODETEXT);
	LPCWSTR pData = (LPCWSTR)::GlobalLock(hMem);

	DWORD dwMinSize = WideCharToMultiByte(CP_OEMCP,NULL,pData,-1,NULL,0,NULL,FALSE);
	CString strText;
	WideCharToMultiByte(CP_OEMCP,NULL,pData,-1,(LPSTR)(LPCTSTR)strText,dwMinSize,NULL,FALSE);

	::GlobalUnlock(hMem);
	CloseClipboard();

	SetCellText(nRow,nCol,strText,TRUE,TRUE);
}

void CCustomListCtrl::ShowNotes( int nRow, int nCol )
{
	CDynDialogEx dlg(this);		//create the dynamic dialog, using this as parent window

	dlg.SetUseSystemButtons(FALSE);
	dlg.SetWindowTitle(_M("Notes"));

	dlg.SetFont(GetFont());

	CString strText = GetItemText(nRow,nCol);
	CRect rcNotes(5,10,300,200);
	dlg.AddDlgControl(_T("EDIT"), _T(""), STYLE_EDIT | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_WANTRETURN, EXSTYLE_EDIT , rcNotes, (void*)&strText);

	CRect rcOk(160,210,220,225);
	dlg.AddDlgControl(BUTTON, _M("OK"), STYLE_BUTTON, EXSTYLE_BUTTON, &rcOk, NULL, IDOK);

	CRect rcCancel(240,210,300,225);
	dlg.AddDlgControl(BUTTON, _M("Cancel"), STYLE_BUTTON, EXSTYLE_BUTTON, &rcCancel, NULL, IDCANCEL);


	UINT nRet = dlg.DoModal();

	m_nEditRow = nRow;
	m_nEditCol = nCol;

	OnCellCtrlHiding(nRet==IDOK, strText);
	OnCellCtrlHidden(nRet==IDOK);
}

void CCustomListCtrl::SetQueryStyle( BOOL bCaseSensitive, BOOL bQueryWhenEditting )
{
	SetFilterInCase(bCaseSensitive);
	SetFilterWhenEditting(bQueryWhenEditting);
}

void CCustomListCtrl::RebuildContextMenu( const CustomListContextMenu& contextMenu )
{
	SetContextMenu(contextMenu);
}

void CCustomListCtrl::LoadDefaultIcon()
{
	SHFILEINFO    sfi;
	HIMAGELIST hImagelist;
	hImagelist=(HIMAGELIST)SHGetFileInfo(_T(""), 
		0,
		&sfi, 
		sizeof(SHFILEINFO), 
		SHGFI_SYSICONINDEX | SHGFI_SMALLICON ) ;

	CImageList* pImageList = CImageList::FromHandle(hImagelist);

	SHFILEINFO sInfo;
	CString strFile = _T("folder");
	SHGetFileInfo(strFile, FILE_ATTRIBUTE_DIRECTORY, &sInfo, sizeof(sInfo),
		SHGFI_USEFILEATTRIBUTES|SHGFI_SMALLICON|SHGFI_SYSICONINDEX);

	m_hFolder = pImageList->ExtractIcon(sInfo.iIcon);
}

BOOL CCustomListCtrl::GetEditRect( int nRow, int nCol, CRect& rcEdit )
{
	CellFormat* pCellFormat = GetCellFormat(nRow,nCol);
	if ( pCellFormat == NULL )
		return FALSE;

	if ( pCellFormat->cellType == cellTextEdit )
	{
		GetSubItemRect(nRow,nCol,LVIR_LABEL,rcEdit);
	}
	else if ( pCellFormat->cellType == cellCtrlDirectory || pCellFormat->cellType == cellCtrlFile )
	{
		rcEdit = pCellFormat->rcCellText;
		rcEdit.left += 1;
		rcEdit.right -= 1;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

void CCustomListCtrl::CopyTextToClipboard( const CString& strText )
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

void CCustomListCtrl::SetColumnWidth( int nCol, int nWidth )
{
	CListCtrl::SetColumnWidth(nCol, nWidth);
	m_ctrlHeader.m_gridColumnsList[nCol].nWidth = nWidth;
}

void CCustomListCtrl::RefreshRow( int nRow )
{
	if ( nRow < 0 || nRow >= GetItemCount() )
		return;

	CRect rc;
	GetItemRect(nRow, &rc, LVIR_BOUNDS);
	InvalidateRect(rc, TRUE);
}

COLORREF CCustomListCtrl::StringToColorRef( const CString& strRGB )
{
	int nR, nG, nB;
	nR = nG = nB = 0;
	sscanf_s(strRGB, _T("RGB(%d,%d,%d)"), &nR, &nG, &nB );

	return RGB(nR,nG,nB);
}

CString CCustomListCtrl::ColorRefToString( COLORREF cr )
{
	CString strRGB;
	strRGB.Format(_T("RGB(%d,%d,%d)"), GetRValue(cr), GetGValue(cr), GetBValue(cr));
	return strRGB;
}

CString CCustomListCtrl::BrowserForFolder(HWND hwndOwner, const char* strTitle, CString strInitDir)
{
	const char* szTitle          = strTitle;
	char szDisplayName[MAX_PATH] = "";
	char szPath[MAX_PATH]        = "";
	BROWSEINFO bi;

	bi.hwndOwner      = hwndOwner;               // Handle of the owner window
	bi.pidlRoot       = NULL;                 // Desktop folder is used
	bi.lpszTitle      = szTitle;              // Title of the dialog box
	bi.pszDisplayName = szDisplayName;        // Buffer for selected folder name
	bi.ulFlags        = BIF_RETURNONLYFSDIRS; // Only returns file system directories
	bi.lpfn           = BrowseCallbackProc;
	bi.lParam         = (long)strInitDir.GetBuffer(0);

	LPITEMIDLIST pItemIDList = SHBrowseForFolder(&bi);

	if (pItemIDList && SHGetPathFromIDList(pItemIDList, szPath))
	{
		// Avoid memory leaks by deleting the PIDL using the shell's task allocator
		IMalloc* pMalloc;
		if (SHGetMalloc(&pMalloc) != NOERROR) {

		}
		pMalloc->Free(pItemIDList);
		if (pMalloc)
			pMalloc->Release();
	}
	return CString(szPath);
}