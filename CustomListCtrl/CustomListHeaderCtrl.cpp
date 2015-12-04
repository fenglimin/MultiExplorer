// FilterSortHeaderCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "CustomListHeaderCtrl.h"
#include "DynDialogEx.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFilterSortHeaderCtrl




CCustomListCtrlHeader::CCustomListCtrlHeader()
{
	m_bFilterWhenEditting = FALSE;
	m_bEnableSort = TRUE;
	m_pEdit=NULL;
	m_nEditColumn=-1;
	m_nComboColumn = -1;
	m_bEndEditSent=TRUE;

	m_pCombo = NULL;
	m_iSortColumn = -1;
	m_bSortAscending = TRUE;
	m_ComboSupportColumnsList.RemoveAll();


	m_bReadOnly = FALSE;
	m_Height = 40;

	m_bEnableFilter = TRUE;

	m_bmpArrowUp.Attach(CreateSortBitmap(TRUE));
	m_bmpArrowDown.Attach(CreateSortBitmap(FALSE));

	m_hPicLoadFailed = CreateTextBMP("N/A");
}

CCustomListCtrlHeader::~CCustomListCtrlHeader()
{
	for (int i=0;i<m_arFilters.GetSize();i++)
		delete (CFilterInfo*)m_arFilters[i];

	DeleteObject(m_hPicLoadFailed);
}


BEGIN_MESSAGE_MAP(CCustomListCtrlHeader, CHeaderCtrl)
	//{{AFX_MSG_MAP(CFilterSortHeaderCtrl)
	ON_WM_LBUTTONDOWN()
	ON_MESSAGE(FLM_EDITTEXTCHANGING,OnEditTextChanging)
	ON_MESSAGE(FLM_EDITTEXTCHANGED,OnEditTextChanged)
	ON_MESSAGE(FLM_COMBOSELCHANGING,OnComboSelChanging)
	ON_MESSAGE(FLM_COMBOSELCHANGED,OnComboSelChanged)
	ON_MESSAGE(HDM_LAYOUT, OnLayout)
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFilterSortHeaderCtrl message handlers
void CCustomListCtrlHeader::SetSortArrow( const int iSortColumn, const BOOL bSortAscending )
{
	m_iSortColumn = iSortColumn;
	m_bSortAscending = bSortAscending;

	if ( m_iSortColumn == -1 )
		return;

	// change the item to owner drawn.
	HD_ITEM hditem;

	hditem.mask = HDI_FORMAT;
	VERIFY( GetItem( iSortColumn, &hditem ) );
	hditem.fmt |= HDF_OWNERDRAW;
	VERIFY( SetItem( iSortColumn, &hditem ) );

	// invalidate the header control so it gets redrawn
	Invalidate();
}

