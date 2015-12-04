#include "stdafx.h"
#include "LayoutManager.h"
#include "SelectLayoutDialog.h"

CLayoutManager::CLayoutManager()
{
	vector<CString> vecLayout;

	// 1 cell
	vecLayout.clear();
	vecLayout.push_back(_T("1X1;0"));
	m_vecLayoutString.push_back(vecLayout);

	// 2 cells
	vecLayout.clear();
	vecLayout.push_back(_T("1X2;0;1"));
	vecLayout.push_back(_T("2X1;0;1"));
	m_vecLayoutString.push_back(vecLayout);

	// 3 cells
	vecLayout.clear();
	vecLayout.push_back(_T("1X3;0;1;2"));
	vecLayout.push_back(_T("3X1;0;1;2"));
	vecLayout.push_back(_T("2X2;0,1;2;3"));
	vecLayout.push_back(_T("2X2;0;1,3;2"));
	vecLayout.push_back(_T("2X2;0;1;2,3"));
	vecLayout.push_back(_T("2X2;0,2;1;3"));
	m_vecLayoutString.push_back(vecLayout);

	// 4 cells
	vecLayout.clear();
	vecLayout.push_back(_T("1X4;0;1;2;3"));
	vecLayout.push_back(_T("2X2;0;1;2;3"));
	vecLayout.push_back(_T("2X3;0;1;2;3,4,5"));
	vecLayout.push_back(_T("2X3;0,3;1,4;2;5"));
	vecLayout.push_back(_T("3X2;0;1,3,5;2;4"));
	vecLayout.push_back(_T("3X2;0;1;2,3;4,5"));
	m_vecLayoutString.push_back(vecLayout);

	// 5 cells
	vecLayout.clear();
	vecLayout.push_back(_T("2X3;0;1;2,5;3;4"));
	vecLayout.push_back(_T("2X4;0;1;2,3;4,5;6,7"));
	vecLayout.push_back(_T("2X4;0;1;2,6;3,7;4,5"));
	vecLayout.push_back(_T("2X4;0;1;2,3;4;5,6,7"));
	vecLayout.push_back(_T("2X4;0;1,5;2,6;3,7;4"));
	vecLayout.push_back(_T("2X6;0,1;2,3;4,5;6,7,8;9,10,11"));
	m_vecLayoutString.push_back(vecLayout);

	// 6 cells
	vecLayout.clear();
	vecLayout.push_back(_T("2X3;0;1;2;3;4;5"));
	vecLayout.push_back(_T("2X4;0;1;2;3;4,5;6,7"));
	vecLayout.push_back(_T("2X4;0;1;2,6;3,7;4;5"));
	vecLayout.push_back(_T("2X4;0;1;2;3,7;4;5,6"));
	m_vecLayoutString.push_back(vecLayout);

	// 7 cells
	vecLayout.clear();
	vecLayout.push_back(_T("2X4;0;1;2;3,7;4;5;6"));
	vecLayout.push_back(_T("2X4;0;1;2;3;4;5;6,7"));
	m_vecLayoutString.push_back(vecLayout);

	// 8 cells
	vecLayout.clear();
	vecLayout.push_back(_T("2X4;0;1;2;3;4;5;6;7"));
	m_vecLayoutString.push_back(vecLayout);

}

CLayoutManager::~CLayoutManager(void)
{
}

BOOL CLayoutManager::TryMergeRect( const CRect& rc1, const CRect& rc2, CRect& rcMerge )
{
	if ( rc1.top == rc2.top && rc1.bottom == rc2.bottom )
	{
		if ( rc1.right == rc2.left || rc1.left == rc2.right )
		{
			rcMerge.top = rc1.top;
			rcMerge.bottom = rc1.bottom;

			rcMerge.left = min(rc1.left,rc2.left);
			rcMerge.right = max(rc1.right, rc2.right);

			return TRUE;
		}
	}
	else if ( rc1.left == rc2.left && rc1.right == rc2.right )
	{
		if ( rc1.bottom == rc2.top || rc1.top == rc2.bottom )
		{
			rcMerge.left = rc1.left;
			rcMerge.right = rc1.right;
		
			rcMerge.top = min(rc1.top, rc2.top);
			rcMerge.bottom = max(rc1.bottom, rc2.bottom);

			return TRUE;
		}
	}

	return FALSE;
}

