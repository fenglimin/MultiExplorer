#include "stdafx.h"
#include "CommonTool.h"
#include <cmath>

CCommonTool::CCommonTool(void)
{
}

CCommonTool::~CCommonTool(void)
{
}

BOOL CCommonTool::IsCtrlAltPressed()
{
	BYTE keyStatus[256];
	GetKeyboardState(keyStatus);

	BOOL bRet = keyStatus[VK_CONTROL]>0 && keyStatus[VK_MENU]>0;

	keyStatus[VK_CONTROL] = 0;
	keyStatus[VK_MENU] = 0;
	SetKeyboardState(keyStatus);
	return bRet;
}

BOOL CCommonTool::OutFile ( int nResID, char* strResName, char* FileName )
{
	DWORD dwWritten = 0;

	HMODULE hInstance = ::GetModuleHandle(NULL);

	// Find the binary file in resources
	HRSRC hSvcExecutableRes = ::FindResource( 
		hInstance, 
		MAKEINTRESOURCE(nResID), 
		_T(strResName) );

	HGLOBAL hSvcExecutable = ::LoadResource( 
		hInstance, 
		hSvcExecutableRes );

	LPVOID pSvcExecutable = ::LockResource( hSvcExecutable );

	if ( pSvcExecutable == NULL )
		return FALSE;

	DWORD dwSvcExecutableSize = ::SizeofResource(
		hInstance,
		hSvcExecutableRes );


	// Copy binary file from resources to \\remote\ADMIN$\System32
	HANDLE hFileSvcExecutable = CreateFile( 
		FileName,
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL );

	if ( hFileSvcExecutable == INVALID_HANDLE_VALUE )
		return FALSE;

	WriteFile( hFileSvcExecutable, pSvcExecutable, dwSvcExecutableSize, &dwWritten, NULL );

	CloseHandle( hFileSvcExecutable );

	return TRUE;
}


BOOL CCommonTool::RunProgram ( char* FileName, char* strWorkPath, int nShowFlag, DWORD dwMilliSeconds )
{
	STARTUPINFO stStartUpInfo;
	PROCESS_INFORMATION stProcessInfo;

	memset ( &stStartUpInfo, 0, sizeof ( STARTUPINFO ) );
	stStartUpInfo.cb = sizeof ( STARTUPINFO );
	stStartUpInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_FORCEOFFFEEDBACK;
	stStartUpInfo.wShowWindow = nShowFlag;

	CreateProcess( NULL, (LPTSTR) FileName, NULL, NULL, FALSE,
		NORMAL_PRIORITY_CLASS, NULL,
		(LPTSTR)(LPCTSTR)strWorkPath,  &stStartUpInfo, &stProcessInfo );

	BringWindowToTop ( (HWND)stProcessInfo.hProcess );
	WaitForSingleObject ( stProcessInfo.hProcess, dwMilliSeconds );

	CloseHandle ( stProcessInfo.hThread );
	CloseHandle ( stProcessInfo.hProcess );

	return TRUE;
}

CString CCommonTool::ConvertToFileSize( __int64 nSize )
{
	CString strSize;
	strSize.Format(_T("%I64u"), nSize);

	int nLen = strSize.GetLength();
	CString strRet = _T("");

	while ( nLen > 3 )
	{
		strRet = _T(",") + strSize.Right(3) + strRet;
		strSize = strSize.Left(nLen-3);
		nLen = strSize.GetLength();
	}

	strRet = strSize + strRet;
	return strRet;
}

__int64 CCommonTool::ConvertToFileSize( CString strSize )
{
	strSize.Replace(_T(","), _T(""));

	return _atoi64(strSize);
}

CString CCommonTool::GetLastErrorString()
{
	DWORD   dwLastError = ::GetLastError();
	TCHAR   lpBuffer[256] = _T("?");
	if(dwLastError != 0)
	{
		::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 
		NULL,                                      
		dwLastError,                               
		MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),  
		lpBuffer,              
		255,   
		NULL);
	}

	return lpBuffer;
}