void CCustomListCtrlHeader::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	if ( m_Height == 0 )
		return;

	HDITEM hdi; 
	char szBuffer[1024];

	hdi.mask = HDI_TEXT | HDI_FORMAT | HDI_IMAGE;
	hdi.pszText = szBuffer;
	hdi.cchTextMax = 1024;

	VERIFY( GetItem(lpDrawItemStruct->itemID, &hdi) );


	CRect rcHeader=lpDrawItemStruct->rcItem; // the header
	m_gridColumnsList[lpDrawItemStruct->itemID].nWidth = rcHeader.Width();
	CRect rcFilter=lpDrawItemStruct->rcItem; // the filter area

	
	if ( m_bEnableFilter )
	{
		// Half/Half the Header title and Filter
		rcHeader.bottom = rcHeader.top + rcHeader.Height()/2;
	}
	

	// if the item is selected, we have to shift the rect
	if( lpDrawItemStruct->itemState == ODS_SELECTED )
	{
		rcHeader.bottom += 2;
	}
	rcFilter.top = rcHeader.bottom;
	

	// need a dc!
	CDC dc;
	dc.Attach(lpDrawItemStruct->hDC);

	int nOldDC=dc.SaveDC();

	// clipping region 
	CRgn rgHeader;
	rgHeader.CreateRectRgnIndirect(&rcHeader); // Title Rect, not including the filter
	dc.SelectObject(&rgHeader);
	rgHeader.DeleteObject();

	// Draw the background
	// Xiaoming
	CBrush brGray(COLOR_FILTER_BK);
	dc.FillRect(rcHeader, &brGray);	

	CRect rc( rcHeader );

	// determine the format for drawing the column label.
	UINT uFormat = DT_SINGLELINE | DT_NOPREFIX | DT_NOCLIP | DT_VCENTER | DT_END_ELLIPSIS ;
	UINT uAlign;

	if( (hdi.fmt & HDF_JUSTIFYMASK) == HDF_LEFT)
		uAlign = DT_LEFT;
	else if( (hdi.fmt & HDF_JUSTIFYMASK) == HDF_RIGHT)
		uAlign = DT_RIGHT;
	else
		uAlign = DT_CENTER;

	uFormat |= uAlign;

	rc.left += 4;
	rc.right -= 4;

	// adjust the rect if the mouse button is pressed on it.
	if( lpDrawItemStruct->itemState == ODS_SELECTED )
	{
		rc.left++;
		rc.top += 2;
		rc.right++;
	}

	// Set font
	CFont* pOldFont = NULL;
	if (m_FilterFont.m_hObject)
		pOldFont=dc.SelectObject(&m_FilterFont);
	
	// Draw Column Title
	if( rc.left < rc.right )
	{
		// XIAOMING
		dc.SetBkMode(TRANSPARENT);
		(void)dc.DrawText( _M(szBuffer), -1, rc, uFormat );
	}

	// Draw the sort arrow.
	if( lpDrawItemStruct->itemID == (UINT)m_iSortColumn )
	{
		// XIAOMING
		POINT p;
		CDC tempDc;
		tempDc.CreateCompatibleDC(&dc);		
		BITMAP bmp;

		if( m_bSortAscending )
		{
			m_bmpArrowUp.GetBitmap( &bmp );
			tempDc.SelectObject( &m_bmpArrowUp );			
		}
		else
		{
			m_bmpArrowDown.GetBitmap( &bmp );
			tempDc.SelectObject( &m_bmpArrowDown );			
		}

		if ( uAlign == DT_RIGHT)
		{
			p.x = rc.left + 2;
		}
		else
		{
			p.x = rc.right - bmp.bmWidth - 2;
		}
		p.y = rc.top + (rc.Height() - bmp.bmHeight)/2;

		dc.BitBlt(p.x, p.y, bmp.bmWidth, bmp.bmHeight, &tempDc, 0, 0, SRCCOPY );
		tempDc.DeleteDC();
	}
	
	// this is a generic 'offset' used to put space between labels and borders
	int nSpaceOffset=dc.GetTextExtent(" ").cx*2;

	int nTextWidth= dc.GetTextExtent(szBuffer,strlen(szBuffer)).cx;

	// some useful booleans
	BOOL bImage=((hdi.fmt & HDF_IMAGE) && GetImageList() && (hdi.iImage!=-1));
	BOOL bImageOnRight=(hdi.fmt & HDF_BITMAP_ON_RIGHT);

	/////////////////////////////////
	// draw the header area
	/////////////////////////////////

	int cxImage=0,cyImage=0;
	int xImage=0,yImage=0;

	rcHeader.DeflateRect(nSpaceOffset,0,nSpaceOffset,0);

	CRect rcText = rcHeader;

	// check if the item is selected
	if (lpDrawItemStruct->itemState==ODS_SELECTED)
	{
		rcText.left++;
		rcText.top +=2;
		rcText.right++;

		xImage++;
		yImage+=2;
	}


	/////////////////////////////////
	// draw the filter area
	/////////////////////////////////

	if ( m_bEnableFilter )
	{
		// clipping region 
		CRgn rgFilter;
		rgFilter.CreateRectRgnIndirect(&rcFilter);
		dc.SelectObject(&rgFilter);
		rgFilter.DeleteObject();

		if (GetFilterStatus(lpDrawItemStruct->itemID)==FILTER_ENABLED)
			dc.FillSolidRect(rcFilter, COLOR_ACQ_CORDEREDIT_BK);
		else
			dc.FillSolidRect(rcFilter, COLOR_ACQ_CORDEREDIT_BK_DISABLED);

		dc.Draw3dRect(rcFilter,GetSysColor(COLOR_3DSHADOW),GetSysColor(COLOR_3DHILIGHT));

		rcFilter.DeflateRect(5,1);

		CString strFilter = GetFilterText(lpDrawItemStruct->itemID);

		dc.SetTextColor(COLOR_ACQ_CORDEREDIT_TEXT);

		// Adjust the rect if the mouse button is pressed on it
		if( lpDrawItemStruct->itemState == ODS_SELECTED )
		{
			rcFilter.left++;
			rcFilter.top +=2;
			rcFilter.right++;
		}


		if (GetFilterStatus(lpDrawItemStruct->itemID)==FILTER_DISABLED)
		{
			if (m_strFilterDisabled.GetLength())
				strFilter = m_strFilterDisabled;

			dc.SetBkMode(TRANSPARENT);
			dc.SetTextColor(::GetSysColor(COLOR_3DHILIGHT));

			CRect rc1=rcFilter;

			rc1.OffsetRect(1,1);

			::DrawText(lpDrawItemStruct->hDC, strFilter, strFilter.GetLength(), 
				&rc1, DT_SINGLELINE|DT_VCENTER|DT_LEFT| DT_END_ELLIPSIS);

			dc.SetTextColor(::GetSysColor(COLOR_3DSHADOW));
		}

		::DrawText(lpDrawItemStruct->hDC, strFilter, strFilter.GetLength(), 
			&rcFilter, DT_SINGLELINE|DT_VCENTER|DT_LEFT| DT_END_ELLIPSIS);
	}	

	if (pOldFont)
		dc.SelectObject(pOldFont);

	dc.RestoreDC(nOldDC);
	dc.Detach();
}

LRESULT CCustomListCtrlHeader::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	if (message==HDM_INSERTITEMA || 
		message==HDM_INSERTITEMW)
	{
		CFilterInfo* pNewFilter=new CFilterInfo();
		m_arFilters.InsertAt((int)wParam,pNewFilter);
	}

	if (message==HDM_DELETEITEM)
	{
		delete (CFilterInfo*)m_arFilters[(int)wParam];
		m_arFilters.RemoveAt((int)wParam);
	}

	if (message==HDM_LAYOUT)
	{
		HD_LAYOUT *layout	=(HD_LAYOUT*)lParam;
		RECT *rect			=layout->prc;
		WINDOWPOS *pos		=layout->pwpos;
		LRESULT lRes		=CWnd::DefWindowProc(message,wParam,lParam);

		int nHeight=0;

		CalcFontHeight();

		nHeight=m_nFont1Height+m_nFont2Height+4;

		pos->cy				= (int)(nHeight*1.2);
		rect->top			= (int)(nHeight*1.2);

		return lRes;
	}
	
	return CHeaderCtrl::DefWindowProc(message, wParam, lParam);
}

void CCustomListCtrlHeader::OnSize(UINT nType, int cx, int cy) 
{
	CHeaderCtrl::OnSize(nType, cx, cy);
	
	CalcFontHeight();
}

void CCustomListCtrlHeader::SetReadOnly(BOOL bRdOnly)
{
	m_bReadOnly = bRdOnly;
	if(m_bReadOnly)
	{
		for (int i=0;i<GetItemCount();i++)
		{
			SetFilterStatus(i,FILTER_DISABLED);
		}
	}
	else
	{
		for (int i=0;i<GetItemCount();i++)
		{
			SetFilterStatus(i,FILTER_ENABLED);
		}
		SetDisableStatus();
	}
}


