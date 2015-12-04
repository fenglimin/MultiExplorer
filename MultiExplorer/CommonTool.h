#pragma once

#include <map>
#include <vector>
#include <list>
#include <algorithm>
using namespace std;

class CPieChart
{
public:
	CString strName;
	int		nCount;

	CPieChart(CString strName, int nCount)
	{
		this->strName = strName;
		this->nCount = nCount;
	}
};

class CCommonTool
{
public:
	CCommonTool(void);
	virtual ~CCommonTool(void);

public:
	BOOL	IsCtrlAltPressed();
	BOOL	RunProgram ( char* FileName, char* strWorkPath, int nShowFlag, DWORD dwMilliSeconds );
	BOOL	OutFile ( int nResID, char* strResName, char* FileName );

	CString ConvertToFileSize(__int64 nSize);
	__int64 ConvertToFileSize(CString strSize);

	void	SplitFilePathName(const CString& strFilePathName, CString& strPath, CString& strOnlyName, CString& strExt );

	CString	GetLastErrorString();

	BOOL CreatePieChart( CBitmap& bitmap, const CRect& rc, COLORREF crBackground, vector<CPieChart>& vecPieChart, const CString& strButtonText );
	HICON HICONFromCBitmap(CBitmap& bitmap);
};
