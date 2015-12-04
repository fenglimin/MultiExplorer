#pragma once

#include <vector>
#include <map>
#include <set>

using namespace std;

class CLayoutManager
{
public:
	CLayoutManager();
	~CLayoutManager(void);

public:
	BOOL TryMergeRect(const CRect& rc1, const CRect& rc2, CRect& rcMerge);
	int SplitString(const CString& strInput, const CString& strDelimiter, vector<CString>& vecResults);
	int SplitString(const CString& strInput, const CString& strDelimiter, vector<int>& vecResults);

	BOOL CreateRectListFromString(const CRect& rcClient, const CString& strLayout, vector<CRect>& vecLayout);
	BOOL MergeCells(const vector<int>& vecCellIndex, const vector<CRect>& vecCellRect, CRect& rcMerge);
	BOOL RecursiveMergeCells(set<CRect>& setCell);

	BOOL GetMatrixCell(const CRect& rcClient, const CString& strMatrix, vector<CRect>& vecCells);

	HBITMAP GetBitmapFromLayoutString(const CRect& rcClient, const CString& strLayout);

	CString SelectLayoutString(int nWndCount, BOOL bMustSelect);
	vector<vector<CString>> m_vecLayoutString;
	//	BOOL CanMerge(const CRect& rc1, const CRect& rc2, BOOL bLeftRight);
};