void CCustomListCtrlHeader::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if( m_bReadOnly || m_bEnableFilter == FALSE )
	{
//		if ( m_bEnableSort )
		{
			CHeaderCtrl::OnLButtonDown(nFlags, point);
		}
		return;		
	}

	if (m_pEdit)
		HideEdit(FALSE);
	if (m_pCombo)
		HideCombo(FALSE);
	//if (m_pCheckCombo)
	//	HideCheckCombo(FALSE);

	int nColumn=-1;

	// HitTest
	for (int i=0;i<GetItemCount();i++)
	{
		if (GetFilterStatus(i)!=FILTER_NONE && GetFilterStatus(i) != FILTER_DISABLED)
		{
			CRect rc;
			
			GetItemRect(i,rc);

			rc.top += rc.Height()/2 + 2;

			rc.DeflateRect(3,2);

			if (rc.PtInRect(point))
				nColumn=i;
		}
	}

	// Hijacked by Xiaoming.Zhao
	if(nColumn != -1)
	{
		CellWindowType ctrlType;
		ctrlType = GetColumnCtrlType( nColumn );

		if ( ctrlType == cellTextEdit || ctrlType == cellTextNotes)
		{
			CString	obStrValidChars;
			obStrValidChars = m_gridColumnsList[nColumn].strValidChars;
			ShowEdit(nColumn,obStrValidChars);
			return;
		}
		else if ( ctrlType == cellTextCombo )
		{
			CStringList obComboItemsList;
			GetComboItemsList(nColumn, obComboItemsList);
			ShowCombo(nColumn,obComboItemsList);
			return;
		}
		else if ( ctrlType == cellTextDate || ctrlType == cellTextTime || ctrlType == cellTextDateTime)
		{
			m_nEditColumn = nColumn;
			DoDateTimeFilter(ctrlType);
			return;
		}
	}

	if ( m_bEnableSort )
	{
		CHeaderCtrl::OnLButtonDown(nFlags, point);
	}
}

void CCustomListCtrlHeader::ShowCombo(int nColumn,CStringList& rComboItemsList)
{
//	FLCN_SHOWINGCOMBO
	if(NULL == m_pCombo)
	{
		CRect rc,rcHeader;

		GetItemRect(nColumn,rc);
		GetWindowRect(rcHeader);

		rc.top += rc.Height()/2+2;
		rc.bottom = rc.top + 300;

		rc.right--;

		DWORD m_dwStyle = WS_TABSTOP | WS_CHILD | WS_VISIBLE | CBS_AUTOHSCROLL | WS_VSCROLL|
							CBS_DROPDOWNLIST;

		m_nComboColumn = nColumn;
		m_nCurrentWorkingColumn = nColumn;
		m_nEditColumn = nColumn;

		m_pCombo = new CFilterCombo();


		m_pCombo->Create(m_dwStyle, rc, this, 0);
		// Set the proper font
		if (m_FilterFont.m_hObject)
			m_pCombo->SetFont(&m_FilterFont);
		else
			m_pCombo->SetFont(GetFont());

		m_pCombo->SetItemHeight(-1, m_Height/2-9);

		int iCurSel = -1;
		int itmp = 0;
		for (POSITION Pos_ = rComboItemsList.GetHeadPosition(); Pos_ != NULL;)
		{
			CString strList = (LPCTSTR) (rComboItemsList.GetNext(Pos_));
			m_pCombo->AddString(strList);
			if(GetFilterText(nColumn).Compare(strList)==0)
			{
				iCurSel = itmp;	
			}
			itmp++;
		}
		m_pCombo->SetCurSel(iCurSel);
	//	CRect rect;
	//	m_pCombo->GetDroppedControlRect(rect);
		m_pCombo->SetFocus();
	}
}

void CCustomListCtrlHeader::HideCombo(BOOL bValidate)
{
	if (m_pCombo)
	{
//
		CString strVal;
		CString strFilter;
		
		m_pCombo->GetWindowText(strVal);

	//	m_pCombo->DestroyWindow();
		delete m_pCombo;
		m_pCombo=NULL;
		if (bValidate)
		{
		//	strFilter = GetFilterText(m_nComboColumn);
		//	if (!strVal.IsEmpty())
			{
				NMCUSTOMHDR nmh;
				nmh.code=FLCN_FILTERCHANGING;
				nmh.hwndFrom=m_hWnd;
				nmh.idFrom=m_nComboColumn;
				nmh.szNewText=strVal.GetBuffer(strVal.GetLength()+1);

				LRESULT lRes=0;

				if (GetParent())
					lRes=GetParent()->SendMessage(WM_NOTIFY,(WPARAM)m_hWnd,(LPARAM)&nmh);

				if (lRes==0)
				{
					GetFilter(m_nComboColumn)->m_strFilter=strVal;

					Invalidate();

					UpdateWindow();

					NMCUSTOMHDR nmh;
					nmh.code=MSG_CUSTOM_LIST_FILTER_TEXT_CHANGED;
					nmh.hwndFrom=m_hWnd;
					nmh.idFrom=m_nComboColumn;
					nmh.szNewText=strVal.GetBuffer(strVal.GetLength()+1);

					if ( m_nComboColumn >= 0 && m_nComboColumn < (int)m_gridColumnsList.size() )
					{
						m_gridColumnsList[m_nComboColumn].strHeaderFilter = "";
						CustomComboboxItemList::iterator it;
						for ( it =  m_gridColumnsList[m_nComboColumn].dropdownValueList.begin();
							  it != m_gridColumnsList[m_nComboColumn].dropdownValueList.end();
							  it++)
						{
							if ( (*it).strText == strVal )
							{
								m_gridColumnsList[m_nComboColumn].strHeaderFilter = (*it).strValue;
								break;
							}
						}						
					}

					if (GetParent())
						GetParent()->SendMessage(WM_NOTIFY,(WPARAM)m_hWnd,(LPARAM)&nmh);
				}
			}
		}
	}
}

void CCustomListCtrlHeader::ShowCheckCombo(int nColumn,CStringList& rComboItemsList)
{
//	FLCN_SHOWINGCOMBO

}

void CCustomListCtrlHeader::HideCheckCombo(BOOL bValidate)
{

}