int CLayoutManager::SplitString(const CString& strInput, const CString& strDelimiter, vector<CString>& vecResults)
{
	vecResults.clear();

	int nDelLength = strDelimiter.GetLength();
	int nPos ;

	CString strResult;
	CString strTemp = strInput;
	while((nPos=strTemp.Find(strDelimiter))>=0)
	{
		strResult = strTemp.Mid(0,nPos);
		if (strResult != _T(""))
		{
			vecResults.push_back(strResult);
		}
		
		strTemp = strTemp.Mid(nPos+nDelLength);
	}

	if (strTemp != _T(""))
	{
		vecResults.push_back(strTemp);
	}

	return vecResults.size();
}

int CLayoutManager::SplitString( const CString& strInput, const CString& strDelimiter, vector<int>& vecResults )
{
	vector<CString> vecTemp;
	int nSize = SplitString(strInput, strDelimiter, vecTemp);

	vecResults.clear();
	for ( int i = 0; i < nSize; i ++ )
	{
		vecResults.push_back(atoi(vecTemp[i]));
	}

	return nSize;
}

BOOL CLayoutManager::CreateRectListFromString( const CRect& rcClient, const CString& strLayout, vector<CRect>& vecLayout )
{
	vector<CString> vecGroup;
	int nSize = SplitString(strLayout, _T(";"), vecGroup);
	ASSERT(nSize>=2);

	CString strMatrix = vecGroup[0];
	vector<CRect> vecMatrix;
	if ( !GetMatrixCell(rcClient, strMatrix, vecMatrix ) )
	{
		return FALSE;
	}

	CString strGroup;
	vector<int> vecCellIndex;
	CRect rcGroup;

	vecLayout.clear();
	for ( int i = 1; i < nSize; i ++ )
	{
		strGroup = vecGroup[i];
		SplitString(strGroup, _T(","), vecCellIndex);

		if ( !MergeCells(vecCellIndex, vecMatrix, rcGroup) )
			return FALSE;

		vecLayout.push_back(rcGroup);

	}

	return TRUE;
}

BOOL CLayoutManager::MergeCells( const vector<int>& vecCellIndex, const vector<CRect>& vecCellRect, CRect& rcMerge )
{
	set<CRect> setCell;

	int nSize = vecCellIndex.size();
	for ( int i = 0; i < nSize; i ++ )
	{
		setCell.insert(vecCellRect[vecCellIndex[i]]);
	}

	RecursiveMergeCells(setCell);

	rcMerge = *setCell.begin();
	return TRUE;
}

BOOL CLayoutManager::GetMatrixCell( const CRect& rcClient, const CString& strMatrix, vector<CRect>& vecCells )
{
	vector<int> vecSize;
	SplitString(strMatrix, _T("X"), vecSize);
	if ( vecSize.size() != 2 )
	{
		CString strError;
		strError.Format(_T("The Matrix string ( %s )  is wrong!"), strMatrix);
		AfxMessageBox(strError);
		return FALSE;
	}

	int nCellWidth = rcClient.Width()/vecSize[1];
	int nCellHeight = rcClient.Height()/vecSize[0];

	for ( int i = 0; i < vecSize[0]; i ++ )
	{
		CRect rc;
		rc.top = rcClient.top+3 + i*nCellHeight;
		rc.bottom = min(rc.top + nCellHeight, rcClient.bottom-2);

		for ( int j = 0; j < vecSize[1]; j ++ )
		{
			rc.left = rcClient.left+2 + j*nCellWidth;
			rc.right = min(rc.left + nCellWidth, rcClient.right-2);

			vecCells.push_back(rc);
		}
	}

	return TRUE;
}