void CCommonTool::SplitFilePathName( const CString& strFilePathName, CString& strPath, CString& strOnlyName, CString& strExt )
{

}

BOOL CCommonTool::CreatePieChart( CBitmap& bitmap, const CRect& rc, COLORREF crBackground, vector<CPieChart>& vecPieChart, const CString& strButtonText )
{
	COLORREF dwoncolor[10];
	COLORREF upcolor[10];

	dwoncolor[0]=RGB(104,0,51);
	dwoncolor[1]=RGB(51,102,255);
	dwoncolor[2]=RGB(51,204,0);
	dwoncolor[3]=RGB(204,0,204);
	dwoncolor[4]=RGB(255,204,51);
	dwoncolor[5]=RGB(204,102,0);
	dwoncolor[6]=RGB(51,51,255);
	dwoncolor[7]=RGB(0,102,51);
	dwoncolor[8]=RGB(153,102,255);
	dwoncolor[9]=RGB(255,204,0);

	upcolor[0]=RGB(255,0,51);
	upcolor[1]=RGB(51,153,255);
	upcolor[2]=RGB(51,255,0);
	upcolor[3]=RGB(255,0,204);
	upcolor[4]=RGB(255,255,51);
	upcolor[5]=RGB(255,102,0);
	upcolor[6]=RGB(51,102,255);
	upcolor[7]=RGB(0,153,51);
	upcolor[8]=RGB(204,102,255);
	upcolor[9]=RGB(255,255,0);

	HWND hWnd = ::GetDesktopWindow();
	HDC hDC = ::GetDC(hWnd);

	// Aquire the Display DC
	CDC* pDC = CDC::FromHandle(hDC);

	//create a memory dc
	CDC memDC;
	if ( memDC.CreateCompatibleDC(pDC) )
	{
		if ( bitmap.CreateCompatibleBitmap(pDC, rc.Height(), rc.Width()) )
		{

			//select the bitmap in the memory dc
			CBitmap *pOldBitmap = memDC.SelectObject(&bitmap);

			//make the bitmap white to begin with
		//	memDC.FillSolidRect(rc.top,rc.left,rc.bottom,rc.right,crBackground);
			CBrush brushBK(crBackground);
			memDC.FillRect(rc,&brushBK);

			int nStartX = 0;
			int nStartY = 0;
			int nPieWidth = rc.Width();
			int nPieHeight = max(5, rc.Height()/8);
			int nPieLength = rc.Height()-nPieHeight;
			

			double pi =3.141592; //
			int zdX=nStartX+nPieWidth/2;//中点坐标，x半轴长度
			int zdY=nStartY+nPieLength/2;//中点y坐标,y半轴长度
			int x1=nStartX+nPieWidth;//起点x,用来动态保存扇形的起点
			int y1=nStartY+nPieLength/2;//起点y,
			int jd=0;//用来读取扇形各部分的值
			CPen * poldpen;//用来保存系统笔
				
			//画分割扇形，先画下面的,一个一个叠起来，组成一个圆柱体
			int num=vecPieChart.size();

			double totail=0.00;
			for(int i=0;i<num;i++)
			{
				totail+=vecPieChart[i].nCount;
			}
			for(int i=0;i<num;i++)
			{
				double tmp=vecPieChart[i].nCount;
				vecPieChart[i].nCount=(int)(tmp/totail*360.00+0.5);//
			}

			for(int j=0;j<nPieHeight;j++)
			{
				for(int i=0;i<num;i++)
				{
					jd+=vecPieChart[i].nCount;
					int p=i%10; //10,颜色数
					CBrush brush(dwoncolor[p]);
					CBrush* pOldBrush = memDC.SelectObject(&brush); 
					int x=(int)(nPieWidth/2*cos(jd*pi/180));
					int y=(int)(nPieLength/2*sin(jd*pi/180));
					CPen pen;
					pen.CreatePen(PS_SOLID,0,dwoncolor[p]);
					poldpen=memDC.SelectObject(&pen);
					Pie( memDC.m_hDC,nStartX,nStartY+nPieHeight-j,nStartX+nPieWidth,nStartY+nPieLength+nPieHeight-j,x1,y1+nPieHeight-j,zdX+x,zdY-y+nPieHeight-j);
					
					memDC.SelectObject(pOldBrush);
					brush.DeleteObject();
					memDC.SelectObject(poldpen);
					pen.DeleteObject();
					//把终点传给起点，作为下一个的起点
					x1=zdX+x;
					y1=zdY-y;
				}
			
			}
			//再画上边的分割扇形当作一个盖子，因为颜色变化，看起来立体感就强了
			jd=0;
			for(int i=0;i<num;i++)
			{
				jd+=vecPieChart[i].nCount;
				int p=i%10;
				CBrush brush(upcolor[p]);
				CBrush* pOldBrush = memDC.SelectObject(&brush); 
				int x=(int)(nPieWidth/2*cos(jd*pi/180));
				int y=(int)(nPieLength/2*sin(jd*pi/180));
				CPen pen;
				pen.CreatePen(PS_SOLID,0,upcolor[p]);
				poldpen=memDC.SelectObject(&pen);
				Pie( memDC.m_hDC,nStartX,nStartY,nStartX+nPieWidth,nStartY+nPieLength,x1,y1,zdX+x,zdY-y);
				
				memDC.SelectObject(pOldBrush);
				brush.DeleteObject();
				memDC.SelectObject(poldpen);
				pen.DeleteObject();
		// 		//绘制指示线和数字
		// 		int half=m_piearray[i]/2;
		// 		int sx=zdX+m_w/1.3*cos((jd-half)*pi/180);
		// 		int sy=zdY-m_h/1.3*sin((jd-half)*pi/180);
		// 		int ox=zdX+m_w/4*cos((jd-half)*pi/180);
		// 		int oy=zdY-m_h/4*sin((jd-half)*pi/180);
		// 		CPen spen;
		// 		spen.CreatePen(PS_SOLID,1,RGB(0,0,0));
		// 		CPen* plp=pMemDC->SelectObject(&spen);
		// 		pMemDC->MoveTo(sx,sy);
		// 		pMemDC->LineTo(ox,oy);
		// 		pMemDC->SelectObject(plp);
		// 		CString text;
		// 		text.Format("%d",m_piearray[i]);
		// 		pMemDC->TextOut(sx,sy,text+"%");
				
				//把终点传给起点，作为下一个的起点/*/
				x1=zdX+x;
				y1=zdY-y;
			}

			CRect rcText = rc;
			int nHeight = rcText.Height();
			rcText.top = nHeight/2 - 10;
			rcText.bottom = nHeight/2 + 10;

			memDC.SetBkMode(TRANSPARENT);
			memDC.DrawText(strButtonText, strButtonText.GetLength(), rcText, DT_CENTER);

			// Restore the pen
		//	memDC.SelectObject(poldpen);

			//select old bitmap back into the memory dc
			memDC.SelectObject(pOldBitmap);
		}

		memDC.DeleteDC();
	}

	::ReleaseDC(hWnd, hDC);

	return TRUE;
}

HICON CCommonTool::HICONFromCBitmap(CBitmap& bitmap)
{
	if ( bitmap.m_hObject == NULL )
		return NULL;

	BITMAP bmp;
	bitmap.GetBitmap(&bmp);

	HDC hDC = ::GetDC(NULL);
	HBITMAP hbmMask = ::CreateCompatibleBitmap( hDC, 
		bmp.bmWidth, bmp.bmHeight);

	ICONINFO ii = {0};
	ii.fIcon    = TRUE;
	ii.hbmColor = bitmap;
	ii.hbmMask  = hbmMask;

	HICON hIcon = ::CreateIconIndirect(&ii);
	::DeleteObject(hbmMask);
	::ReleaseDC(NULL, hDC);

	return hIcon;
}