void CCustomListCtrlHeader::ShowEdit(int nColumn,CString& rStrValidChars)
{
	TRACE("Show\r\n");
	if (m_pEdit==NULL)
	{
		CRect rc;//rcHeader;

		GetItemRect(nColumn,rc);

		rc.left   += 2;
		rc.top    += rc.Height()/2 + 2;
		rc.bottom -= 2;
		rc.right--;

		LRESULT lRes=0;

		NMHDR nmh;
		nmh.code=FLCN_SHOWINGEDIT;
		nmh.hwndFrom=m_hWnd;
		nmh.idFrom=nColumn;

		if (GetParent())
			lRes=GetParent()->SendMessage(WM_NOTIFY,(WPARAM)m_hWnd,(LPARAM)&nmh);

		if (lRes==0)
		{
			if (m_bEndEditSent)
			{
				NMHDR nmh;
				nmh.code=FLCN_BEGINFILTEREDIT;
				nmh.hwndFrom=m_hWnd;
				nmh.idFrom=nColumn;

				if (GetParent())
					GetParent()->SendMessage(WM_NOTIFY,(WPARAM)m_hWnd,(LPARAM)&nmh);

				m_bEndEditSent=FALSE;
			}

			m_nEditColumn = nColumn;
			m_nCurrentWorkingColumn = nColumn;

			m_pEdit=new CFilterEdit();

			m_pEdit->CreateEx(WS_EX_CLIENTEDGE | WS_EX_NOPARENTNOTIFY ,"EDIT",NULL,ES_WANTRETURN | WS_VISIBLE | WS_CHILD | WS_GROUP | ES_AUTOHSCROLL,rc,this,IDC_ED_EDIT,NULL);

			if (m_FilterFont.m_hObject)
				m_pEdit->SetFont(&m_FilterFont);
			else
				m_pEdit->SetFont(GetFont());
			
			m_pEdit->SetValidChars(rStrValidChars);

			m_pEdit->SetWindowText(GetFilter(m_nEditColumn)->m_strFilter);

			m_pEdit->SetSel(0,m_pEdit->GetWindowTextLength());

			m_pEdit->SetFocus();
		}
	}
}

void CCustomListCtrlHeader::HideEdit(BOOL bValidate)
{
	if (m_pEdit)
	{
		CString strVal;
		CString strFilter;
		m_pEdit->GetWindowText(strVal);

		m_pEdit->DestroyWindow();

		m_pEdit=NULL;

		if (bValidate)
		{
			strFilter = GetFilterText(m_nEditColumn);
			if (!strVal.IsEmpty()||!strFilter.IsEmpty())
			{
				//NMCUSTOMHDR nmh;
				//nmh.code=FLCN_FILTERCHANGING;
				//nmh.hwndFrom=m_hWnd;
				//nmh.idFrom=m_nEditColumn;
				//nmh.szNewText=strVal.GetBuffer(strVal.GetLength()+1);
				LRESULT lRes=0;

				//if (GetParent())
				//	lRes=GetParent()->SendMessage(WM_NOTIFY,(WPARAM)m_hWnd,(LPARAM)&nmh);

				if (lRes==0)
				{
					GetFilter(m_nEditColumn)->m_strFilter=strVal;

					Invalidate();

					UpdateWindow();

					NMCUSTOMHDR nmh;
					nmh.code=MSG_CUSTOM_LIST_FILTER_TEXT_CHANGED;
					nmh.hwndFrom=m_hWnd;
					nmh.idFrom=m_nEditColumn;
					nmh.szNewText=strVal.GetBuffer(strVal.GetLength()+1);

					if ( m_nEditColumn >= 0 && m_nEditColumn < (int)m_gridColumnsList.size() )
					{
						m_gridColumnsList[m_nEditColumn].strHeaderFilter = strVal;
					}

					if (GetParent())
						GetParent()->SendMessage(WM_NOTIFY,(WPARAM)m_hWnd,(LPARAM)&nmh);
				}
			}
		}
	}
}
LRESULT CCustomListCtrlHeader::OnComboSelChanged(WPARAM wParam, LPARAM lParam)
{
	HideCombo(wParam);
	HideCheckCombo(wParam);
	if(lParam)
	{
		int nCount=0;

		int nOrd=IndexToOrder(m_nEditColumn);

		do
		{
			nOrd++;
			nOrd=nOrd % GetItemCount();

			nCount++;
		}
		while (nCount!=GetItemCount() && GetFilterStatus(OrderToIndex(nOrd))!=FILTER_ENABLED);

		m_nEditColumn=OrderToIndex(nOrd);
		m_nComboColumn=m_nEditColumn;
//		if(ISEDIT[m_nEditColumn])
		{
//			ShowEdit(m_nEditColumn);
		}
//		else
		{
	//		ShowCombo(m_nComboColumn);
		}
	}
	return 0;

}

LRESULT CCustomListCtrlHeader::OnEditTextChanging(WPARAM wParam, LPARAM lParam)
{
	if (m_pEdit && m_bFilterWhenEditting)
	{
		CString strVal;
		CString strFilter;
		m_pEdit->GetWindowText(strVal);

		GetFilter(m_nEditColumn)->m_strFilter=strVal;
		NMCUSTOMHDR nmh;
		nmh.code=MSG_CUSTOM_LIST_FILTER_TEXT_CHANGED;
		nmh.hwndFrom=m_hWnd;
		nmh.idFrom=m_nEditColumn;
		nmh.szNewText=strVal.GetBuffer(strVal.GetLength()+1);

		if ( m_nEditColumn >= 0 && m_nEditColumn < (int)m_gridColumnsList.size() )
		{
			m_gridColumnsList[m_nEditColumn].strHeaderFilter = strVal;
		}

		if (GetParent())
			GetParent()->SendMessage(WM_NOTIFY,(WPARAM)m_hWnd,(LPARAM)&nmh);
	}

	return 0;
}