BOOL CLayoutManager::RecursiveMergeCells( set<CRect>& setCell )
{
	if ( setCell.size() == 1 )
	{
		return TRUE;
	}

	set<CRect>::iterator itMerge;
	set<CRect>::iterator itBeMerge;
	CRect rcMerge;
	BOOL bMerged = FALSE;
	for ( itMerge = setCell.begin(); itMerge != setCell.end(); itMerge++ )
	{
		for ( itBeMerge = setCell.begin(); itBeMerge != setCell.end(); itBeMerge++ )
		{
			if ( itBeMerge == itMerge )
				continue;

			if ( TryMergeRect(*itMerge, *itBeMerge, rcMerge) )
			{
				bMerged = TRUE;
				setCell.erase(itMerge);
				setCell.erase(itBeMerge);
				setCell.insert(rcMerge);

				break;;
			}
		}

		if ( bMerged )
			break;
	}

	if ( bMerged )
		return RecursiveMergeCells(setCell);
	else
		return FALSE;
}

HBITMAP CLayoutManager::GetBitmapFromLayoutString( const CRect& rcClient, const CString& strLayout )
{
	vector<CRect> vecLayout;
	if ( !CreateRectListFromString(rcClient, strLayout, vecLayout) )
		return NULL;


	CDC dc;
	CBitmap bmp;
	CClientDC ClientDC(AfxGetMainWnd());
	dc.CreateCompatibleDC(&ClientDC);
	bmp.CreateCompatibleBitmap(&ClientDC, rcClient.Width(), rcClient.Height());
	CBitmap* pOldBmp = (CBitmap*)dc.SelectObject(&bmp);


	CPen pen(PS_SOLID, 1, RGB(0,0,0));
	CPen penShadow(PS_SOLID, 2, RGB(20,20,20));

	CPen *pOldPen = dc.SelectObject(&pen);
	
	int nCount = vecLayout.size();
	CString strID;
	CRect rcText;
	int nTemp;
	for ( int i = 0; i < nCount; i ++ )
	{
		dc.Rectangle(&vecLayout[i]);

		strID.Format(_T("%d"),i+1);
		rcText = vecLayout[i];
		nTemp = (rcText.Height()-16)/2;
		rcText.top += nTemp;
		rcText.bottom -= nTemp;
		dc.DrawText(strID, rcText, DT_CENTER);
	}

	//draw 3D shadow
	dc.SelectObject(&penShadow);
	CRect rcBorder = rcClient;
	rcBorder.DeflateRect(1,1,1,1);
		
	dc.MoveTo(rcBorder.right, rcBorder.top);
	dc.LineTo(rcBorder.right, rcBorder.bottom);
	dc.LineTo(rcBorder.left, rcBorder.bottom);


	dc.SelectObject(pOldPen);
	dc.SelectObject(pOldBmp);
	dc.DeleteDC();

	HBITMAP hBitmap = (HBITMAP)::CopyImage((HANDLE)((HBITMAP)bmp), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);

	return hBitmap;
}

CString CLayoutManager::SelectLayoutString( int nWndCount, BOOL bMustSelect )
{
	CSelectLayoutDialog sld;

	sld.m_nCellWidth = 100;
	sld.m_nCellHeight = 70;

	CRect rcCell(0,0,sld.m_nCellWidth, sld.m_nCellHeight);

	nWndCount--;
	int nCount = m_vecLayoutString[nWndCount].size();

	for ( int i = 0; i < nCount; i ++ )
	{
		HBITMAP hBitmap = GetBitmapFromLayoutString(rcCell, m_vecLayoutString[nWndCount][i]);
		sld.m_vecBitmap.push_back(hBitmap);
	}

	sld.m_bMustSelect = bMustSelect;
	if ( sld.DoModal() != IDOK )
		return _T("");

	return m_vecLayoutString[nWndCount][sld.m_nIndex];
}