LRESULT CCustomListCtrlHeader::OnEditTextChanged(WPARAM wParam, LPARAM lParam)
{
	TRACE("OnEditTextChanged\r\n");
	HideEdit(wParam);

	if (lParam)
	{
		int nCount=0;
		int nOrd=IndexToOrder(m_nEditColumn);

		do
		{
			nOrd++;
			nOrd=nOrd % GetItemCount();
			nCount++;
		}
		while (nCount!=GetItemCount() && GetFilterStatus(OrderToIndex(nOrd))!=FILTER_ENABLED);

		m_nEditColumn=OrderToIndex(nOrd);
		m_nComboColumn=m_nEditColumn;
	}
	else
	{
		NMHDR nmh;
		nmh.code=FLCN_ENDFILTEREDIT;
		nmh.hwndFrom=m_hWnd;
		nmh.idFrom=0;

		if (GetParent())
			GetParent()->SendMessage(WM_NOTIFY,(WPARAM)m_hWnd,(LPARAM)&nmh);

		m_bEndEditSent=TRUE;

		if (GetParent())
			GetParent()->SetFocus();
	}

	return 0;
}





void CCustomListCtrlHeader::SetFilterStatus(int nIndex, UINT uStatus)
{
	if(nIndex>=0)
	{
		GetFilter(nIndex)->m_nStatus=uStatus;

		if (IsWindow(m_hWnd))
		{
			Invalidate();
			UpdateWindow();
		}
	}

}

void CCustomListCtrlHeader::SetFilterDisabledString(CString strText)
{
	m_strFilterDisabled=strText;
}

void CCustomListCtrlHeader::CalcFontHeight()
{
	if (IsWindow(m_hWnd))
	{
		CDC* pDC=GetDC();
		int nOldDC=pDC->SaveDC();

		m_nFont1Height=pDC->GetTextExtent(" ").cy;
		m_nFont2Height=m_nFont1Height;

		if (m_FilterFont.m_hObject)
		{
			CFont* pOldFont=pDC->SelectObject(&m_FilterFont);

			m_nFont2Height=pDC->GetTextExtent(" ").cy;

			pDC->SelectObject(pOldFont);
		}

		pDC->RestoreDC(nOldDC);
		ReleaseDC(pDC);
	}
}


void CCustomListCtrlHeader::SetFilterFont(CFont *pFont)
{
	if (m_FilterFont.m_hObject)
		m_FilterFont.DeleteObject();

	if (pFont)
	{
		LOGFONT lf;

		pFont->GetLogFont(&lf);

		m_FilterFont.CreateFontIndirect(&lf);
	}

	CalcFontHeight();
}

BOOL CCustomListCtrlHeader::FilterEditing()
{
	return (m_pEdit ? TRUE : FALSE);
}

int CCustomListCtrlHeader::IndexToOrder(int nIndex)
{
	int nOrd=-1;

	int	nCount=GetItemCount();
	LPINT pnOrder=(LPINT)malloc(nCount*sizeof(int));

	GetOrderArray(pnOrder,nCount);

	for (int i=0;i<nCount && nOrd==-1;i++)
	{
		if (pnOrder[i]==nIndex)
			nOrd=i;
	}

	free(pnOrder);

	return nOrd;
}

BOOL CCustomListCtrlHeader::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// TODO: Add your specialized code here and/or call the base class
	
	return CHeaderCtrl::OnNotify(wParam, lParam, pResult);
}

void CCustomListCtrlHeader::SetFilterRect()
{
	if (m_pEdit!=NULL)
	{
		CRect rc;//rcHeader;

		GetItemRect(0,rc);

		rc.top += rc.Height()/2 + 2;
		rc.bottom++;

		rc.right--;
		m_pEdit->MoveWindow(rc);
	}
	if (m_pCombo!=NULL)
	{
		CRect rc;//rcHeader;

		GetItemRect(m_nComboColumn,rc);
//		GetWindowRect(rcHeader);

		rc.top+=m_nFont1Height+2;
		rc.bottom = rc.top + 65;

		rc.right--;
		m_pCombo->MoveWindow(rc);
	}	
}

void CCustomListCtrlHeader::SetComboColumns(int iColumnIndex, BOOL bSet /*= TRUE*/)
{
	// If the Column Index is not present && Set flag is false
	// Then do nothing 
	// If the Column Index is present && Set flag is true
	// Then do nothing
	POSITION Pos = m_ComboSupportColumnsList.Find(iColumnIndex);

	// If the Column Index is not present && Set flag is true
	// Then Add to list
	if ((NULL == Pos) && bSet) 
	{
		m_ComboSupportColumnsList.AddTail(iColumnIndex); 
	}

	// If the Column Index is present && Set flag is false
	// Then Remove from list
	if ((NULL != Pos) && !bSet) 
	{
		m_ComboSupportColumnsList.RemoveAt(Pos); 
	}
}

BOOL CCustomListCtrlHeader::IsCombo(int iColumnIndex)
{
	if (m_ComboSupportColumnsList.Find(iColumnIndex))
	{
		return true;
	}
	return false;
}

void CCustomListCtrlHeader::SetDisableStatus()
{

}

CellWindowType CCustomListCtrlHeader::GetColumnCtrlType( int nColumn )
{
	if( nColumn >= (int)m_gridColumnsList.size() || nColumn < 0 )
		return cellUnknown;
	return m_gridColumnsList[nColumn].ctrlType;
}

bool CCustomListCtrlHeader::GetComboItemsList( int nColumn, CStringList& clComboItemsList, bool AddAll /*= true*/ )
{
	clComboItemsList.RemoveAll();
	if( nColumn >= (int)m_gridColumnsList.size() || nColumn < 0 )
		return FALSE;

	CustomComboboxItemList::iterator it;
	for ( it = m_gridColumnsList[nColumn].dropdownValueList.begin(); 
		it != m_gridColumnsList[nColumn].dropdownValueList.end(); 
		it++ )
	{
		CustomComboboxItem* item = &(*it);
		clComboItemsList.AddTail((item->strText));
	}

	if ( AddAll )
	{
		clComboItemsList.AddHead(_M("ALL"));
	}
	return TRUE;
}

int CCustomListCtrlHeader::GetFilterStatus( int nIndex )
{
	CFilterInfo* cfi = GetFilter(nIndex);
	if (cfi->m_nStatus == FILTER_DISABLED)
	{
		return FILTER_DISABLED;
	}

	if ( nIndex >= 0 && nIndex < (int)m_gridColumnsList.size() )
	{
		if ( m_gridColumnsList[nIndex].ctrlType == cellUnknown )
		{
			return FILTER_DISABLED;
		}
	}
	return FILTER_ENABLED;
}

CString CCustomListCtrlHeader::GetFilterText( int nIndex )
{
	if(nIndex>=0)
		return GetFilter(nIndex)->m_strFilter;
	else
		return "";
}

void CCustomListCtrlHeader::SetFilterText( int nIndex, CString strText )
{
	if(nIndex>=0)
		((CFilterInfo*)m_arFilters[nIndex])->m_strFilter = strText;
}

void CCustomListCtrlHeader::ResetFilter()
{
	for (int i=0;i<m_arFilters.GetSize();i++)
	{
		((CFilterInfo*)m_arFilters[i])->m_strFilter = "";
	}
	CustomColumnList::iterator it;
	for (it=m_gridColumnsList.begin();it!=m_gridColumnsList.end();it++)
	{
		CustomColumn* currentColumn = &(*it);
		currentColumn->strHeaderFilter = "";
	}
	Invalidate();
}

LRESULT CCustomListCtrlHeader::OnLayout( WPARAM wParam, LPARAM lParam )
{
	LRESULT lResult = CHeaderCtrl::DefWindowProc(HDM_LAYOUT, 0, lParam); 
	HD_LAYOUT &hdl = *( HD_LAYOUT * ) lParam; 
	RECT *prc = hdl.prc; 
	WINDOWPOS *pwpos = hdl.pwpos; 
	
	int nHeight = (int)(pwpos->cy);
	if (m_Height >= 0)
	{
		nHeight = m_Height;
		pwpos->cy = nHeight; 
		prc->top = nHeight; 
	}
	
	return lResult; 
}

void CCustomListCtrlHeader::OnPaint()
{
	if ( m_Height == 0)
		return;

	CHeaderCtrl::OnPaint();

	int nItem = GetItemCount();

	//»­Í·²¿Ê£Óà²¿·Ö
	CRect rtRect;
	CRect clientRect;
	GetItemRect(nItem - 1,rtRect);
	GetClientRect(clientRect);
	rtRect.left = rtRect.right;
	rtRect.right = clientRect.right;


	CClientDC dc(this);
	CBrush _brush; 
	_brush.CreateSolidBrush(GetSysColor(CTLCOLOR_DLG));//´´½¨»­Ë¢ 
	dc.FillRect(&rtRect,&_brush); //Ìî³ä±³¾° 
	_brush.DeleteObject(); //ÊÍ·Å»­Ë¢ 


	if ( m_bEnableFilter )
	{
		RECT rcBody;
		GetClientRect( &rcBody );

		CClientDC dc(this);
		int nOldDC = dc.SaveDC();

		CPen myPen;
		myPen.CreatePen(PS_SOLID, 2, RGB(239,239,239));

		dc.SelectObject(&myPen);
		dc.MoveTo(rcBody.left, rcBody.top);
		dc.LineTo(rcBody.right, rcBody.top);
		dc.MoveTo(rcBody.left, rcBody.bottom - 1);
		dc.LineTo(rcBody.right, rcBody.bottom - 1);
		myPen.DeleteObject();

		dc.RestoreDC(nOldDC);
	}	
}

BOOL CCustomListCtrlHeader::OnEraseBkgnd( CDC* pDC )
{
	return TRUE;
}

void CCustomListCtrlHeader::EnableFilter( BOOL bEnabled /*= TRUE */ )
{
	m_bEnableFilter = bEnabled;
}

HBITMAP CCustomListCtrlHeader::CreateSortBitmap(BOOL bAscending)
{
	HWND hWnd = ::GetDesktopWindow();
	HDC hDC = ::GetDC(hWnd);

	// Aquire the Display DC
	CDC* pDC = CDC::FromHandle(hDC);
	//create a memory dc
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	//Create a memory bitmap
	CBitmap newbmp;
	CRect rcIcon(0, 0, 20, 20);
	newbmp.CreateCompatibleBitmap(pDC, rcIcon.Height(), rcIcon.Width());

	//select the bitmap in the memory dc
	CBitmap *pOldBitmap = memDC.SelectObject(&newbmp);

	//make the bitmap white to begin with
	memDC.FillSolidRect(rcIcon.top,rcIcon.left,rcIcon.bottom,rcIcon.right,COLOR_FILTER_BK);

	// Set up pens to use for drawing the triangle
	CPen penLight(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
	CPen penShadow(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
	CPen *pOldPen = memDC.SelectObject( &penLight );

	int iOffset = (rcIcon.bottom - rcIcon.top) / 5;

	if( bAscending )
	{
		// draw the arrow pointing upwards.
		memDC.MoveTo( rcIcon.right - 2 * iOffset, iOffset);
		memDC.LineTo( rcIcon.right - iOffset, rcIcon.bottom - iOffset - 1 );
		memDC.LineTo( rcIcon.right - 3 * iOffset - 2, rcIcon.bottom - iOffset - 1 );
		(void)memDC.SelectObject( &penShadow );
		memDC.MoveTo( rcIcon.right - 3 * iOffset - 1, rcIcon.bottom - iOffset - 1 );
		memDC.LineTo( rcIcon.right - 2 * iOffset, iOffset - 1);		
	}
	else
	{
		// draw the arrow pointing downwards.
		memDC.MoveTo( rcIcon.right - iOffset - 1, iOffset );
		memDC.LineTo( rcIcon.right - 2 * iOffset - 1, rcIcon.bottom - iOffset );
		(void)memDC.SelectObject( &penShadow );
		memDC.MoveTo( rcIcon.right - 2 * iOffset - 2, rcIcon.bottom - iOffset );
		memDC.LineTo( rcIcon.right - 3 * iOffset - 1, iOffset );
		memDC.LineTo( rcIcon.right - iOffset - 1, iOffset );		
	}

	// Restore the pen
	memDC.SelectObject(pOldPen);

	//select old bitmap back into the memory dc
	memDC.SelectObject(pOldBitmap);

	::ReleaseDC(hWnd, hDC);

	return (HBITMAP)newbmp.Detach();
}

HBITMAP CCustomListCtrlHeader::CreateTextBMP(CString strText)
{
	HWND hWnd = ::GetDesktopWindow();
	HDC hDC = ::GetDC(hWnd);

	// Aquire the Display DC
	CDC* pDC = CDC::FromHandle(hDC);

	//create a memory dc
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);

	//Create a memory bitmap
	CBitmap newbmp;
	CRect rcIcon(0, 0, 30, (int)(memDC.GetTextExtent(strText).cx*1.2));
	newbmp.CreateCompatibleBitmap(pDC, rcIcon.Height(), rcIcon.Width());

	//select the bitmap in the memory dc
	CBitmap *pOldBitmap = memDC.SelectObject(&newbmp);

	//make the bitmap white to begin with
	memDC.FillSolidRect(rcIcon.top,rcIcon.left,rcIcon.bottom,rcIcon.right,RGB(255,0,0));

	memDC.DrawText(strText, strText.GetLength(), rcIcon, DT_LEFT);

	//// Set up pens to use for drawing the triangle
	//CPen penLight(PS_SOLID, 1, GetSysColor(COLOR_3DHILIGHT));
	//CPen penShadow(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));
	//CPen *pOldPen = memDC.SelectObject( &penLight );

	//int iOffset = (rcIcon.bottom - rcIcon.top) / 4;

	//if( bAscending )
	//{
	//	// draw the arrow pointing upwards.
	//	memDC.MoveTo( rcIcon.right - 2 * iOffset, iOffset);
	//	memDC.LineTo( rcIcon.right - iOffset, rcIcon.bottom - iOffset - 1 );
	//	memDC.LineTo( rcIcon.right - 3 * iOffset - 2, rcIcon.bottom - iOffset - 1 );
	//	(void)memDC.SelectObject( &penShadow );
	//	memDC.MoveTo( rcIcon.right - 3 * iOffset - 1, rcIcon.bottom - iOffset - 1 );
	//	memDC.LineTo( rcIcon.right - 2 * iOffset, iOffset - 1);		
	//}
	//else
	//{
	//	// draw the arrow pointing downwards.
	//	memDC.MoveTo( rcIcon.right - iOffset - 1, iOffset );
	//	memDC.LineTo( rcIcon.right - 2 * iOffset - 1, rcIcon.bottom - iOffset );
	//	(void)memDC.SelectObject( &penShadow );
	//	memDC.MoveTo( rcIcon.right - 2 * iOffset - 2, rcIcon.bottom - iOffset );
	//	memDC.LineTo( rcIcon.right - 3 * iOffset - 1, iOffset );
	//	memDC.LineTo( rcIcon.right - iOffset - 1, iOffset );		
	//}

	//// Restore the pen
	//memDC.SelectObject(pOldPen);

	//select old bitmap back into the memory dc
	memDC.SelectObject(pOldBitmap);

	::ReleaseDC(hWnd, hDC);

	return (HBITMAP)newbmp.Detach();
}

void CCustomListCtrlHeader::DoDateTimeFilter( CellWindowType ctrlType )
{
	CString strStartDateTime;
	CString strEndDateTime;

	CString strOldFilter = GetFilter(m_nEditColumn)->m_strFilter;
	int nPos = strOldFilter.Find(_T(" ~ "));
	if (nPos != -1)
	{
		strStartDateTime = strOldFilter.Left(nPos);
		strEndDateTime = strOldFilter.Right(strOldFilter.GetLength()-nPos-3);
	}
	

	CDynDialogEx dlg(this);		//create the dynamic dialog, using this as parent window

	DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_TABSTOP | DTS_RIGHTALIGN;
	CString strTitle;
	if ( ctrlType == cellTextDate )
	{
		dwStyle |= DTS_SHORTDATEFORMAT;
		strTitle = _M("Set Date Range");
	}
	else if ( ctrlType == cellTextTime )
	{
		dwStyle |= DTS_TIMEFORMAT;
		strTitle = _M("Set Time Range");
	}
	else
		return;

	dlg.SetFont(GetFont());

	dlg.SetUseSystemButtons(FALSE);
	dlg.SetWindowTitle(strTitle);

	CRect rcFrom(6,10,30,20);
	dlg.AddDlgControl(_T("STATIC"), _M("From"), STYLE_STATIC, EXSTYLE_STATIC, rcFrom);

	COleDateTime oleDateTimeStart;
	GetDateTime(strStartDateTime, oleDateTimeStart);
	CRect rcStart(30,8,80,19);
	dlg.AddDlgControl(_T("SysDateTimePick32"), _T(""), dwStyle, 0, rcStart, (void*)&oleDateTimeStart);

	CRect rcTo(90,10,100,20);
	dlg.AddDlgControl(_T("STATIC"), _M("To"), STYLE_STATIC, EXSTYLE_STATIC, rcTo);

	COleDateTime oleDateTimeEnd;
	GetDateTime(strEndDateTime, oleDateTimeEnd);
	CRect rcEnd(110,8,160,19);
	dlg.AddDlgControl(_T("SysDateTimePick32"), _T(""), dwStyle, 0, rcEnd, (void*)&oleDateTimeEnd);

	CRect rcOk(6,30,50,42);
	dlg.AddDlgControl(BUTTON, _M("OK"), STYLE_BUTTON, EXSTYLE_BUTTON, &rcOk, NULL, IDOK);

	CRect rcClear(65,30,105,42);
	dlg.AddDlgControl(BUTTON, _M("Clear"), STYLE_BUTTON, EXSTYLE_BUTTON, &rcClear, NULL, ID_BUTTON_CLEAR);

	CRect rcCancel(120,30,160,42);
	dlg.AddDlgControl(BUTTON, _M("Cancel"), STYLE_BUTTON, EXSTYLE_BUTTON, &rcCancel, NULL, IDCANCEL);


	UINT nRet = dlg.DoModal();
	if ( nRet == IDCANCEL )
		return;

	CString strFilter;
	if ( !dlg.IsClear() )
	{
		if ( ctrlType == cellTextDate )
		{
			strFilter.Format("%04d-%02d-%02d ~ %04d-%02d-%02d", oleDateTimeStart.GetYear(), oleDateTimeStart.GetMonth(), oleDateTimeStart.GetDay(),
				oleDateTimeEnd.GetYear(), oleDateTimeEnd.GetMonth(), oleDateTimeEnd.GetDay());
		}
		else if ( ctrlType == cellTextTime )
		{
			strFilter.Format("%02d:%02d:%02d ~ %02d:%02d:%02d", oleDateTimeStart.GetHour(), oleDateTimeStart.GetMinute(), oleDateTimeStart.GetSecond(),
				oleDateTimeEnd.GetHour(), oleDateTimeEnd.GetMinute(), oleDateTimeEnd.GetSecond());
		}
	}	

	if ( strFilter == strOldFilter )
		return;

	GetFilter(m_nEditColumn)->m_strFilter=strFilter;

	Invalidate();

	UpdateWindow();

	NMCUSTOMHDR nmh;
	nmh.code=MSG_CUSTOM_LIST_FILTER_TEXT_CHANGED;
	nmh.hwndFrom=m_hWnd;
	nmh.idFrom=m_nEditColumn;
	nmh.szNewText=strFilter.GetBuffer(strFilter.GetLength()+1);

	if ( m_nEditColumn >= 0 && m_nEditColumn < (int)m_gridColumnsList.size() )
	{
		m_gridColumnsList[m_nEditColumn].strHeaderFilter = strFilter;
	}

	CWnd* pParent = GetParent();
	if (pParent)
		pParent->SendMessage(WM_NOTIFY,(WPARAM)m_hWnd,(LPARAM)&nmh);
}

BOOL CCustomListCtrlHeader::GetDateTime(LPCTSTR szDateTime, COleDateTime& tm)
{
	int y, m, d, H, M, S;

	y=2000;
	m=d=1;
	H=M=S=0;

	LPCTSTR deli = "-/";
	CString strTime = szDateTime;
	if ( strTime.IsEmpty() )
	{
		tm = COleDateTime::GetCurrentTime();
		return TRUE;
	}

	int i0, i1;

	strTime.TrimLeft();
	strTime.TrimRight();

	if((i0 = strTime.FindOneOf(deli)) >= 0)
	{
		y = atoi(strTime.Mid(0, i0));

		i1 = i0+1;
		strTime = strTime.GetBuffer(0) + i1;

		if((i0 = strTime.FindOneOf(deli)) >= 0)
		{
			m = atoi(strTime.Mid(0, i0));

			i1 = i0+1;
			strTime = strTime.GetBuffer(0) + i1;

			if((i0 = strTime.GetLength()) >= 0)
			{
				d = atoi(strTime.Mid(0, i0>1?2:i0));
			}
		}

		if(d > 31 && y < 13)
		{
			std::swap(y, m);
			std::swap(m, d);
		}

		y=y<1970?1970:y;
		y=y>3000?3000:y;
		m=m<1?1:m;
		m=m>12?12:m;
		d=d<1?1:d;
		d=d>31?31:d;
	}

	strTime = szDateTime;
	if((i0 = strTime.FindOneOf(" ")) >= 0)
	{
		i1 =i0+1;
		strTime = strTime.GetBuffer(0) + i1;
	}

	if((i0 = strTime.FindOneOf(":")) >= 0)
	{
		H = atoi(strTime.Mid(0, i0));

		i1 = i0+1;
		strTime = strTime.GetBuffer(0) + i1;

		if((i0 = strTime.FindOneOf(":")) >= 0)
		{
			M = atoi(strTime.Mid(0, i0));

			i1 = i0+1;
			strTime = strTime.GetBuffer(0) + i1;

			if((i0 = strTime.GetLength()) >= 0)
			{
				S = atoi(strTime.Mid(0, i0));

				i1 = i0+1;
				strTime = strTime.GetBuffer(0) + i1;
			}
		}
	}

	tm = COleDateTime(y, m, d, H, M, S);

	strTime = szDateTime;
	return strTime.FindOneOf("-/:")>0;
}

LRESULT CCustomListCtrlHeader::OnComboSelChanging( WPARAM, LPARAM )
{
	if (m_pCombo && m_bFilterWhenEditting)
	{
		CString strVal;
		CString strFilter;
		m_pCombo->GetWindowText(strVal);

		GetFilter(m_nEditColumn)->m_strFilter=strVal;
		NMCUSTOMHDR nmh;
		nmh.code=MSG_CUSTOM_LIST_FILTER_TEXT_CHANGED;
		nmh.hwndFrom=m_hWnd;
		nmh.idFrom=m_nEditColumn;
		nmh.szNewText=strVal.GetBuffer(strVal.GetLength()+1);

		if ( m_nEditColumn >= 0 && m_nEditColumn < (int)m_gridColumnsList.size() )
		{
			m_gridColumnsList[m_nEditColumn].strHeaderFilter = strVal;
		}

		if (GetParent())
			GetParent()->SendMessage(WM_NOTIFY,(WPARAM)m_hWnd,(LPARAM)&nmh);
	}

	return 0;
}

BOOL CCustomListCtrlHeader::PreTranslateMessage( MSG* pMsg )
{
	BOOL bRetVal=FALSE;

	if (pMsg->message==WM_KEYDOWN  )
	{

		if (m_pEdit == NULL && m_pCombo == NULL )
		{
			if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_DELETE || 
				pMsg->wParam == VK_F5 || pMsg->wParam == VK_BACK )
			{
				bRetVal=TRUE;
			}
		}

	}

	if (!bRetVal)
		bRetVal=CHeaderCtrl::PreTranslateMessage(pMsg);

	return bRetVal;
}
