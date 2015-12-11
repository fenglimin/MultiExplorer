#include "StdAfx.h"
#include "DiskFileManager.h"
#include "resource.h"
#include "FileExplorer.h"
#include "UserOptionDialog.h"
#include "AddToFavoriteDialog.h"

static UINT NetWorkerThreadFunc(LPVOID pParam)
{
	CDiskFileManager* pDiskFileManager = (CDiskFileManager*)pParam;

	if (!pDiskFileManager->m_pWorkTool->StartWorking(pDiskFileManager->m_userOption.nLocalListeningPort))
	{
		CString strShow;
		strShow.Format(_M("Can NOT start net listener with port %d, please check!"), pDiskFileManager->m_userOption.nLocalListeningPort);
		AfxMessageBox(strShow);
	}
	return 0;
}

static UINT UpdateIconThreadFunc(LPVOID pParam)
{
	CUpdateIconThreadParam* pUpdateIconThreadPara = (CUpdateIconThreadParam*)pParam;

	CCustomListCtrl* pListCtrl = (CCustomListCtrl*)&pUpdateIconThreadPara->pFileExplorer->m_listFile;
	pListCtrl->EnableSort(FALSE);
	pUpdateIconThreadPara->pDiskFileManager->DoUpdateIcon(pUpdateIconThreadPara->pFileExplorer);
	pUpdateIconThreadPara->pFileExplorer->EnableAllCtrls();
	pListCtrl->EnableSort(TRUE);
	pListCtrl->SortPrevious();

	delete pUpdateIconThreadPara;
	return 0;
}



static UINT RefreshDiskDriverThreadFunc(LPVOID pParam)
{
	CDiskFileManager* pDiskFileManager = (CDiskFileManager*)pParam;

	DWORD dwStart = GetTickCount();
	pDiskFileManager->GetDiskDriverList(FALSE);
	pDiskFileManager->SetTitleWithNetworkInfo();
	DWORD dwEnd = GetTickCount();

	CString strText;
	strText.Format("GetDiskDriverList start %d, end %d, period %d", dwStart, dwEnd, dwEnd-dwStart);
//	pDiskFileManager->m_pUserWnd->SetWindowText(strText);

	return 0;
}

CDiskFileManager::CDiskFileManager(void)
{
	m_pOperationDlg = NULL;
	m_pActiveFileExplorer = NULL;
	m_hRefreshDiskDriveThreadHandle = 0;
	m_strDefaultIniFile = _T("");
	m_pWorkTool = new CWorkTool(NULL);
}

CDiskFileManager::~CDiskFileManager(void)
{
	delete	m_pWorkTool;
}

vector<CDiskDriverInfo> CDiskFileManager::GetDiskDriverList(BOOL bNotGetCapacityAndLabel)
{
	DWORD  dwNumBytesForDriveStrings;
	HANDLE hHeap;
	LPTSTR pszAllDrives, pszDrive;

	// Get the number of bytes needed to hold all
	// the logical drive strings.
	dwNumBytesForDriveStrings =
		GetLogicalDriveStrings(0, NULL) * sizeof(TCHAR);

	// The GetLogicalDriveStrings function is 
	// supported on this platform.

	// Allocate memory from the heap for the drive
	// string names.
	hHeap = GetProcessHeap();
	pszAllDrives = (LPTSTR) HeapAlloc(hHeap, 
		HEAP_ZERO_MEMORY, dwNumBytesForDriveStrings);

	// Get the drive string names in our buffer.
	GetLogicalDriveStrings(
		dwNumBytesForDriveStrings / sizeof(TCHAR), 
		pszAllDrives);

	// Parse the memory block, and fill the combo box.
	pszDrive = pszAllDrives;
	m_vecDiskDriverInfo.clear();
	while (pszDrive[0] != 0)
	{
		CDiskDriverInfo di;
		GetDriverInfo(pszDrive, di, bNotGetCapacityAndLabel);

		if ( di.nDriverType != DRIVE_CDROM )
			m_vecDiskDriverInfo.push_back(di);

		pszDrive = strchr(pszDrive, 0) + 1;// Go to next string.
	}

	HeapFree(hHeap, 0, pszAllDrives);

	return m_vecDiskDriverInfo;
}

BOOL CDiskFileManager::GetDirFileListRecursive( const CString& strDir, BOOL bRecursive, const CString& strRecursiveDir, vector<CDirectoryInfo>& vecAllDirs, vector<CFileInfo>& vecAllFiles, BOOL bLoadHidden, const CString& strFilterName, const CString& strFilterExt )
{

	vector<CDirectoryInfo> vecDir;
	vector<CFileInfo> vecFile;
	GetDirFileList(strDir+strRecursiveDir, vecDir, vecFile, bLoadHidden);

	CString strDirFilter = strFilterName + strFilterExt;
	CString strDirName;
	for ( int i = 0; i < (int)vecDir.size(); i ++ )
	{
		strDirName = vecDir[i].strName;
		if ( bRecursive )
		{
			GetDirFileListRecursive(strDir, TRUE, strRecursiveDir+strDirName+_T("\\"), vecAllDirs, vecAllFiles, bLoadHidden, strFilterName, strFilterExt);
		}

		strDirName.MakeLower();
		if ( strDirName.Find(strDirFilter) == -1 )
			continue;

		vecDir[i].strName = strRecursiveDir+vecDir[i].strName;
		vecAllDirs.push_back(vecDir[i]);
	}

	CString strName, strFileName, strFileExt;
	int nToken;
	for ( int i = 0; i < (int)vecFile.size(); i ++ )
	{
		strName = vecFile[i].strName;
		strName.MakeLower();
		nToken = strName.ReverseFind(_T('.'));
		strFileName = strName.Left(nToken);
		strFileExt = strName.Right(strName.GetLength()-nToken);

		if ( !strFilterName.IsEmpty() )
		{
			if ( strFileName.Find(strFilterName) == -1 )
				continue;
		}

		if ( !strFilterExt.IsEmpty() )
		{
			if ( strFileExt.Find(strFilterExt) == -1 )
				continue;
		}

		vecFile[i].strName = strRecursiveDir+vecFile[i].strName;
		vecAllFiles.push_back(vecFile[i]);
	}

	return TRUE;
}

BOOL CDiskFileManager::GetDirFileList( const CString& strDir, vector<CDirectoryInfo>& vecSubDir, vector<CFileInfo>& vecFile, BOOL bLoadHidden )
{
	char	strDescDir[MAX_PATH*10];

	strcpy_s ( strDescDir, strDir );
	strcat_s ( strDescDir, "*.*" );


	WIN32_FIND_DATA FindData; // File information

	HANDLE hFind = FindFirstFile ( strDescDir, &FindData);
	BOOL bOk = ( hFind != INVALID_HANDLE_VALUE );

	if ( !bOk )
		return FALSE;

	CString strFile, strTime;
	while ( bOk )
	{
		if ( !bLoadHidden && ( (FindData.dwFileAttributes&FILE_ATTRIBUTE_HIDDEN) || (FindData.dwFileAttributes&FILE_ATTRIBUTE_SYSTEM) ) )
		{
			bOk = FindNextFile ( hFind, &FindData );
			continue;
		}

		strFile = FindData.cFileName;

		COleDateTime dt(FindData.ftLastWriteTime);
		strTime.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), dt.GetYear(), dt.GetMonth(), dt.GetDay(),
			dt.GetHour(), dt.GetMinute(), dt.GetSecond());

		BOOL bIsDir = FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;

		if ( bIsDir )
		{
			if ( strFile != _T(".") && strFile != _T("..") )
			{
				CDirectoryInfo di;
				di.strName = strFile;
				di.strDate = strTime;
		
				vecSubDir.push_back(di);
			}
		}
		else
		{
			CFileInfo fi;
			fi.strName = strFile;

			int nToken1 = fi.strName.ReverseFind(_T('.'));
			int nToken2 = fi.strName.ReverseFind(_T('\\'));
			if ( nToken1 > nToken2 )
			{
				fi.strExt = fi.strName.Right(fi.strName.GetLength()-nToken1-1);
			}
			else
			{
				fi.strExt = _T("");
			}
			fi.strDate = strTime;

			fi.nSize = ((__int64)FindData.nFileSizeHigh << 32) + FindData.nFileSizeLow;
			vecFile.push_back(fi);
		}

		bOk = FindNextFile ( hFind, &FindData );
	}
	FindClose ( hFind );

	return TRUE;
}

BOOL CDiskFileManager::GetDirFileList( const CString& strDir, vector<CString>& vecSubDir, vector<CString>& vecFile )
{
	char	strDescDir[MAX_PATH];

	strcpy_s ( strDescDir, strDir );
	strcat_s ( strDescDir, "*.*" );


	WIN32_FIND_DATA FindData; // File information

	HANDLE hFind = FindFirstFile ( strDescDir, &FindData);
	BOOL bOk = ( hFind != INVALID_HANDLE_VALUE );

	if ( !bOk )
		return FALSE;

	CString strName;
	while ( bOk )
	{
		strName = FindData.cFileName;
		BOOL bIsDir = FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
		if ( bIsDir )
		{
			if ( strName != _T(".") && strName != _T("..") )
			{
				vecSubDir.push_back(strName);
			}
		}
		else
		{
			vecFile.push_back(strName);
		}

		bOk = FindNextFile ( hFind, &FindData );
	}
	FindClose ( hFind );

	return TRUE;
}

BOOL CDiskFileManager::OnCellTextChanged( CListCtrl* pListCtrl, int nRow, int nCol )
{
	return TRUE;
}

BOOL CDiskFileManager::OnCellTextChanging( CListCtrl* pListCtrl, int nRow, int nCol, CellFormat* pCellFormat, const CString& strOldValue, CString& strNewValue )
{
	CFileExplorer*	pFileExplorer =  m_mapPresenter[pListCtrl]->pFileExplorer;
	if ( pFileExplorer->m_bUpdateIconInProgress )
		return FALSE;

	CString strCurDir = pFileExplorer->GetCurDir();

	WIN32_FIND_DATA FindData; // File information

	HANDLE hFind = FindFirstFile ( strCurDir+strNewValue, &FindData);
	BOOL bExist = (hFind != INVALID_HANDLE_VALUE);

	BOOL bRet = FALSE;
	if ( !bExist || strOldValue.CompareNoCase(strNewValue) == 0 )
	{
		bRet = OnRename(strCurDir+strOldValue, strCurDir+strNewValue);
		if ( bRet && pCellFormat->cellType == cellCtrlFile )
		{
			int nToken = strNewValue.ReverseFind(_T('.'));
			if ( nToken != -1 )
			{
				CString strExt = strNewValue.Right(strNewValue.GetLength()-nToken-1);
				pCellFormat->SetPicture(GetIconFromFile(FILE_ATTRIBUTE_NORMAL, pFileExplorer->GetCurDir()+strNewValue));

				((CCustomListCtrl*)pListCtrl)->SetCellText(nRow,2, strExt, FALSE, TRUE);
			}
		}
	}
	else
	{
		CString strShow;
		strShow.Format(_M("%s exist! Please input another name!"), strNewValue);
		AfxMessageBox(strShow);
		bRet = FALSE;
	}

	return bRet;
}

BOOL CDiskFileManager::OnRowLDblClicked( CListCtrl* pListCtrl, int nRow, int nCol, UINT nFlags, CPoint point )
{
	CFileExplorer* pFileExplorer =  m_mapPresenter[pListCtrl]->pFileExplorer;
	pFileExplorer->SelectChanged();
	return HandleOpen(pListCtrl, nRow);
}

BOOL CDiskFileManager::OnRowRClicked( CListCtrl* pListCtrl, int nRow, int nCol, UINT nFlags, CPoint point )
{
	CMenu menu;
	menu.CreatePopupMenu();

	CMenu subMenuCopy;
	subMenuCopy.CreatePopupMenu();

	CMenu subMenuSendTo;
	subMenuSendTo.CreatePopupMenu();

	CFileExplorer*	pFileExplorer =  m_mapPresenter[pListCtrl]->pFileExplorer;
	pFileExplorer->SelectChanged();
	CString strCurDir = pFileExplorer->GetCurDir();
	CString strMenuText, strName, strOnlyName, strExt, strRename, strRarFileName, strDirForCode;
	BOOL bIsItemDir;

	strMenuText.Format(_M("Open Dir [%s] In Explorer"), strCurDir);
	menu.AppendMenu(MF_STRING|MF_ENABLED, 1, strMenuText );
	menu.SetMenuItemBitmaps(1, MF_BYCOMMAND, m_pBitmapExplorer, NULL);

	strMenuText.Format(_M("Open Dir [%s] In Dos Command"), strCurDir);
	menu.AppendMenu(MF_STRING|MF_ENABLED, 2, strMenuText );
	menu.SetMenuItemBitmaps(2, MF_BYCOMMAND, m_pBitmapCmd, NULL);

	strMenuText.Format(_M("Add Dir [%s] To Favorite"), strCurDir);
	menu.AppendMenu(MF_STRING|MF_ENABLED, 106, strMenuText );
	menu.SetMenuItemBitmaps(106, MF_BYCOMMAND, m_pBitmapExplorer, NULL);

	if ( m_mapPresenter.size() < 8 )
	{
		strMenuText.Format(_M("Open Dir [%s] In New Cell"), strCurDir);
		menu.AppendMenu(MF_STRING|MF_ENABLED, 13, strMenuText );
		menu.SetMenuItemBitmaps(13, MF_BYCOMMAND, m_pBitmapMultiExplorer, NULL);
	}
	
	CCustomListCtrl* pList = (CCustomListCtrl*)pListCtrl;
	vector<int> vecSel = pList->FindSelectedRow();
	int nSelCount = vecSel.size();
	if ( nSelCount == 1 )
	{
		ASSERT(nRow == vecSel[0]);
	}

	CDiskFile diskFile;
	diskFile.strWorkDir = strCurDir;
	GetDiskFileSelection(pList, diskFile);

	if ( nRow != -1 )
	{
		CellFormat* pCellFormat = pList->GetCellFormat(nRow,0);
		ASSERT(pCellFormat);

		bIsItemDir = (pCellFormat->cellType == cellCtrlDirectory);
		strName = pList->GetItemText(nRow,0);

		strOnlyName = strName;
		strExt = _T("");
		if ( !bIsItemDir )
		{
			int nToken = strName.ReverseFind(_T('.'));
			if ( nToken != -1 )
			{
				strExt = strName.Right(strName.GetLength()-nToken-1);
				strOnlyName = strName.Left(nToken);
			}
		}


		menu.AppendMenu(MF_SEPARATOR);

		if ( bIsItemDir )
		{
			strMenuText.Format(_M("Open Dir [%s] In Explorer"), strCurDir+strName);
			menu.AppendMenu(MF_STRING|MF_ENABLED, 3, strMenuText );
			menu.SetMenuItemBitmaps(3, MF_BYCOMMAND, m_pBitmapExplorer, NULL);

			strMenuText.Format(_M("Open Dir [%s] In Dos Command"), strCurDir+strName);
			menu.AppendMenu(MF_STRING|MF_ENABLED, 4, strMenuText );
			menu.SetMenuItemBitmaps(4, MF_BYCOMMAND, m_pBitmapCmd, NULL);

			strMenuText.Format(_M("Add Dir [%s] To Favorite"), strCurDir+strName);
			menu.AppendMenu(MF_STRING|MF_ENABLED, 107, strMenuText );
			menu.SetMenuItemBitmaps(107, MF_BYCOMMAND, m_pBitmapExplorer, NULL);

			if ( m_mapPresenter.size() < 8 )
			{
				strMenuText.Format(_M("Open Dir [%s] In New Cell"), strCurDir+strName);
				menu.AppendMenu(MF_STRING|MF_ENABLED, 14, strMenuText );
				menu.SetMenuItemBitmaps(14, MF_BYCOMMAND, m_pBitmapMultiExplorer, NULL);
			}


		}
		else
		{
			if ( strExt.CompareNoCase(_T("bat")) == 0 || strExt.CompareNoCase(_T("exe")) == 0 )
			{
				strMenuText.Format(_M("Run File [%s] in Dos Command"), strName);
				menu.AppendMenu(MF_STRING|MF_ENABLED, 25, strMenuText );
				menu.SetMenuItemBitmaps(25, MF_BYCOMMAND, m_pBitmapCmd, NULL);

				menu.AppendMenu(MF_SEPARATOR);

			}

			strMenuText.Format(_M("Open File [%s]"), strName);
			menu.AppendMenu(MF_STRING|MF_ENABLED, 5, strMenuText );
			menu.SetMenuItemBitmaps(5, MF_BYCOMMAND, GetBitmapFromIcon((HICON)pCellFormat->hPicture), NULL);

			strMenuText.Format(_M("Open File [%s] In Notepad"), strName);
			menu.AppendMenu(MF_STRING|MF_ENABLED, 6, strMenuText );
			menu.SetMenuItemBitmaps(6, MF_BYCOMMAND, m_pBitmapNotepad, NULL);

			if ( m_userOption.strInternetExplorer.GetLength() > 3 )
			{
				strMenuText.Format(_M("Open File [%s] In Internet Explorer"), strName);
				menu.AppendMenu(MF_STRING|MF_ENABLED, 29, strMenuText );
				menu.SetMenuItemBitmaps(29, MF_BYCOMMAND, m_mapBitmap[m_userOption.strInternetExplorer], NULL);
			}

			if ( m_userOption.strUltraEditPath.GetLength() > 3 )
			{
				strMenuText.Format(_M("Open Selected File In UltraEdit"));
				menu.AppendMenu(MF_STRING|MF_ENABLED, 19, strMenuText );
				menu.SetMenuItemBitmaps(19, MF_BYCOMMAND, m_mapBitmap[m_userOption.strUltraEditPath], NULL);
			}

			if ( m_userOption.strNotepadPPPath.GetLength() > 3 )
			{
				strMenuText.Format(_M("Open Selected File In Notepad++"));
				menu.AppendMenu(MF_STRING|MF_ENABLED, 21, strMenuText );
				menu.SetMenuItemBitmaps(21, MF_BYCOMMAND, m_mapBitmap[m_userOption.strNotepadPPPath], NULL);
			}
		}

		////////////////////////////////////////////////////////////////////////////////////////////
		// Handle Toolbar
		if (diskFile.vecFile.size() > 0 )
		{
			menu.AppendMenu(MF_SEPARATOR);
			strMenuText.Format(_M("Add Selected Files to Toolbar"));
			menu.AppendMenu(MF_STRING|MF_ENABLED, 28, strMenuText );
		}

		////////////////////////////////////////////////////////////////////////////////////////////
		// Handle Beyond Compare 
		if ( m_userOption.strBeyondComparePath.GetLength() > 3)
		{
			CBitmap* pBeyondCompare = m_mapBitmap[m_userOption.strBeyondComparePath];
			if ( nSelCount == 1 )
			{
				menu.AppendMenu(MF_SEPARATOR);
				menu.AppendMenu(MF_STRING|MF_ENABLED, 7, _M("Select Left Side to Compare"));
				menu.SetMenuItemBitmaps(7, MF_BYCOMMAND, pBeyondCompare, NULL);

				if ( !m_strCompareLeft.IsEmpty() )
				{
					if ( bIsItemDir == m_bIsCompareLeftDir )
					{
						strMenuText.Format(_M("Compare to [%s] In Beyond Compare"), m_strCompareLeft);
						menu.AppendMenu(MF_STRING|MF_ENABLED, 8, strMenuText);
						menu.SetMenuItemBitmaps(8, MF_BYCOMMAND, pBeyondCompare, NULL);
					}
				}
			}
			else if ( nSelCount == 2 )
			{
				if ( pList->GetCellFormat(vecSel[0],0)->cellType == pList->GetCellFormat(vecSel[1],0)->cellType )
				{
					menu.AppendMenu(MF_SEPARATOR);
					menu.AppendMenu(MF_STRING|MF_ENABLED, 9, _M("Compare In Beyond Compare"));
					menu.SetMenuItemBitmaps(9, MF_BYCOMMAND, pBeyondCompare, NULL);
				}
			}
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////
		// WinRar
		if ( m_userOption.strWinRarPath.GetLength() > 3 && nSelCount > 0 )
		{
			CBitmap* pWinRar = m_mapBitmap[m_userOption.strWinRarPath];
			menu.AppendMenu(MF_SEPARATOR);

			strRarFileName = strOnlyName + _T(".rar");

			if ( nSelCount == 1 && strExt.CompareNoCase(_T("rar")) == 0 )
			{
				menu.AppendMenu(MF_STRING|MF_ENABLED, 22, _M("Extract To Current Directory"));
				menu.SetMenuItemBitmaps(22, MF_BYCOMMAND, pWinRar, NULL);

				strMenuText.Format(_M("Extract To %s"), strOnlyName+_T("\\"));
				menu.AppendMenu(MF_STRING|MF_ENABLED, 23, strMenuText);
				menu.SetMenuItemBitmaps(23, MF_BYCOMMAND, pWinRar, NULL);
			}
			else
			{
				strMenuText.Format(_M("Add To %s"), strRarFileName);
				menu.AppendMenu(MF_STRING|MF_ENABLED, 24, strMenuText);
				menu.SetMenuItemBitmaps(24, MF_BYCOMMAND, pWinRar, NULL);
			}
		}

		///////////////////////////////////////////////////////////////////////////////////
		// Send To
		if ( nSelCount > 0 )
		{
			subMenuSendTo.AppendMenu(MF_STRING|MF_ENABLED, 104, _M("Desktop") );
			subMenuSendTo.AppendMenu(MF_STRING|MF_ENABLED, 105, _M("My Documents") );
			subMenuSendTo.AppendMenu(MF_SEPARATOR);

			for ( int i = 0; i < (int)m_vecDiskDriverInfoLast.size(); i ++)
			{
				subMenuSendTo.AppendMenu(MF_STRING|MF_ENABLED, 1000+i, m_vecDiskDriverInfoLast[i].strLabel+_T(" ( ")+m_vecDiskDriverInfoLast[i].strDriverLetter+_T(" )") );
			}
			

			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_POPUP, (UINT)subMenuSendTo.m_hMenu, _M("Send To"));
		}
	
		//////////////////////////////////////////////////////////////////////////////////////
		// Rename
		if ( nSelCount == 1 && !pFileExplorer->m_bUpdateIconInProgress)
		{
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING|MF_ENABLED, 11, _M("Rename") );

			strRename = GetDefaultRename(strCurDir, strName);
			strMenuText.Format(_M("Rename To %s"), strRename);
			menu.AppendMenu(MF_STRING|MF_ENABLED, 12, strMenuText);
		}

		/////////////////////////////////////////////////////////////////////////////////////////
		// Delete
		if ( !pFileExplorer->m_bUpdateIconInProgress )
		{
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING|MF_ENABLED, 15, _M("Delete") );
		}

		///////////////////////////////////////////////////////////////////////////////////////////////
		// Copy,Cut
		if ( nSelCount > 0 )
		{
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING|MF_ENABLED, 16, _M("Copy") );

			if ( !pFileExplorer->m_bUpdateIconInProgress )
			{
				menu.AppendMenu(MF_STRING|MF_ENABLED, 17, _M("Cut") );
			}
		}

	}

	//////////////////////////////////////////////////////////////////////////////////////
	// Paste
	CDiskFile dikFileClipboard;
	BOOL bCutClipboard;
	if ( !pFileExplorer->m_bUpdateIconInProgress  )
	{
		if ( CheckCopyFromClipboard(dikFileClipboard, bCutClipboard) )
		{
			menu.AppendMenu(MF_SEPARATOR);
			menu.AppendMenu(MF_STRING|MF_ENABLED, 18, _M("Paste") );
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////
	// New Directory and File
	if (  !pFileExplorer->m_bUpdateIconInProgress )
	{
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING|MF_ENABLED, 26, _M("Create a New Directory") );
		menu.AppendMenu(MF_STRING|MF_ENABLED, 27, _M("Create a New Text File") );
	}

	///////////////////////////////////////////////////////////////////////////////////
	// Copy Unicode String To Windows Clipboard 
	strDirForCode = strCurDir;
	subMenuCopy.AppendMenu(MF_STRING|MF_ENABLED, 100, strDirForCode );
	if ( nSelCount == 1 )
		subMenuCopy.AppendMenu(MF_STRING|MF_ENABLED, 101, strDirForCode+strName );

	strDirForCode.Replace(_T("\\"), _T("\\\\"));

	subMenuCopy.AppendMenu(MF_SEPARATOR);
	subMenuCopy.AppendMenu(MF_STRING|MF_ENABLED, 102, strDirForCode );
	if ( nSelCount == 1 )
		subMenuCopy.AppendMenu(MF_STRING|MF_ENABLED, 103, strDirForCode+strName );

	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_POPUP, (UINT)subMenuCopy.m_hMenu, _M("Copy Unicode String To Windows Clipboard"));


	//////////////////////////////////////////////////////////////////////////////////////
	// Property
	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING|MF_ENABLED, 20, _M("Property") );

	pListCtrl->ClientToScreen ( &point );
	int nResult = menu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, point.x, point.y, pListCtrl);
	switch ( nResult)
	{
	case 1:	OnOpenDirWithExplorer(strCurDir);
			break;
	case 2: OnOpenDirWithDosCmd(strCurDir);
			break;	
	case 3:	OnOpenDirWithExplorer(strCurDir+strName);
			break;
	case 4: OnOpenDirWithDosCmd(strCurDir+strName);
			break;
	case 5: OnOpenFile(strCurDir+strName,_T(""));
			break;
	case 6: OnOpenFile(strCurDir+strName,_T("Notepad.exe"));
			break;
	case 7: m_strCompareLeft = strCurDir+strName;
			m_bIsCompareLeftDir = bIsItemDir;
			break;
	case 8: OnCompare(m_strCompareLeft, strCurDir+pList->GetItemText(vecSel[0],0));
			break;
	case 9: OnCompare(strCurDir+pList->GetItemText(vecSel[0],0), strCurDir+pList->GetItemText(vecSel[1],0));
			break;
	case 11:pList->StartCellEdit(nRow,0);
			break;
	case 12:if ( OnRename(strCurDir+strName, strCurDir+strRename) )
			{
				pList->SetCellText(nRow,0,strRename, FALSE, TRUE);
			}
			break;
	case 13:OnNewCell(strCurDir);
			break;
	case 14:OnNewCell(strCurDir+strName);
			break;
	case 15:OnDelete(diskFile, pFileExplorer, TRUE);
			break;
	case 16:OnCopy(diskFile);
			break;
	case 17:OnCut(diskFile, pListCtrl);
			break;
	case 18:OnPaste(dikFileClipboard, strCurDir,pFileExplorer, bCutClipboard);
			break;
	case 19:OnOpenMultiFile(diskFile,m_userOption.strUltraEditPath);
			break;
	case 20:OnProperty(diskFile);
			break;
	case 21:OnOpenMultiFile(diskFile,m_userOption.strNotepadPPPath);
			break;
	case 22:OnExtractFile(strCurDir+strName, strCurDir);
			break;
	case 23:OnExtractFile(strCurDir+strName, strCurDir+strOnlyName+_T("\\"));
			break;
	case 24:OnCreateRarFile(strCurDir+strRarFileName, diskFile);
			break;
	case 25:OnRunFileInDosCmd(strName, strCurDir);
			break;
	case 26:OnCreateNewDirectory(pFileExplorer);
			break;
	case 27:OnCreateNewTextFile(pFileExplorer);
			break;
	case 28:OnAddToToolbar(diskFile);
			break;
	case 29:OnOpenFile(strCurDir+strName,m_userOption.strInternetExplorer);
			break;
	case 100:pList->CopyTextToClipboard(strCurDir);
			break;
	case 101:pList->CopyTextToClipboard(strCurDir+strName);
			break;
	case 102:pList->CopyTextToClipboard(strDirForCode);
			break;
	case 103:pList->CopyTextToClipboard(strDirForCode+strName);
			break;
	case 104:OnPaste(diskFile, m_defaultDir.strDesktopDir, pFileExplorer, FALSE);
			break;
	case 105:OnPaste(diskFile, m_defaultDir.strMyDocumentsDir, pFileExplorer, FALSE);
			break;
	case 106:OnAddToFavorite(strCurDir);
			break;
	case 107:OnAddToFavorite(strCurDir+strName+"\\");
			break;
	default:if ( nResult >= 1000 && (nResult-1000)< (int)m_vecDiskDriverInfoLast.size() )
			{
				OnPaste(diskFile, m_vecDiskDriverInfoLast[nResult-1000].strDriverLetter, pFileExplorer, FALSE);
			}
			break;
	}

	return TRUE;
}

void CDiskFileManager::AddDiskFilePresenter( CListCtrl* pListCtrl, CFileExplorer* pFileExplorer )
{
	CDiskFilePresenter* pDfp = new CDiskFilePresenter;
	pDfp->pFileExplorer = pFileExplorer;
	pDfp->vecDirHistory.clear();
	pDfp->nCursor = 0;

	m_mapPresenter[pListCtrl] = pDfp;
}

BOOL CDiskFileManager::OnVirtualKeyHitted( CListCtrl* pListCtrl, UINT uVirtualKey )
{
	CCustomListCtrl* pList = (CCustomListCtrl*)pListCtrl;
	CFileExplorer*	pFileExplorer =  m_mapPresenter[pListCtrl]->pFileExplorer;

	if ( pFileExplorer->m_bUpdateIconInProgress )
		return FALSE;

	if ( uVirtualKey == VK_RETURN )
	{
		vector<int> vecSelectedRows = pList->FindSelectedRow();
		if ( vecSelectedRows.size() != 1 )
			return FALSE;

		return HandleOpen(pListCtrl, vecSelectedRows[0]);
	}
	else if ( uVirtualKey == VK_DELETE )
	{
		CDiskFile diskFile;
		diskFile.strWorkDir = pFileExplorer->GetCurDir();
		GetDiskFileSelection(pList, diskFile);

		//return OnDelete(diskFile, pFileExplorer, TRUE);
		return OnShellFileOperation(diskFile, _T(""), pFileExplorer, FO_DELETE);
	}
	else if ( uVirtualKey == VK_F5 )
	{
		pFileExplorer->OnBnClickedButtonRefresh();
	}
	else if ( uVirtualKey == VK_BACK )
	{
		if ( pFileExplorer->m_btShowUpward.IsWindowEnabled() )
			pFileExplorer->OnBnClickedButtonUpward();
	}

	return FALSE;
	
}

BOOL CDiskFileManager::HandleOpen( CListCtrl* pListCtrl, int nRow )
{
	CCustomListCtrl* pList = (CCustomListCtrl*)pListCtrl;
	if ( nRow == -1 )
		return FALSE;

	CellFormat* pCellFormat = pList->GetCellFormat(nRow,0);
	if ( pCellFormat == NULL )
		return FALSE;

	CString strDirFile;

	CFileExplorer*	pFileExplorer =  m_mapPresenter[pListCtrl]->pFileExplorer;
	strDirFile = pFileExplorer->GetCurDir();
	strDirFile += pListCtrl->GetItemText(nRow,0);

	if ( pCellFormat->cellType == cellCtrlDirectory )
	{
		if ( pFileExplorer->m_bUpdateIconInProgress )
			return FALSE;

		strDirFile += _T("\\");
		pFileExplorer->ShowDirectory(strDirFile, TRUE);
	}
	else
	{
		ShellExecute(NULL, _T("Open"), strDirFile, _T(""), pFileExplorer->GetCurDir(), SW_SHOW );
	}

	return TRUE;

}

BOOL CDiskFileManager::OnRowLClicked( CListCtrl* pListCtrl, int nRow, int nCol, UINT nFlags, CPoint point )
{
	CFileExplorer* pFileExplorer =  m_mapPresenter[pListCtrl]->pFileExplorer;
	pFileExplorer->SelectChanged();
	return TRUE;
}

void CDiskFileManager::DirShown( CListCtrl* pListCtrl, const CString& strDir )
{
	CDiskFilePresenter* pPresenter = m_mapPresenter[pListCtrl];

	for ( int i = pPresenter->nCursor+1; i < (int)pPresenter->vecDirHistory.size(); i++)
	{
		pPresenter->vecDirHistory.pop_back();
	}

	pPresenter->vecDirHistory.push_back(strDir);
	pPresenter->nCursor = pPresenter->vecDirHistory.size()-1;
}

void CDiskFileManager::ShowPrevDir( CListCtrl* pListCtrl )
{
	CDiskFilePresenter* pPresenter = m_mapPresenter[pListCtrl];
	pPresenter->nCursor--;
	pPresenter->pFileExplorer->ShowDirectory(pPresenter->vecDirHistory[pPresenter->nCursor], FALSE);
}

void CDiskFileManager::ShowNextDir( CListCtrl* pListCtrl )
{
	CDiskFilePresenter* pPresenter = m_mapPresenter[pListCtrl];
	pPresenter->nCursor++;
	pPresenter->pFileExplorer->ShowDirectory(pPresenter->vecDirHistory[pPresenter->nCursor], FALSE);
}

BOOL CDiskFileManager::HasPrevDir( CListCtrl* pListCtrl )
{
	CDiskFilePresenter* pPresenter = m_mapPresenter[pListCtrl];
	return pPresenter->nCursor > 0;
}

BOOL CDiskFileManager::HasNextDir( CListCtrl* pListCtrl )
{
	CDiskFilePresenter* pPresenter = m_mapPresenter[pListCtrl];
	return pPresenter->nCursor < (int)pPresenter->vecDirHistory.size()-1;
}

void CDiskFileManager::OnOpenDirWithExplorer(const CString& strDir)
{
	ShellExecute(NULL, _T("Open"), _T("Explorer.exe"), strDir, _T(""), SW_SHOWMAXIMIZED );
}

void CDiskFileManager::OnOpenDirWithDosCmd(const CString& strDir )
{
	CString strCommand;
	strCommand.Format(_T("/k cd /d %s"), strDir);

	ShellExecute(NULL, _T("Open"), _T("cmd"), strCommand, _T(""), SW_SHOW );
}

void CDiskFileManager::OnOpenFile(const CString& strFile, const CString& strOpener)
{
	CString strRunFile, strRunPara;

	if ( strOpener.IsEmpty() )
	{
		strRunFile = strFile;
		strRunPara = _T("");
	}
	else
	{
		strRunFile = strOpener;
		strRunPara = _T("\"") + strFile + _T("\"");
	}

	ShellExecute(NULL, _T("Open"), strRunFile, strRunPara, _T(""), SW_SHOWMAXIMIZED );
}

void CDiskFileManager::OnCompare( const CString& strLeft, const CString& strRight )
{
	CString strRunPara;
	strRunPara.Format(_T("\"%s\" \"%s\""), strLeft, strRight);
	ShellExecute(NULL, _T("Open"), m_userOption.strBeyondComparePath, strRunPara, _T(""), SW_SHOWMAXIMIZED );

	m_strCompareLeft = _T("");
}

CString CDiskFileManager::GetDefaultRename( const CString& strDir, const CString& strName )
{
	CString strExt= _T("");
	CString strOnlyName = strName;

	int nToken = strName.ReverseFind(_T('.'));
	if ( nToken != -1 )
	{
		strExt = strName.Right(strName.GetLength()-nToken);
		strOnlyName = strName.Left(nToken);
	}

	WIN32_FIND_DATA FindData; // File information
	HANDLE hFind;
	strOnlyName += _T("_");
	CString strRename;
	do
	{
		strOnlyName += _T("Old");
		strRename = strDir+strOnlyName+strExt;
		hFind = FindFirstFile ( strRename, &FindData);
	}
	while ( hFind != INVALID_HANDLE_VALUE );

	return strOnlyName+strExt;
}

BOOL CDiskFileManager::OnRename( const CString& strOld, const CString& strNew )
{
	if ( !MoveFile(strOld, strNew) )
	{
		CString strShow;
		strShow.Format(_M("Can NOT rename file %s to %s!\r\nError : "), strOld, strNew, m_commonTool.GetLastErrorString());
		AfxMessageBox(strShow);
		return FALSE;
	}

	return TRUE;
}

CBitmap* CDiskFileManager::GetBitmapFromFile( const CString& strFile )
{
	HANDLE hIcon = GetIconFromFile(FILE_ATTRIBUTE_NORMAL, strFile);

	return GetBitmapFromIcon((HICON)hIcon);


	//ICONINFO iconInfo;
	//GetIconInfo(hIcon, &iconInfo);
	//return CBitmap::FromHandle(iconInfo.hbmColor);
}

HANDLE CDiskFileManager::GetIconFromFile( DWORD dwAttr, const CString& strFile )
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
	SHGetFileInfo(strFile, dwAttr, &sInfo, sizeof(sInfo),
		SHGFI_USEFILEATTRIBUTES|SHGFI_SMALLICON|SHGFI_SYSICONINDEX);

	HANDLE hIcon = pImageList->ExtractIcon(sInfo.iIcon);
	DeleteObject(hImagelist);

	return hIcon;
}

BOOL CDiskFileManager::OnProperty( const CDiskFile& diskFile )
{
	TCHAR szFiles[409600];
	UINT uBufLen;
	ConvertToClipboardFormat(diskFile, szFiles, uBufLen);


	SHELLEXECUTEINFO ShExecInfo ={0};
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_INVOKEIDLIST ;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = "properties";
	ShExecInfo.lpFile = szFiles;
	ShExecInfo.lpParameters = ""; 
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL; 
	BOOL bOk = ShellExecuteEx(&ShExecInfo);
	
	return TRUE;
}

CBitmap* CDiskFileManager::GetBitmapFromIcon( HICON hIcon )
{
	CDC dc;
	CBitmap bmp;
	CClientDC ClientDC(AfxGetMainWnd());
	dc.CreateCompatibleDC(&ClientDC);
	bmp.CreateCompatibleBitmap(&ClientDC, 14, 14);
	CBitmap* pOldBmp = (CBitmap*)dc.SelectObject(&bmp);
	::DrawIconEx(dc.GetSafeHdc(), 0, 0, hIcon, 14, 14, 0, (HBRUSH)RGB(255, 255, 255), DI_NORMAL);
	dc.SelectObject(pOldBmp);
	dc.DeleteDC();
	HBITMAP hBitmap = (HBITMAP)::CopyImage((HANDLE)((HBITMAP)bmp), IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE);

	return CBitmap::FromHandle(hBitmap);
}

void CDiskFileManager::OnNewCell( const CString& strDir )
{
	int nCount = (int)m_mapPresenter.size();
	if ( nCount >= 8 )
		return;

	CString strLayout = m_layoutManager.SelectLayoutString(GetExplorerCount()+1, FALSE);
	if ( strLayout.IsEmpty())
		return;

	CFileExplorer* pFileExplorer = new CFileExplorer();
	pFileExplorer->m_nLayoutIndex = nCount + 1;
	pFileExplorer->m_listFile.m_nSortColumn = 0;
	pFileExplorer->m_listFile.m_bSortAscending = TRUE;
	pFileExplorer->m_pDiskFileManager = this;
	pFileExplorer->m_strCurDir = strDir;
	if ( pFileExplorer->m_strCurDir.GetAt(pFileExplorer->m_strCurDir.GetLength()-1) != _T('\\') )
	{
		pFileExplorer->m_strCurDir += _T("\\");
	}
	pFileExplorer->m_strFilter = _T("");

	pFileExplorer->m_vecColumnWidth.push_back(100);
	pFileExplorer->m_vecColumnWidth.push_back(70);
	pFileExplorer->m_vecColumnWidth.push_back(40);
	pFileExplorer->m_vecColumnWidth.push_back(110);

	AddDiskFilePresenter(&pFileExplorer->m_listFile, pFileExplorer);

	pFileExplorer->Create(IDD_DIALOG_EXPLORER, m_pUserWnd);

	m_userOption.strLayout = strLayout;
	CRect rcClient;
	m_pUserWnd->GetClientRect(&rcClient);
	DoRelayout(rcClient, m_userOption.strLayout, TRUE);
	SetActiveFileExplorer(pFileExplorer);
}

BOOL CDiskFileManager::Init(CWnd* pUserWnd)
{
	InitDefaultDirs();
	InitDefaultBitmaps();
	GetDiskDriverList(TRUE);

	m_pUserWnd = pUserWnd;

	m_strAppName = GetAppName();
	m_pUserWnd->SetWindowText(m_strAppName);

	m_strIniFile = m_defaultDir.strTempDir + m_strAppName + _T(".ini");
	m_strRumCmdFile = m_defaultDir.strTempDir + _T("RunCmd.exe");

	m_toolbarDialog.SetDiskFileManager(this);
	m_toolbarDialog.Create(IDD_DIALOG_TOOLBAR, m_pUserWnd);

	m_dirTreeDialog.SetDiskFileManager(this);
	m_dirTreeDialog.Create(IDD_DIALOG_DIR_TREE, m_pUserWnd);

	m_faviouriteDirDialog.SetDiskFileManager(this);
	m_faviouriteDirDialog.Create(IDD_DIALOG_DIR_FAVIOURITE, m_pUserWnd);

	if ( m_strDefaultIniFile.IsEmpty())
	{
		m_commonTool.OutFile( IDR_CONFIG, "IDR_EXE", m_strIniFile.GetBuffer(0));
		if ( !LoadIniFile(m_strIniFile,FALSE) )
			return FALSE;
	}
	else
	{
		if ( !LoadIniFile(m_strDefaultIniFile,FALSE) )
			return FALSE;

	}

	if ( !m_userOption.bShowDirTree )
	{
		m_dirTreeDialog.ShowWindow(SW_HIDE);
	}

	CRect rcClient;
	pUserWnd->GetClientRect(&rcClient);
	Relayout(rcClient, FALSE, FALSE, FALSE);

	StartNetWorkerThread();

	return TRUE;
}

void CDiskFileManager::StartNetWorkerThread()
{
	if (m_userOption.nLocalListeningPort != -1)
		AfxBeginThread(NetWorkerThreadFunc, (void*)this);
}

BOOL CDiskFileManager::LoadIniFile(CString strFileName, BOOL bEncrypt)
{
	m_iniFile.Clear();
	if ( !m_iniFile.Read ( strFileName, bEncrypt ) )
		return FALSE;

	m_iniFile.GetValue ( "General", "Language", m_userOption.strLanguage );
	CMultiLanguage::SetLanguage(m_userOption.strLanguage == _T("English"));

	m_iniFile.GetValue ( "General", "Layout", m_userOption.strLayout );
	m_iniFile.GetValue ( "General", "InternetExplorerPath", m_userOption.strInternetExplorer );
	m_iniFile.GetValue ( "General", "BeyondComparePath", m_userOption.strBeyondComparePath );
	m_iniFile.GetValue ( "General", "UltraEditPath", m_userOption.strUltraEditPath );
	m_iniFile.GetValue ( "General", "NotepadPPPath", m_userOption.strNotepadPPPath );
	m_iniFile.GetValue ( "General", "WinRarPath", m_userOption.strWinRarPath );
	m_iniFile.GetValue ( "General", "RowHeight", m_userOption.nRowHeight );
	m_iniFile.GetValue ( "General", "PromptForSave", m_userOption.bPromptForSave );
	m_iniFile.GetValue ( "General", "ShowDirTree", m_userOption.bShowDirTree );
	m_iniFile.GetValue ( "General", "DirTreeDlgWidth", m_userOption.nDirTreeDlgWidth );

	int nExplorerCount = 0;
	if ( !m_iniFile.GetValue ( "Explorers", "ExplorerCount", nExplorerCount ))
	{
		return FALSE;
	}

	int nActiveExplorerIndex = 1;
	BOOL bFindActiveExplorer = FALSE;
	m_iniFile.GetValue ( "Explorers", "ActiveExplorerIndex", nActiveExplorerIndex );

	m_iniFile.GetValue ( "Explorers", "OddLineBKColorActive", m_userOption.strOddLineBKColorActive );
	m_iniFile.GetValue ( "Explorers", "EvenLineBKColorActive", m_userOption.strEvenLineBKColorActive );
	m_iniFile.GetValue ( "Explorers", "OddLineBKColorInactive", m_userOption.strOddLineBKColorInactive );
	m_iniFile.GetValue ( "Explorers", "EvenLineBKColorInactive", m_userOption.strEvenLineBKColorInactive );

	CString strTemp,strKey;
	int nWidth;
	for ( int i = 1; i <= nExplorerCount; i++ )
	{
		CFileExplorer* pFileExplorer = new CFileExplorer();
		pFileExplorer->m_pDiskFileManager = this;
		pFileExplorer->m_nLayoutIndex = i;
		AddDiskFilePresenter(&pFileExplorer->m_listFile, pFileExplorer);

		strTemp.Format(_T("Explorer%02d"), i);
		for ( int j = 0; j < 4; j ++ )
		{
			strKey.Format(_T("%s.ColumnWidth%d"), strTemp, j);
			m_iniFile.GetValue ( "Explorers", (const char*)strKey, nWidth );
			pFileExplorer->m_vecColumnWidth.push_back(nWidth);
		}

		strKey = strTemp + _T(".WorkDir");
		m_iniFile.GetValue( "Explorers", (const char*)strKey, pFileExplorer->m_strCurDir );

		strKey = strTemp + _T(".Filter");
		m_iniFile.GetValue( "Explorers", (const char*)strKey, pFileExplorer->m_strFilter );
		pFileExplorer->m_strFilter.TrimRight();

		strKey = strTemp + _T(".SortColumn");
		m_iniFile.GetValue( "Explorers", (const char*)strKey, pFileExplorer->m_listFile.m_nSortColumn );

		strKey = strTemp + _T(".SortAscending");
		m_iniFile.GetValue( "Explorers", (const char*)strKey, pFileExplorer->m_listFile.m_bSortAscending );

		pFileExplorer->Create(IDD_DIALOG_EXPLORER, m_pUserWnd);

		if ( i == nActiveExplorerIndex )
		{
			SetActiveFileExplorer(pFileExplorer);
			bFindActiveExplorer = TRUE;
		}
	}

	if ( !bFindActiveExplorer )
	{
		m_pActiveFileExplorer = NULL;
	}

	// Toolbar
	int nToolbarButtonCount = 0;
	if ( !m_iniFile.GetValue ( "ToolbarButtons", "ToolbarButtonCount", nToolbarButtonCount ))
	{
		return FALSE;
	}

	m_iniFile.GetValue ( "ToolbarButtons", "LoadQuickLaunchFromTaskbar", m_userOption.bLoadQuickLaunchFromTaskbar );
	m_iniFile.GetValue ( "ToolbarButtons", "LoadQuickLaunchFromDesktop", m_userOption.bLoadQuickLaunchFromDesktop );

	m_iniFile.GetValue ( "ToolbarButtons", "ToolbarButtonSize", m_userOption.nToolbarButtonSize );
	m_toolbarDialog.m_nButtonSize = m_userOption.nToolbarButtonSize;

	CString strButtonFile;
	for ( int i = 1; i <= nToolbarButtonCount; i++ )
	{
		strKey.Format(_T("ToolbarButton%02d"), i);
		m_iniFile.GetValue( "ToolbarButtons", (const char*)strKey, strButtonFile );

		m_toolbarDialog.AppendQuickLaunchButton(strButtonFile, TRUE);
	}

	// Favorite dir
	int nFavoriteDirectoryCount = 0;
	if ( !m_iniFile.GetValue ( "FavoriteDirectories", "FavoriteDirectoryCount", nFavoriteDirectoryCount ))
	{
		return FALSE;
	}

	CString strGroup, strName, strDir;
	for ( int i = 1; i <= nFavoriteDirectoryCount; i++ )
	{
		strKey.Format(_T("FavoriteDirectory%02d.Group"), i);
		m_iniFile.GetValue( "FavoriteDirectories", (const char*)strKey, strGroup );

		strKey.Format(_T("FavoriteDirectory%02d.Name"), i);
		m_iniFile.GetValue( "FavoriteDirectories", (const char*)strKey, strName );

		strKey.Format(_T("FavoriteDirectory%02d.Dir"), i);
		m_iniFile.GetValue( "FavoriteDirectories", (const char*)strKey, strDir );

		m_faviouriteDirDialog.AddFavoriteDir(strGroup, strName, strDir);
	}

	if ( m_userOption.bLoadQuickLaunchFromTaskbar )
	{
		LoadQuickLaunchFromWindowDefault();
	}

	if ( m_userOption.bLoadQuickLaunchFromDesktop )
	{
		LoadQuickLaunchFromDesktop();
	}

	// Load local listening port and Remote Machine
	if (!m_iniFile.GetValue("Network", "LocalListeningPort", m_userOption.nLocalListeningPort))
		m_userOption.nLocalListeningPort = 1229;
	m_iniFile.GetValue("Network", "ClipboardUnicode", m_userOption.bClipboardUnicode);
	m_iniFile.GetValue("Network", "AppendMessage", m_userOption.bAppendMessage);

	int nRemoteMachineCount = 0;
	m_iniFile.GetValue("Network", "RemoteMachineCount", nRemoteMachineCount);

	for (int i = 1; i <= nRemoteMachineCount; i++)
	{
		RemoteMachine remoteMachine;

		strTemp.Format(_T("RemoteMachine%02d"), i);

		strKey = strTemp + _T(".IpAddress");
		m_iniFile.GetValue("Network", (const char*)strKey, remoteMachine.strIpAddress);

		strKey = strTemp + _T(".Port");
		m_iniFile.GetValue("Network", (const char*)strKey, remoteMachine.nPort);

		m_userOption.vecRemoteMachine.push_back(remoteMachine);
	}

	RebuildBitmapMap();

	return TRUE;
}


BOOL CDiskFileManager::ExportIniFile(const CString& strFileName, BOOL bEncrypt)
{
	m_iniFile.Clear();

	m_iniFile.SetValue ( "General", "Language", m_userOption.strLanguage );
	m_iniFile.SetValue ( "General", "Layout", m_userOption.strLayout );
	m_iniFile.SetValue ( "General", "InternetExplorerPath", m_userOption.strInternetExplorer );
	m_iniFile.SetValue ( "General", "BeyondComparePath", m_userOption.strBeyondComparePath );
	m_iniFile.SetValue ( "General", "UltraEditPath", m_userOption.strUltraEditPath );
	m_iniFile.SetValue ( "General", "NotepadPPPath", m_userOption.strNotepadPPPath );
	m_iniFile.SetValue ( "General", "WinRarPath", m_userOption.strWinRarPath );
	m_iniFile.SetValue ( "General", "RowHeight", m_userOption.nRowHeight );
	m_iniFile.SetValue ( "General", "PromptForSave", m_userOption.bPromptForSave );
	m_iniFile.SetValue ( "General", "ShowDirTree", m_userOption.bShowDirTree );
	m_iniFile.SetValue ( "General", "DirTreeDlgWidth", m_userOption.nDirTreeDlgWidth );

	int nCount = m_mapPresenter.size();
	m_iniFile.SetValue ( "Explorers", "ExplorerCount", nCount );

	m_iniFile.SetValue ( "Explorers", "ActiveExplorerIndex", m_pActiveFileExplorer->m_nLayoutIndex );

	m_iniFile.SetValue ( "Explorers", "OddLineBKColorActive", m_userOption.strOddLineBKColorActive );
	m_iniFile.SetValue ( "Explorers", "EvenLineBKColorActive", m_userOption.strEvenLineBKColorActive );
	m_iniFile.SetValue ( "Explorers", "OddLineBKColorInactive", m_userOption.strOddLineBKColorInactive );
	m_iniFile.SetValue ( "Explorers", "EvenLineBKColorInactive", m_userOption.strEvenLineBKColorInactive );

	CString strTemp, strKey;
	CDiskFilePresenter* pPresenter;

	map<CListCtrl*, CDiskFilePresenter*>::iterator it;
	for ( it = m_mapPresenter.begin(); it != m_mapPresenter.end(); it++ )
	{
		pPresenter = it->second;

		strTemp.Format(_T("Explorer%02d"), pPresenter->pFileExplorer->m_nLayoutIndex);

		for ( int j = 0; j < 4; j ++ )
		{
			strKey.Format(_T("%s.ColumnWidth%d"), strTemp, j);
			m_iniFile.SetValue ( "Explorers", (const char*)strKey, pPresenter->pFileExplorer->GetColumnWidth(j) );
		}

		strKey = strTemp + _T(".WorkDir");
		m_iniFile.SetValue ( "Explorers", (const char*)strKey, pPresenter->pFileExplorer->m_strCurDir );

		strKey = strTemp + _T(".Filter");
		m_iniFile.SetValue( "Explorers", (const char*)strKey, pPresenter->pFileExplorer->GetFilter() );

		strKey = strTemp + _T(".SortColumn");
		m_iniFile.SetValue( "Explorers", (const char*)strKey, pPresenter->pFileExplorer->m_listFile.m_nSortColumn );

		strKey = strTemp + _T(".SortAscending");
		m_iniFile.SetValue( "Explorers", (const char*)strKey, pPresenter->pFileExplorer->m_listFile.m_bSortAscending );
	}


	m_iniFile.SetValue ( "ToolbarButtons", "LoadQuickLaunchFromTaskbar", m_userOption.bLoadQuickLaunchFromTaskbar );
	m_iniFile.SetValue ( "ToolbarButtons", "LoadQuickLaunchFromDesktop", m_userOption.bLoadQuickLaunchFromDesktop );
	m_iniFile.SetValue ( "ToolbarButtons", "ToolbarButtonSize", m_userOption.nToolbarButtonSize );

	list<CToolbarButton*>::iterator itButton;
	int i = 1;
	for ( itButton = m_toolbarDialog.m_listQuickLaunchButtons.begin(); itButton != m_toolbarDialog.m_listQuickLaunchButtons.end(); itButton ++)
	{
		CToolbarButton* pToolbarButton = *itButton;
		if ( !pToolbarButton->bNeedSave )
			continue;

		strKey.Format(_T("ToolbarButton%02d"), i);
		m_iniFile.SetValue( "ToolbarButtons", (const char*)strKey, pToolbarButton->strButtonFile );

		i++;
	}

	m_iniFile.SetValue ( "ToolbarButtons", "ToolbarButtonCount", i-1 );

	// Favorite directories
	int nFavoriteDirectoryCount = 0;
	HTREEITEM hChild = m_faviouriteDirDialog.m_pDirTree->GetChildItem(m_faviouriteDirDialog.m_hRootItem);
	while ( hChild != NULL)
	{
		CString strGroup = m_faviouriteDirDialog.m_pDirTree->GetItemText(hChild);
		HTREEITEM hGrandChild = m_faviouriteDirDialog.m_pDirTree->GetChildItem(hChild);

		while (hGrandChild != NULL)
		{
			nFavoriteDirectoryCount++;
			CString strName = m_faviouriteDirDialog.m_pDirTree->GetItemText(hGrandChild);
			CString* pData = (CString*) m_faviouriteDirDialog.m_pDirTree->GetItemData(hGrandChild);
			
			strKey.Format(_T("FavoriteDirectory%02d.Group"), nFavoriteDirectoryCount);
			m_iniFile.SetValue( "FavoriteDirectories", (const char*)strKey, strGroup );

			strKey.Format(_T("FavoriteDirectory%02d.Name"), nFavoriteDirectoryCount);
			m_iniFile.SetValue( "FavoriteDirectories", (const char*)strKey, strName );

			strKey.Format(_T("FavoriteDirectory%02d.Dir"), nFavoriteDirectoryCount);
			m_iniFile.SetValue( "FavoriteDirectories", (const char*)strKey, *pData );
		
			hGrandChild = m_faviouriteDirDialog.m_pDirTree->GetNextSiblingItem(hGrandChild);
		}

		hChild = m_faviouriteDirDialog.m_pDirTree->GetNextSiblingItem(hChild);
	}
	m_iniFile.SetValue ( "FavoriteDirectories", "FavoriteDirectoryCount", nFavoriteDirectoryCount );

	// Export local listening port and Remote Machine
	m_iniFile.SetValue("Network", "LocalListeningPort", m_userOption.nLocalListeningPort);
	m_iniFile.SetValue("Network", "ClipboardUnicode", m_userOption.bClipboardUnicode);
	m_iniFile.SetValue("Network", "AppendMessage", m_userOption.bAppendMessage);

	int nRemoteMachineCount = m_userOption.vecRemoteMachine.size();
	m_iniFile.SetValue("Network", "RemoteMachineCount", nRemoteMachineCount);

	for (int i = 0; i < nRemoteMachineCount; i++)
	{
		RemoteMachine remoteMachine = m_userOption.vecRemoteMachine[i];

		strTemp.Format(_T("RemoteMachine%02d"), i+1);

		strKey = strTemp + _T(".IpAddress");
		m_iniFile.SetValue("Network", (const char*)strKey, remoteMachine.strIpAddress);

		strKey = strTemp + _T(".Port");
		m_iniFile.SetValue("Network", (const char*)strKey, remoteMachine.nPort);
	}

	m_iniFile.Write(strFileName, bEncrypt);

	return TRUE;

}

void CDiskFileManager::UpdateIniFileInResource()
{
	char strFileName[MAX_PATH];
	GetModuleFileName ( NULL, strFileName, MAX_PATH );

	CString strCmdLine = "\"" + m_strRumCmdFile + "\" 2*";
	strCmdLine += strFileName;
	strCmdLine += "*IDR_EXE*134*";
	strCmdLine += m_strIniFile;

	ExportIniFile(m_strIniFile, FALSE);

	m_commonTool.OutFile( IDR_EXE_RUN_CMD, "IDR_EXE", m_strRumCmdFile.GetBuffer(0));
	m_commonTool.RunProgram(strCmdLine.GetBuffer(0), "C:\\", SW_HIDE, IGNORE);
}

void CDiskFileManager::DistroyAllCell()
{
	CDiskFilePresenter* pPresenter;

	map<CListCtrl*, CDiskFilePresenter*>::iterator it;
	for ( it = m_mapPresenter.begin(); it != m_mapPresenter.end(); it++ )
	{
		pPresenter = it->second;
		pPresenter->pFileExplorer->DestroyWindow();

		delete pPresenter->pFileExplorer;
		delete pPresenter;
	}

	m_mapPresenter.clear();
	m_pActiveFileExplorer = NULL;
}

BOOL CDiskFileManager::Relayout( const CRect& rcClient ,BOOL bCountChanged, BOOL bMustSelect, BOOL bAdjustListColumn)
{
	int nCount = m_mapPresenter.size();
	if (nCount == 0)
		return FALSE;

	if ( bCountChanged )
	{
		CString strLayout = m_layoutManager.SelectLayoutString(nCount, bMustSelect);
		if ( strLayout.IsEmpty() )
			return FALSE;
		m_userOption.strLayout = strLayout;
	}

	DoRelayout(rcClient, m_userOption.strLayout, bAdjustListColumn);

	return TRUE;
}

void CDiskFileManager::OnDeleteCell( CFileExplorer* pFileExplorer, const CString& strNewLayout )
{
	ASSERT(m_pActiveFileExplorer == pFileExplorer);
	CDiskFilePresenter* pPresenter;

	int nLayoutIndex = pFileExplorer->m_nLayoutIndex;
	map<CListCtrl*, CDiskFilePresenter*>::iterator it;
	for ( it = m_mapPresenter.begin(); it != m_mapPresenter.end(); it++ )
	{
		pPresenter = it->second;
		if( pPresenter->pFileExplorer == pFileExplorer )
		{
			pPresenter->pFileExplorer->DestroyWindow();
			delete pPresenter->pFileExplorer;
			delete pPresenter;

			m_mapPresenter.erase(it);
			break;
		}
	}

	for ( it = m_mapPresenter.begin(); it != m_mapPresenter.end(); it++ )
	{
		pPresenter = it->second;
		if( pPresenter->pFileExplorer->m_nLayoutIndex > nLayoutIndex )
			pPresenter->pFileExplorer->m_nLayoutIndex--;
	}

	m_userOption.strLayout = strNewLayout;
	CRect rcClient;
	m_pUserWnd->GetClientRect(&rcClient);
	DoRelayout(rcClient, m_userOption.strLayout, TRUE);

	m_pActiveFileExplorer = NULL;
	SetActiveFileExplorer(m_mapPresenter.begin()->second->pFileExplorer);
}

void CDiskFileManager::OnChangeLayout(BOOL bMustSelect)
{
	CRect rcClient;
	m_pUserWnd->GetClientRect(&rcClient);
	Relayout(rcClient, TRUE, bMustSelect, TRUE);
}

BOOL CDiskFileManager::OnSort( CListCtrl* pListCtrl, RowItemData* pid1, RowItemData* pid2, int& nCompareResult )
{
	CCustomListCtrl* pList = (CCustomListCtrl*)pListCtrl;

	CellFormat* pCellFormat1 = pid1->FindCellFormat(0, TRUE);
	CellFormat* pCellFormat2 = pid2->FindCellFormat(0, TRUE);
	if ( pCellFormat1->cellType != pCellFormat2->cellType )
	{
		if ( pList->m_bSortAscending )
			nCompareResult = (int)(pCellFormat1->cellType < pCellFormat2->cellType);
		else
			nCompareResult = (int)(pCellFormat1->cellType > pCellFormat2->cellType);

		return TRUE;
	}

	int nSortColumn = pList->GetSortColumnID();

	if ( pCellFormat1->cellType == cellCtrlDirectory )
	{
		if ( nSortColumn != 3 )
		{
			CString strText1 = pid1->GetCellText(0);
			CString strText2 = pid2->GetCellText(0);

			nCompareResult = pList->m_bSortAscending ? lstrcmp( strText1, strText2 ) : lstrcmp( strText2, strText1 );

			return TRUE;
		}
	}

	// Sorting file size
	if ( nSortColumn == 1 )
	{
		CString strText1 = pid1->GetCellText(nSortColumn);
		CString strText2 = pid2->GetCellText(nSortColumn);

		__int64 nSize1 = m_commonTool.ConvertToFileSize(strText1);
		__int64 nSize2 = m_commonTool.ConvertToFileSize(strText2);


		if ( nSize1 < nSize2 )
			nCompareResult = 1;
		else if ( nSize1 == nSize2 )
			nCompareResult = 0;
		else
			nCompareResult = -1;

		if ( pList->m_bSortAscending )
			nCompareResult = -nCompareResult;

		return TRUE;
	}


	return FALSE;
}


BOOL CDiskFileManager::OnDelete( const CDiskFile& diskFile, CFileExplorer*	pFileExplorer, BOOL bConfirm )
{
	if ( bConfirm && !ConfirmDelete(diskFile) )
		return FALSE;

	int nCount = GetFileCount(diskFile);

	m_pOperationDlg = new COperationDialog;
	m_pOperationDlg->SetOperatioinType(operationDelete);
	m_pOperationDlg->Show(0, nCount);

	BOOL bFailed = FALSE;
	nCount = diskFile.vecDirectory.size();
	for ( int i = 0; i < nCount; i ++ )
	{
		if ( !DeleteDir(diskFile.strWorkDir+diskFile.vecDirectory[i]+_T("\\")) )
		{
			bFailed = TRUE;
			break;
		}
	}

	if ( !bFailed )
	{
		nCount = diskFile.vecFile.size();
		for ( int i = 0; i < nCount; i ++ )
		{
			if ( !DeleteFile(diskFile.strWorkDir, diskFile.vecFile[i]) )
			{
				bFailed = TRUE;
				break;
			}
		}
	}

	m_pOperationDlg->DestroyWindow();
	delete m_pOperationDlg;

	if ( !bFailed )
	{
		if ( pFileExplorer )
			pFileExplorer->OnBnClickedButtonRefresh();
		return TRUE;
	}

	return FALSE;
}

int CDiskFileManager::GetFileCount( const CString& strDir )
{
	vector<CString> vecDir, vecFile;

	if ( !GetDirFileList(strDir, vecDir, vecFile) )
		return -1;

	int nCount = vecFile.size();

	for ( int i = 0; i < (int)vecDir.size(); i ++ )
	{
		nCount += GetFileCount(strDir+vecDir[i]+_T("\\"));
	}

	return nCount;
}

int CDiskFileManager::GetFileCount( const CDiskFile& diskFile )
{
	int nCount = diskFile.vecFile.size();

	for ( int i = 0; i < (int)diskFile.vecDirectory.size(); i ++ )
	{
		nCount += GetFileCount(diskFile.strWorkDir+diskFile.vecDirectory[i]+_T("\\"));
	}

	return nCount;
}

void CDiskFileManager::GetDiskFileSelection( CCustomListCtrl* pListCtrl, CDiskFile& diskFile )
{
	diskFile.Init();

	vector<int> vecSel = pListCtrl->FindSelectedRow();
	int nCount = vecSel.size();
	CellFormat* pCellFormat;
	CString strName;
	for ( int i = 0; i < nCount; i ++ )
	{
		pCellFormat = pListCtrl->GetCellFormat(vecSel[i], 0);
		strName = pListCtrl->GetItemText(vecSel[i],0);

		if ( pCellFormat->cellType == cellCtrlDirectory )
			diskFile.vecDirectory.push_back(strName);
		else
			diskFile.vecFile.push_back(strName);
	}
}

BOOL CDiskFileManager::DeleteFile( const CString& strDir, const CString& strFile )
{
	CString strPathName = strDir+strFile;
	BOOL bOk = ::DeleteFile(strPathName);
	
	if ( !bOk )
	{
		SetFileAttributes(strPathName, GetFileAttributes(strPathName)&~FILE_ATTRIBUTE_READONLY);
		bOk = ::DeleteFile(strPathName);
		if ( !bOk )
		{
			CString strError;
			strError.Format(_M("Can NOT delete file %s. \r\nReason : %s"), strPathName, m_commonTool.GetLastErrorString());
			AfxMessageBox(strError);
		}
	}
	
	return m_pOperationDlg->Step(strDir, strFile);
}

BOOL CDiskFileManager::DeleteDir( const CString& strDir )
{
	vector<CString> vecDir, vecFile;

	if ( !GetDirFileList(strDir, vecDir, vecFile) )
		return FALSE;

	int nCount = vecFile.size();
	for ( int i = 0; i < nCount; i ++ )
	{
		if ( !DeleteFile(strDir, vecFile[i]) )
			return FALSE;
	}

	nCount = vecDir.size();
	for ( int i = 0; i < nCount; i ++ )
	{
		if ( !DeleteDir(strDir+vecDir[i]+_T("\\")) )
			return FALSE;
	}

	return ::RemoveDirectory(strDir);
}

BOOL CDiskFileManager::ConfirmDelete( const CDiskFile& diskFile )
{
	int nDirCount = diskFile.vecDirectory.size();
	int nFileCount = diskFile.vecFile.size();

	CString strShow;
	if ( nFileCount+nDirCount > 1 )
	{
		strShow.Format(_M("Are you sure to delete these %d items?"), nFileCount+nDirCount);
	}
	else if ( nFileCount+nDirCount == 1)
	{
		if ( nDirCount == 1 )
		{
			ASSERT(nFileCount==0);
			strShow.Format(_M("Are you sure to delete \" %s \" ?"), diskFile.vecDirectory[0]);
		}
		else
		{
			ASSERT(nFileCount==1);
			strShow.Format(_M("Are you sure to delete \" %s \" ?"), diskFile.vecFile[0]);
		}
	}
	else
	{
		return FALSE;
	}

	return ( AfxMessageBox(strShow, MB_YESNO) == IDYES );
}

BOOL CDiskFileManager::OnKeyHitted( CListCtrl* pListCtrl, UINT uKey )
{
	CFileExplorer*	pFileExplorer =  m_mapPresenter[pListCtrl]->pFileExplorer;
	CString strCurDir = pFileExplorer->GetCurDir();

	CCustomListCtrl* pList = (CCustomListCtrl*)pListCtrl;
	CDiskFile diskFile;
	diskFile.strWorkDir = strCurDir;
	GetDiskFileSelection(pList, diskFile);

	switch ( uKey )
	{
	case CTRL_A:pList->SetAllItemSelected(TRUE);
				pFileExplorer->SelectChanged();
				break;
	case CTRL_C:OnCopy(diskFile);
				break;
	case CTRL_X:OnCut(diskFile, pListCtrl);
				break;
	case CTRL_V:
				CDiskFile dikFileClipboard;
				BOOL bCutClipboard;
				if ( CheckCopyFromClipboard(dikFileClipboard, bCutClipboard) )
					OnShellFileOperation(dikFileClipboard, strCurDir, pFileExplorer, bCutClipboard? FO_MOVE : FO_COPY);
				break;
	}

	return TRUE;
}

void CDiskFileManager::OnCopy( const CDiskFile& diskFile )
{
	CopyOrCutToClipboard(diskFile, TRUE);
}

void CDiskFileManager::OnCut( const CDiskFile& diskFile, CListCtrl* pListCtrl )
{
	CopyOrCutToClipboard(diskFile, FALSE);
	m_pCutList = pListCtrl;
}

CString CDiskFileManager::GetCopyName( const CString& strDir, const CString& strName, BOOL bIsDir )
{
	WIN32_FIND_DATA FindData; // File information

	int i = 0;
	CString strCopyFileName;

	CString strExt= _T("");
	CString strOnlyName = strName;

	if ( !bIsDir )
	{
		int nToken = strName.ReverseFind(_T('.'));
		if ( nToken != -1 )
		{
			strExt = strName.Right(strName.GetLength()-nToken);
			strOnlyName = strName.Left(nToken);
		}
	}

	while(TRUE)
	{
		strCopyFileName.Format(_M("%s_copy(%d)%s"), strOnlyName, i, strExt);

		if ( FindFirstFile(strDir+strCopyFileName, &FindData) == INVALID_HANDLE_VALUE )
			break;

		i++;
	}

	return strCopyFileName;
}

BOOL CDiskFileManager::CopyFile( const CString& strWorkDir, const CString& strDestFile, const CString& strSrcFullPath )
{
	BOOL bOk = ::CopyFile(strSrcFullPath, strWorkDir+strDestFile, FALSE);
	return m_pOperationDlg->Step(strWorkDir, strDestFile);
}

BOOL CDiskFileManager::CopyDir( const CString& strWorkDir, const CString& strDestDir, const CString& strSrcFullPath )
{
	CreateDirectory(strWorkDir+strDestDir, NULL);

	vector<CString> vecDir, vecFile;

	if ( !GetDirFileList(strSrcFullPath, vecDir, vecFile) )
		return FALSE;

	int nCount = vecFile.size();
	for ( int i = 0; i < nCount; i ++ )
	{
		if ( !CopyFile(strWorkDir+strDestDir+_T("\\"), vecFile[i], strSrcFullPath+vecFile[i]) )
			return FALSE;
	}

	nCount = vecDir.size();
	for ( int i = 0; i < nCount; i ++ )
	{
		if ( !CopyDir(strWorkDir+strDestDir+_T("\\"), vecDir[i], strSrcFullPath+vecDir[i]+_T("\\")) )
			return FALSE;
	}

	return TRUE;
}

HANDLE CDiskFileManager::GetIconFromCache( const CString& strExtName )
{
	if ( m_mapPicture.find(strExtName) != m_mapPicture.end() )
	{
		return m_mapPicture[strExtName];
	}

	HANDLE hPicture = GetIconFromFile(FILE_ATTRIBUTE_NORMAL, _T(".")+strExtName);
	m_mapPicture[strExtName] = hPicture;

	return hPicture;
}

void CDiskFileManager::StartUpdateIconThread( CFileExplorer* pFileExplorer )
{
	CUpdateIconThreadParam* pPara = new CUpdateIconThreadParam;
	pPara->pDiskFileManager = this;
	pPara->pFileExplorer = pFileExplorer;

	AfxBeginThread(UpdateIconThreadFunc, pPara);
}

BOOL CDiskFileManager::IsRefreshDiskDriverThreadActive()
{
	if ( m_hRefreshDiskDriveThreadHandle == 0 )
		return FALSE;

	DWORD dwExitCode;
	GetExitCodeThread(m_hRefreshDiskDriveThreadHandle, &dwExitCode);
	if ( dwExitCode == STILL_ACTIVE )
		return TRUE;

	m_vecDiskDriverInfoLast = m_vecDiskDriverInfo;

	return FALSE;
}

CWinThread* CDiskFileManager::StartRefreshDiskDriverThread()
{
	CWinThread* pRefreshDiskDriveThread = AfxBeginThread(RefreshDiskDriverThreadFunc, (void*)this);
	m_hRefreshDiskDriveThreadHandle = pRefreshDiskDriveThread->m_hThread;
	return pRefreshDiskDriveThread;
}

void CDiskFileManager::DoUpdateIcon(CFileExplorer* pFileExplorer)
{
	CellFormat* pCellFormat;
	CString strFileName, strFileExt;
	HANDLE hPicture;
	CRect rc;

	int nLen = pFileExplorer->m_listFile.GetItemCount();
	for ( int i = 0; i < nLen; i ++ )
	{
		pCellFormat = pFileExplorer->m_listFile.GetCellFormat(i,0);
		if ( pCellFormat->cellType == cellCtrlDirectory )
			continue;

		ASSERT(pCellFormat->cellType == cellCtrlFile);

		strFileName = pFileExplorer->GetCurDir() + pFileExplorer->m_listFile.GetItemText(i,0);

		strFileExt = strFileName.Right(4);
		if ( strFileExt.CompareNoCase(_T(".ico")) == 0 )
		{
			hPicture = ::LoadImage(::GetModuleHandle(NULL), 
				strFileName,IMAGE_ICON,0,0, LR_CREATEDIBSECTION|LR_LOADFROMFILE);
		}
		else if ( strFileExt.CompareNoCase(_T(".exe")) == 0 || strFileExt.CompareNoCase(_T(".lnk")) == 0)
		{
			hPicture = GetIconFromFile(FILE_ATTRIBUTE_NORMAL, strFileName);
		}
		else
		{
			continue;
		}

		pCellFormat->SetPicture(hPicture);

		pFileExplorer->m_listFile.GetSubItemRect(i,0,LVIR_LABEL,rc);
		pFileExplorer->m_listFile.InvalidateRect(rc);
	}
}

void CDiskFileManager::OnUserOption()
{
	UserOptions userOptionOld = m_userOption;
	CUserOptionDialog dlg(&m_userOption);
	if ( dlg.DoModal() != IDOK )
		return;

	if ( userOptionOld.nToolbarButtonSize != m_userOption.nToolbarButtonSize )
	{
		m_toolbarDialog.m_nButtonSize = m_userOption.nToolbarButtonSize;

		CRect rcClient;
		m_pUserWnd->GetClientRect(&rcClient);
		Relayout(rcClient, FALSE, FALSE, FALSE);
	}

	if ( !userOptionOld.bLoadQuickLaunchFromDesktop && m_userOption.bLoadQuickLaunchFromDesktop )
	{
		LoadQuickLaunchFromDesktop();
	}

	if ( !userOptionOld.bLoadQuickLaunchFromTaskbar && m_userOption.bLoadQuickLaunchFromTaskbar )
	{
		LoadQuickLaunchFromWindowDefault();
	}

	if ( userOptionOld.bLoadQuickLaunchFromDesktop && !m_userOption.bLoadQuickLaunchFromDesktop )
	{
		DeleteQuickLaunchFromDesktop();
	}

	if ( userOptionOld.bLoadQuickLaunchFromTaskbar && !m_userOption.bLoadQuickLaunchFromTaskbar )
	{
		DeleteQuickLaunchFromWindowDefault();
	}

	if ( m_userOption.nRowHeight != userOptionOld.nRowHeight )
	{
		OnListStyleChanged();
	}

	if ( m_userOption.strOddLineBKColorActive != userOptionOld.strOddLineBKColorActive || 
		 m_userOption.strEvenLineBKColorActive != userOptionOld.strEvenLineBKColorActive ||
		 m_userOption.strOddLineBKColorInactive != userOptionOld.strOddLineBKColorInactive ||
		 m_userOption.strEvenLineBKColorInactive != userOptionOld.strEvenLineBKColorInactive )
	{
		OnListBackGroundColorChanged();
	}

	if ( userOptionOld.strLanguage != m_userOption.strLanguage )
	{
		CMultiLanguage::SetLanguage(m_userOption.strLanguage == _T("English"));
	
		CDiskFilePresenter* pPresenter;
		map<CListCtrl*, CDiskFilePresenter*>::iterator it;
		for ( it = m_mapPresenter.begin(); it != m_mapPresenter.end(); it++ )
		{
			pPresenter = it->second;
			pPresenter->pFileExplorer->OnLanguageChanged();
		}
	}

	if ( userOptionOld.bShowDirTree != m_userOption.bShowDirTree || userOptionOld.nDirTreeDlgWidth != m_userOption.nDirTreeDlgWidth )
	{
		CRect rcClient;
		m_pUserWnd->GetClientRect(&rcClient);
		Relayout(rcClient, FALSE, FALSE, FALSE);
	}
	RebuildBitmapMap();
}

BOOL CDiskFileManager::OnListStyleChanged()
{
	CDiskFilePresenter* pPresenter;

	map<CListCtrl*, CDiskFilePresenter*>::iterator it;
	for ( it = m_mapPresenter.begin(); it != m_mapPresenter.end(); it++ )
	{
		pPresenter = it->second;
		pPresenter->pFileExplorer->m_listFile.SetHeaderHeight(m_userOption.nRowHeight);
		pPresenter->pFileExplorer->m_listFile.SetRowHeigt(m_userOption.nRowHeight);
	}

	return TRUE;
}

BOOL CDiskFileManager::OnExport()
{
	CFileDialog dlg(FALSE, "ini", m_defaultDir.strWorkPath + m_strAppName + _T(".ini"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Ini Files (*.ini)|*.ini||" );
	if ( dlg.DoModal() != IDOK )
		return FALSE;

	CString strFileName = dlg.GetPathName();
	ExportIniFile(strFileName, FALSE);

	CString strMsg;
	strMsg.Format(_M("Export to file %s successfully!"), strFileName);
	AfxMessageBox(strMsg);

	return TRUE;
}

BOOL CDiskFileManager::OnImport()
{
	CFileDialog dlg(TRUE, "ini", m_defaultDir.strWorkPath + m_strAppName + _T(".ini"), NULL,
		"Ini Files (*.ini)|*.ini||" );
	if ( dlg.DoModal() != IDOK )
		return FALSE;

	DistroyAllCell();
	m_toolbarDialog.DeleteQuickLaunchButtons();

	CRect rcClient;
	m_pUserWnd->GetClientRect(&rcClient);
	CString strFileName = dlg.GetPathName();
	if ( LoadIniFile(strFileName,FALSE) )
		return Relayout(rcClient, FALSE, FALSE, FALSE);

	return FALSE;
}

BOOL CDiskFileManager::OnExtractFile( const CString& strFile, const CString& strDir )
{
	CreateDirectory(strDir, NULL);

	CString strRunPara;
	strRunPara.Format(_T("x \"%s\" \"%s\""), strFile, strDir);
	ShellExecute(NULL, _T("Open"), m_userOption.strWinRarPath, strRunPara, _T(""), SW_SHOW );

	return TRUE;
}

BOOL CDiskFileManager::OnCreateRarFile( const CString& strFile, const CDiskFile& diskFile )
{
	CString strRunPara;
	strRunPara.Format(_T(" a \"%s\" "), strFile);

	CString strItem;

	int nCount = diskFile.vecDirectory.size();
	for ( int i = 0; i < nCount; i ++ )
	{
		strItem.Format(_T("\"%s\" "), /*diskFile.strWorkDir+*/diskFile.vecDirectory[i]);
		strRunPara += strItem;
	}

	nCount = diskFile.vecFile.size();
	for ( int i = 0; i < nCount; i ++ )
	{
		strItem.Format(_T("\"%s\" "), /*diskFile.strWorkDir+*/diskFile.vecFile[i]);
		strRunPara += strItem;
	}

	CString strCmdLine = _T("\"") + m_userOption.strWinRarPath + _T("\"") + strRunPara;
	m_commonTool.RunProgram(strCmdLine.GetBuffer(0), (LPTSTR)(LPCTSTR)diskFile.strWorkDir, SW_SHOWNORMAL, IGNORE);

	return TRUE;
}

void CDiskFileManager::OnOpenMultiFile( const CDiskFile& diskFile, const CString& strOpener )
{
	CString strRunPara;

	int	nCount = diskFile.vecFile.size();
	for ( int i = 0; i < nCount; i ++ )
	{
		strRunPara += _T(" \"") + diskFile.strWorkDir+diskFile.vecFile[i] + _T("\"");
	}

	ShellExecute(NULL, _T("Open"), strOpener, strRunPara, _T(""), SW_SHOW );
}

void CDiskFileManager::OnRunFileInDosCmd( const CString strFile, const CString& strDir )
{
	CString strCommand;
	strCommand.Format(_T("/k \"cd /d %s & \"%s\"\""), strDir, strFile);

	ShellExecute(NULL, _T("Open"), _T("cmd"), strCommand, _T(""), SW_SHOW );	
}

void CDiskFileManager::DoRelayout( const CRect& rcClient,const CString& strLayout, BOOL bAdjustListColumn )
{
	CRect rcToolbar = rcClient;
	rcToolbar.top += 2;
	rcToolbar.left += 2;
	rcToolbar.right -= 2;
	rcToolbar.bottom = rcToolbar.top + m_toolbarDialog.m_nButtonSize+2;
	m_toolbarDialog.SetPosition(rcToolbar);

	CRect rcFavoriteDir = rcClient;
	rcFavoriteDir.top += m_toolbarDialog.m_nButtonSize+2;
	rcFavoriteDir.right = rcFavoriteDir.left;
	if ( m_userOption.bShowDirTree )
	{
		rcFavoriteDir.right = rcFavoriteDir.left +  m_userOption.nDirTreeDlgWidth;
		rcFavoriteDir.bottom = 300;
		m_faviouriteDirDialog.SetPosition(rcFavoriteDir);

		CRect rcDirTree = rcFavoriteDir;
		rcDirTree.top = rcFavoriteDir.bottom + 2;
		rcDirTree.bottom = rcClient.bottom;
		m_dirTreeDialog.SetPosition(rcDirTree);
	}
	else
	{
		m_dirTreeDialog.ShowWindow(SW_HIDE);
	}

	CRect rcExplorers = rcClient;
	rcExplorers.top = rcFavoriteDir.top;
	rcExplorers.left = rcFavoriteDir.right;
	
	m_layoutManager.CreateRectListFromString(rcExplorers, m_userOption.strLayout, m_vecCurrentLayout);

	map<CListCtrl*, CDiskFilePresenter*>::iterator it;
	for ( it = m_mapPresenter.begin(); it != m_mapPresenter.end(); it++ )
	{
		CFileExplorer* pFileExplorer = it->second->pFileExplorer;

		pFileExplorer->SetPosition(m_vecCurrentLayout[pFileExplorer->m_nLayoutIndex-1], m_mapPresenter.size()>1);
		if ( bAdjustListColumn )
		{
			pFileExplorer->AdjustListColumn();
		}
		pFileExplorer->Invalidate();
		pFileExplorer->ShowWindow(SW_SHOW);
	}
}

BOOL CDiskFileManager::OnCreateNewDirectory( CFileExplorer* pFileExplorer )
{
	CDirectoryInfo di;

	CString strCurDir = pFileExplorer->GetCurDir();

	di.strName = GetDefaultNewName(strCurDir, _M("New Folder"), _T(""));
	CreateDirectory(strCurDir+di.strName, NULL);

	WIN32_FIND_DATA FindData; // File information

	HANDLE hFind = FindFirstFile ( strCurDir+di.strName, &FindData);
	ASSERT( hFind != INVALID_HANDLE_VALUE );
	FindClose(hFind);

	COleDateTime dt(FindData.ftLastWriteTime);
	di.strDate.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), dt.GetYear(), dt.GetMonth(), dt.GetDay(),
		dt.GetHour(), dt.GetMinute(), dt.GetSecond());

	int nRow = pFileExplorer->AppendDir(di);
	CCustomListCtrl* pList = &pFileExplorer->m_listFile;

	if ( m_userOption.bShowDirTree )
	{
		m_dirTreeDialog.m_pDirTree->PopulateTree();
		m_dirTreeDialog.m_pDirTree->TunnelTree(strCurDir);
	}


	pList->EnsureVisible(nRow, FALSE);
	pList->UpdateWindow();
	pList->StartCellEdit(nRow,0);

	return TRUE;
}

BOOL CDiskFileManager::OnCreateNewTextFile( CFileExplorer* pFileExplorer )
{
	CFileInfo fi;

	CString strCurDir = pFileExplorer->GetCurDir();

	fi.strName = GetDefaultNewName(strCurDir, _M("New Text File"), _T(".txt"));
	HANDLE hFile = CreateFile(strCurDir+fi.strName, GENERIC_WRITE, FILE_SHARE_READ, 
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
	CloseHandle(hFile);

	WIN32_FIND_DATA FindData; // File information

	HANDLE hFind = FindFirstFile ( strCurDir+fi.strName, &FindData);
	ASSERT( hFind != INVALID_HANDLE_VALUE );
	FindClose(hFind);

	COleDateTime dt(FindData.ftLastWriteTime);
	fi.strDate.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), dt.GetYear(), dt.GetMonth(), dt.GetDay(),
		dt.GetHour(), dt.GetMinute(), dt.GetSecond());

	fi.nSize = 0;
	fi.strExt = _T("txt");

	int nRow = pFileExplorer->AppendFile(fi);
	CCustomListCtrl* pList = &pFileExplorer->m_listFile;

	pList->EnsureVisible(nRow, FALSE);
	pList->UpdateWindow();
	pList->StartCellEdit(nRow,0);

	return TRUE;
}

CString CDiskFileManager::GetDefaultNewName( const CString& strDir, const CString& strDefaultName, const CString& strDefaultExt )
{
	WIN32_FIND_DATA FindData; // File information
	CString strDefaultNewName = strDefaultName+strDefaultExt;
	HANDLE hFind = FindFirstFile(strDir+strDefaultNewName, &FindData);
	if ( hFind == INVALID_HANDLE_VALUE )
	{
		FindClose(hFind);
		return strDefaultNewName;
	}

	int i = 1;
	while(TRUE)
	{
		strDefaultNewName.Format(_M("%s(%d)%s"), strDefaultName, i, strDefaultExt);
		if ( FindFirstFile(strDir+strDefaultNewName, &FindData) == INVALID_HANDLE_VALUE )
			break;
		i++;
	}

	return strDefaultNewName;
}

CString CDiskFileManager::GetCurrentFormatTime()
{
	CString strTime;

	COleDateTime dt = COleDateTime::GetCurrentTime();
	strTime.Format(_T("%04d-%02d-%02d %02d:%02d:%02d"), dt.GetYear(), dt.GetMonth(), dt.GetDay(),
		dt.GetHour(), dt.GetMinute(), dt.GetSecond());

	return strTime;
}

void CDiskFileManager::OnAddToToolbar( const CDiskFile& diskFile )
{
	int nCount = diskFile.vecFile.size();
	for ( int i = 0; i < nCount; i ++ )
	{
		m_toolbarDialog.AppendQuickLaunchButton(diskFile.strWorkDir+diskFile.vecFile[i], TRUE);
	}
}

void CDiskFileManager::InitDefaultDirs()
{
	char	strPath[MAX_PATH];

	GetWindowsDirectory(strPath, MAX_PATH);
	m_defaultDir.strWindowsDir = strPath;
	m_defaultDir.strWindowsDir += _T("\\");

	GetTempPath(MAX_PATH, strPath);
	m_defaultDir.strTempDir = strPath;

	GetCurrentDirectory(MAX_PATH, strPath);
	m_defaultDir.strWorkPath = strPath;
	if ( m_defaultDir.strWorkPath.GetAt(m_defaultDir.strWorkPath.GetLength()-1) != _T('\\'))
		m_defaultDir.strWorkPath += _T("\\");

	m_defaultDir.strAllUserDesktopDir = GetSpecialFolder(CSIDL_COMMON_DESKTOPDIRECTORY);
	m_defaultDir.strDesktopDir = GetSpecialFolder(CSIDL_DESKTOPDIRECTORY);
	m_defaultDir.strMyDocumentsDir = GetSpecialFolder(CSIDL_MYDOCUMENTS);
	m_defaultDir.strAppDataDir = GetSpecialFolder(CSIDL_APPDATA);
}

void CDiskFileManager::InitDefaultBitmaps()
{
	m_pBitmapExplorer = GetBitmapFromIcon((HICON)GetIconFromFile(FILE_ATTRIBUTE_DIRECTORY, _T("folder")));
	m_pBitmapCmd = GetBitmapFromFile(m_defaultDir.strWindowsDir + _T("system32\\cmd.exe"));
	m_pBitmapNotepad = GetBitmapFromFile(m_defaultDir.strWindowsDir + _T("Notepad.exe"));
	m_pBitmapMultiExplorer = GetBitmapFromIcon((HICON)::LoadImage(::GetModuleHandle(NULL), 
		MAKEINTRESOURCE(128),IMAGE_ICON,0,0, LR_CREATEDIBSECTION));
}

CString CDiskFileManager::GetSpecialFolder( UINT uFolderID )
{
	CString strFolder;
	char	strPath[MAX_PATH];

	LPITEMIDLIST pidl;
	LPMALLOC pShellMalloc;

	if ( SUCCEEDED(SHGetMalloc(&pShellMalloc)))
	{
		if ( SUCCEEDED(SHGetSpecialFolderLocation(NULL, uFolderID, &pidl)))
		{
			if ( SHGetPathFromIDList(pidl, strPath) )
			{
				strFolder = strPath;
			}

			pShellMalloc->Free(pidl);
		}

		pShellMalloc->Release();
	}

	if ( strFolder.Right(1) != _T("\\") )
		strFolder += _T("\\");

	return strFolder;
}

BOOL CDiskFileManager::OnPaste( CDiskFile& diskFile, const CString& strDestDir, CFileExplorer* pFileExplorer, BOOL bCut )
{
	int nCount = GetFileCount(diskFile);

	m_pOperationDlg = new COperationDialog;
	m_pOperationDlg->SetOperatioinType(operationPaste);
	m_pOperationDlg->Show(0, nCount);

	BOOL bFailed = FALSE;
	BOOL bIsSameDir = (strDestDir == diskFile.strWorkDir);

	CString strName;

	nCount = diskFile.vecDirectory.size();
	for ( int i = 0; i < nCount; i ++ )
	{
		strName = diskFile.vecDirectory[i];
		if ( bIsSameDir)
			strName = GetCopyName(strDestDir, strName, TRUE);

		CopyDir(strDestDir, strName, diskFile.strWorkDir+diskFile.vecDirectory[i]+_T("\\") );
	}

	nCount = diskFile.vecFile.size();
	for ( int i = 0; i < nCount; i ++ )
	{
		strName = diskFile.vecFile[i];
		if ( bIsSameDir )
			strName = GetCopyName(strDestDir, strName, FALSE);

		CopyFile(strDestDir, strName, diskFile.strWorkDir+diskFile.vecFile[i]);
	}


	m_pOperationDlg->DestroyWindow();
	delete m_pOperationDlg;

	if ( !bFailed )
	{
		pFileExplorer->OnBnClickedButtonRefresh();
	}
	else
	{
		return FALSE;
	}

	if ( bCut )
	{
		CFileExplorer*	pCutFileExplorer = NULL;
		if ( m_mapPresenter.find( m_pCutList ) != m_mapPresenter.end() )
		{
			pCutFileExplorer =  m_mapPresenter[m_pCutList]->pFileExplorer;
		}

		OnDelete(diskFile, pCutFileExplorer, FALSE);

		if( OpenClipboard(NULL) )
		{
			EmptyClipboard();
			CloseClipboard();
		}
	}

	return TRUE;

}

BOOL CDiskFileManager::CheckCopyFromClipboard( CDiskFile& diskFile, BOOL& bCut )
{
	UINT uDropEffect=RegisterClipboardFormat("Preferred DropEffect");
	UINT cFiles=0;
	DWORD dwEffect,*dw;

	if( OpenClipboard( NULL)) 
	{
		HDROP hDrop = HDROP( GetClipboardData( CF_HDROP));
		if( hDrop) 
		{
			dw=(DWORD*)(GetClipboardData( uDropEffect));
			if(dw==NULL)
				dwEffect=DROPEFFECT_COPY;
			else
				dwEffect=*dw;

			cFiles = DragQueryFile( hDrop, (UINT) -1, NULL, 0);
			char szFile[ MAX_PATH];
			for( UINT count = 0; count < cFiles; count++ ) 
			{
				DragQueryFile( hDrop, count, szFile, sizeof( szFile));
				if ( !AddToDiskFile(diskFile, szFile) )
				{
					CloseClipboard();
					return FALSE;
				}
			}
		}

		CloseClipboard();

		if ( !hDrop )
			return FALSE;

		if(dwEffect & DROPEFFECT_MOVE) 
		{
			bCut = TRUE;
		} 
		else if(dwEffect & DROPEFFECT_COPY) 
		{
			bCut = FALSE;
		}
		else
		{
			return FALSE;
		}
	}

	return (cFiles!=0);
}

BOOL CDiskFileManager::CopyOrCutToClipboard( const CDiskFile& diskFile, BOOL bCopy )
{
	TCHAR szFiles[409600];
	UINT uBufLen;
	ConvertToClipboardFormat(diskFile, szFiles, uBufLen);


	UINT uDropEffect;
	DROPFILES dropFiles;
	UINT uGblLen,uDropFilesLen;
	HGLOBAL hGblFiles,hGblEffect;
	char *szData,*szFileList;

	DWORD *dwDropEffect;

	uDropEffect=RegisterClipboardFormat("Preferred DropEffect");
	hGblEffect=GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE,sizeof(DWORD));
	dwDropEffect=(DWORD*)GlobalLock(hGblEffect);
	if(bCopy)
		*dwDropEffect=DROPEFFECT_COPY;
	else 
		*dwDropEffect=DROPEFFECT_MOVE;
	GlobalUnlock(hGblEffect);

	uDropFilesLen=sizeof(DROPFILES);
	dropFiles.pFiles =uDropFilesLen;
	dropFiles.pt.x=0;
	dropFiles.pt.y=0;
	dropFiles.fNC =FALSE;
	dropFiles.fWide =TRUE;

	uGblLen=uDropFilesLen+uBufLen*2+8;
	hGblFiles= GlobalAlloc(GMEM_ZEROINIT|GMEM_MOVEABLE|GMEM_DDESHARE, uGblLen);
	szData=(char*)GlobalLock(hGblFiles);
	memcpy(szData,(LPVOID)(&dropFiles),uDropFilesLen);
	szFileList=szData+uDropFilesLen;

	MultiByteToWideChar(CP_ACP,MB_COMPOSITE,
		szFiles,uBufLen,(WCHAR *)szFileList,uBufLen);

	GlobalUnlock(hGblFiles);

	if( OpenClipboard(NULL) )
	{
		EmptyClipboard();
		SetClipboardData( CF_HDROP, hGblFiles );
		SetClipboardData(uDropEffect,hGblEffect);
		CloseClipboard();
	}

	return TRUE;
}

BOOL CDiskFileManager::ConvertToClipboardFormat( const CDiskFile& diskFile, TCHAR* szFiles, UINT& uBufLen )
{
	CString strBuffer;
	int nCount = diskFile.vecDirectory.size();
	for ( int i = 0; i < nCount; i ++ )
	{
		strBuffer += diskFile.strWorkDir + diskFile.vecDirectory[i] + _T("\r\n");
	}
	nCount = diskFile.vecFile.size();
	for ( int i = 0; i < nCount; i ++ )
	{
		strBuffer += diskFile.strWorkDir + diskFile.vecFile[i] + _T("\r\n");
	}


	TCHAR *szBuffer,*lp,*lpStart,*lpFileStart;
	UINT uLen,uTotalLen=0;

	memset(szFiles,0,4096*sizeof(TCHAR));

	szBuffer = strBuffer.GetBuffer(0);
	if(*szBuffer!=0) 
	{
		memset(szFiles,0,4096*sizeof(TCHAR));
		lpFileStart=szFiles;
		lpStart=szBuffer;
		lp=szBuffer;
		while(*lp!=0) 
		{
			if(*lp=='\r') 
			{ 
				*lp=0;
				lstrcat(lpFileStart,lpStart);
				uLen=lstrlen(lpStart)+1;
				lpFileStart+=uLen;
				uTotalLen+=uLen;
				lp++;
				lpStart=lp+1;
			}
			lp++;
		}
	}

	uBufLen = uTotalLen;

	return TRUE;

}

BOOL CDiskFileManager::AddToDiskFile( CDiskFile& diskFile, const CString& strName )
{
	if ( !IsDriverAlive(strName.Left(3)))
		return FALSE;

	WIN32_FIND_DATA FindData; // File information
	HANDLE hFind = FindFirstFile ( strName, &FindData);
	BOOL bOk = ( hFind != INVALID_HANDLE_VALUE );

	if ( !bOk )
		return FALSE;

	int nToken = strName.ReverseFind(_T('\\'));
	CString strWorkDir = strName.Left(nToken+1);
	CString strOnlyName = strName.Right(strName.GetLength()-nToken-1);

	if (diskFile.strWorkDir.IsEmpty() )
	{
		diskFile.strWorkDir = strWorkDir;
	}
	else
	{
		ASSERT(diskFile.strWorkDir.CompareNoCase(strWorkDir) == 0 );
	}

	if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
		diskFile.vecDirectory.push_back(strOnlyName);
	else
		diskFile.vecFile.push_back(strOnlyName);

	return TRUE;
}

BOOL CDiskFileManager::GetDiskUsageInfo(LPCTSTR szDirectory, __int64& i64Used, __int64& i64Total)
{
	ULARGE_INTEGER bytesTotal, bytesAvailableToUser, bytesAvailable;
	
	if ( GetDiskFreeSpaceEx(szDirectory, &bytesAvailableToUser, &bytesTotal, &bytesAvailable) )
	{
		i64Used = bytesTotal.QuadPart - bytesAvailable.QuadPart;
		i64Total = bytesTotal.QuadPart;
		return TRUE;
	}
	else
	{
		i64Used = 0;
		i64Total = 1;
		return FALSE;
	}
}

void CDiskFileManager::GetDriverInfo( LPCTSTR szDriver, CDiskDriverInfo& di, BOOL bNotGetCapacityAndLabel )
{
	di.strDriverLetter = szDriver;

	di.nDriverType = GetDriveType(szDriver);
	di.strLabel = _T("");
	di.nUsedSpace = 0;
	di.nTotalSpace = 1000;

	di.bAlive = TRUE;
	if ( di.nDriverType != DRIVE_CDROM && !bNotGetCapacityAndLabel)
	{
		di.bAlive = GetDiskUsageInfo(di.strDriverLetter, di.nUsedSpace, di.nTotalSpace);

		TCHAR lpVolumeNameBuffer[200];
		DWORD dwVolumeSerialNumber, dwMaximumComponentLength;
		DWORD dwFileSystemFlags;
		TCHAR lpFileSystemNameBuffer[50];

		BOOL bOk = GetVolumeInformation ( szDriver,
			lpVolumeNameBuffer, 200,
			&dwVolumeSerialNumber,
			&dwMaximumComponentLength, 
			&dwFileSystemFlags,
			lpFileSystemNameBuffer, 50 );

		if ( bOk )
			di.strLabel = lpVolumeNameBuffer;
	}
}

void CDiskFileManager::LoadQuickLaunchFromDir( const CString& strDir )
{
	vector<CDirectoryInfo> vecDir;
	vector<CFileInfo> vecFile;
	GetDirFileList(strDir, vecDir, vecFile, TRUE);
	for ( int i = 0; i < (int)vecFile.size(); i ++ )
	{
		if ( vecFile[i].strExt.CompareNoCase(_T("lnk")) == 0 ||
			vecFile[i].strExt.CompareNoCase(_T("exe")) == 0)
			m_toolbarDialog.AppendQuickLaunchButton(strDir+vecFile[i].strName, FALSE);
	}
}

void CDiskFileManager::LoadQuickLaunchFromWindowDefault()
{
	CString strDir;
	strDir = m_defaultDir.strAppDataDir + _T("Microsoft\\Internet Explorer\\Quick Launch\\");
	LoadQuickLaunchFromDir(strDir);

	strDir += _T("User Pinned\\TaskBar\\");
	LoadQuickLaunchFromDir(strDir);
}

void CDiskFileManager::DeleteQuickLaunchFromWindowDefault()
{
	CString strDir;
	strDir = m_defaultDir.strAppDataDir + _T("Microsoft\\Internet Explorer\\Quick Launch\\");
	DeleteQuickLaunchFromDir(strDir);

	strDir += _T("User Pinned\\TaskBar\\");
	DeleteQuickLaunchFromDir(strDir);
}

void CDiskFileManager::DeleteQuickLaunchFromDir( const CString& strDir )
{
	vector<CDirectoryInfo> vecDir;
	vector<CFileInfo> vecFile;
	GetDirFileList(strDir, vecDir, vecFile, TRUE);
	for ( int i = 0; i < (int)vecFile.size(); i ++ )
	{
		if ( vecFile[i].strExt.CompareNoCase(_T("lnk")) == 0 ||
			vecFile[i].strExt.CompareNoCase(_T("exe")) == 0)
			m_toolbarDialog.DeleteButton(strDir+vecFile[i].strName);
	}
}

void CDiskFileManager::LoadQuickLaunchFromDesktop()
{
	LoadQuickLaunchFromDir(m_defaultDir.strAllUserDesktopDir);
	LoadQuickLaunchFromDir(m_defaultDir.strDesktopDir);
}

void CDiskFileManager::DeleteQuickLaunchFromDesktop()
{
	DeleteQuickLaunchFromDir(m_defaultDir.strAllUserDesktopDir);
	DeleteQuickLaunchFromDir(m_defaultDir.strDesktopDir);
}

CString CDiskFileManager::GetAppName()
{
	char	szName[MAX_PATH];
	GetModuleFileName(NULL, szName, MAX_PATH );

	CString strName = szName;
	int nToken = strName.ReverseFind(_T('\\'));
	CString strOnlyName = strName.Right(strName.GetLength()-nToken-1);

	nToken = strOnlyName.ReverseFind(_T('.'));

	return strOnlyName.Left(nToken);
}

void CDiskFileManager::RebuildBitmapMap()
{
	for ( map<CString,CBitmap*>::iterator it = m_mapBitmap.begin(); it != m_mapBitmap.end(); it ++ )
	{
		CBitmap* pBitmap = it->second;
		if ( pBitmap )
			pBitmap->DeleteObject();
	}

	m_mapBitmap.clear();

	m_mapBitmap[m_userOption.strInternetExplorer] = GetBitmapFromFile(m_userOption.strInternetExplorer);
	m_mapBitmap[m_userOption.strBeyondComparePath] = GetBitmapFromFile(m_userOption.strBeyondComparePath);
	m_mapBitmap[m_userOption.strUltraEditPath] = GetBitmapFromFile(m_userOption.strUltraEditPath);
	m_mapBitmap[m_userOption.strNotepadPPPath] = GetBitmapFromFile(m_userOption.strNotepadPPPath);
	m_mapBitmap[m_userOption.strWinRarPath] = GetBitmapFromFile(m_userOption.strWinRarPath);
}

void CDiskFileManager::DoExit(BOOL bSave)
{
	if ( bSave)
	{
		UpdateIniFileInResource();
	}

	DistroyAllCell();
	CMultiLanguage::Clear();

	//::TerminateThread(m_hRefreshDiskDriveThreadHandle, 0);
	//CloseHandle(m_hRefreshDiskDriveThreadHandle);
}

BOOL CDiskFileManager::IsDriverAlive( const CString& strDriverLetter )
{
	int nCount = (int)m_vecDiskDriverInfoLast.size();
	for ( int i = 0; i < nCount; i ++ )
	{
		if ( m_vecDiskDriverInfoLast[i].strDriverLetter.CompareNoCase(strDriverLetter) == 0 )
			return m_vecDiskDriverInfoLast[i].bAlive;
	}

	return FALSE;
}

void CDiskFileManager::SetActiveFileExplorer(CFileExplorer* pFileExplorer)
{
	if ( m_pActiveFileExplorer == pFileExplorer )
		return;

	ASSERT(pFileExplorer);

	if ( m_pActiveFileExplorer )
	{
		SetFileExplorerBackground(m_pActiveFileExplorer, FALSE);
	}

	m_pActiveFileExplorer = pFileExplorer;
	SetFileExplorerBackground(m_pActiveFileExplorer, TRUE);

	if ( m_userOption.bShowDirTree && m_dirTreeDialog.m_hWnd != NULL )
	{
		m_dirTreeDialog.m_pDirTree->TunnelTree(m_pActiveFileExplorer->m_strCurDir);
	}
}

void CDiskFileManager::SetFileExplorerBackground( CFileExplorer* pFileExplorer, BOOL bActive )
{
	pFileExplorer->m_listFile.m_vecRowColors.clear();

	if ( bActive )
	{
		pFileExplorer->m_listFile.AddDefaultRowColor(m_userOption.strOddLineBKColorActive);
		pFileExplorer->m_listFile.AddDefaultRowColor(m_userOption.strEvenLineBKColorActive);
	}
	else
	{
		pFileExplorer->m_listFile.AddDefaultRowColor(m_userOption.strOddLineBKColorInactive);
		pFileExplorer->m_listFile.AddDefaultRowColor(m_userOption.strEvenLineBKColorInactive);
	}

	pFileExplorer->m_listFile.Invalidate();
}

void CDiskFileManager::SwitchWithActiveFileExplorer( CFileExplorer* pFileExplorer )
{
	if ( m_pActiveFileExplorer == NULL )
	{
		SetActiveFileExplorer(pFileExplorer);
		return;
	}

	if ( pFileExplorer == m_pActiveFileExplorer )
		return;

	int nLayoutIndexActive = m_pActiveFileExplorer->m_nLayoutIndex;

	m_pActiveFileExplorer->m_nLayoutIndex = pFileExplorer->m_nLayoutIndex;
	m_pActiveFileExplorer->SetPosition(m_vecCurrentLayout[m_pActiveFileExplorer->m_nLayoutIndex-1], TRUE);
	m_pActiveFileExplorer->AdjustListColumn();

	pFileExplorer->m_nLayoutIndex = nLayoutIndexActive;
	pFileExplorer->SetPosition(m_vecCurrentLayout[pFileExplorer->m_nLayoutIndex-1], TRUE);
	pFileExplorer->AdjustListColumn();
}

BOOL CDiskFileManager::OnSaveAs()
{
	CFileDialog dlg(FALSE, "exe", m_defaultDir.strWorkPath + m_strAppName + _T(".exe"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		"Exe Files (*.exe)|*.exe||" );
	if ( dlg.DoModal() != IDOK )
		return FALSE;

	char strFileName[MAX_PATH];
	GetModuleFileName ( NULL, strFileName, MAX_PATH );

	CString strSaveAsName = dlg.GetPathName();
	::CopyFile(strFileName, strSaveAsName, FALSE);

	CString strCmdLine = "\"" + m_strRumCmdFile + "\" 2*";
	strCmdLine += strSaveAsName;
	strCmdLine += "*IDR_EXE*134*";
	strCmdLine += m_strIniFile;

	ExportIniFile(m_strIniFile, FALSE);

	m_commonTool.OutFile( IDR_EXE_RUN_CMD, "IDR_EXE", m_strRumCmdFile.GetBuffer(0));
	m_commonTool.RunProgram(strCmdLine.GetBuffer(0), "C:\\", SW_HIDE, INFINITE);


	CString strMsg;
	strMsg.Format(_M("Export to file %s successfully!"), strSaveAsName);
	AfxMessageBox(strMsg);

	return TRUE;
}

BOOL CDiskFileManager::StartNewMultiExplorer( const CString& strDir )
{
	CString strIniFile = m_defaultDir.strTempDir + _T("MultiExplorerTemp.ini");

	ExportIniFile(strIniFile, FALSE);
	
	m_iniFile.SetValue ( "General", "Layout", _T("1X1;0") );
	m_iniFile.SetValue("Explorers", "ExplorerCount", 1);
	m_iniFile.SetValue ( "Explorers", "ActiveExplorerIndex", 1 );

	CString strTemp, strKey;
	strTemp = _T("Explorer01");


	vector<int> vecColumnWidth;
	vecColumnWidth.push_back(600);
	vecColumnWidth.push_back(80);
	vecColumnWidth.push_back(50);
	vecColumnWidth.push_back(120);

	for ( int j = 0; j < 4; j ++ )
	{
		strKey.Format(_T("%s.ColumnWidth%d"), strTemp, j);
		m_iniFile.SetValue ( "Explorers", (const char*)strKey, vecColumnWidth[j] );
	}

	strKey = strTemp + _T(".WorkDir");
	m_iniFile.SetValue ( "Explorers", (const char*)strKey, strDir );

	strKey = strTemp + _T(".Filter");
	m_iniFile.SetValue( "Explorers", (const char*)strKey, _T("") );

	strKey = strTemp + _T(".SortColumn");
	m_iniFile.SetValue( "Explorers", (const char*)strKey, 0 );

	strKey = strTemp + _T(".SortAscending");
	m_iniFile.SetValue( "Explorers", (const char*)strKey, 1 );

	m_iniFile.SetValue("Network", "LocalListeningPort", -1);

	m_iniFile.Write(strIniFile, FALSE);

	char strFileName[MAX_PATH];
	GetModuleFileName ( NULL, strFileName, MAX_PATH );

	OnOpenFile(strIniFile, strFileName);

	return TRUE;
}

BOOL CDiskFileManager::OnListBackGroundColorChanged()
{
	CFileExplorer* pFileExplorer;

	map<CListCtrl*, CDiskFilePresenter*>::iterator it;
	for ( it = m_mapPresenter.begin(); it != m_mapPresenter.end(); it++ )
	{
		pFileExplorer = it->second->pFileExplorer;

		SetFileExplorerBackground(pFileExplorer, pFileExplorer==m_pActiveFileExplorer);
	}

	return TRUE;
}

BOOL CDiskFileManager::OnShellFileOperation( CDiskFile& diskFile, const CString& strDestDir, CFileExplorer* pFileExplorer, UINT uOperation )
{
	SHFILEOPSTRUCT ShFileOp = {0};

	TCHAR szFiles[409600];
	UINT uBufLen;
	ConvertToClipboardFormat(diskFile, szFiles, uBufLen);

	ShFileOp.hwnd = m_pUserWnd->m_hWnd;
	ShFileOp.wFunc = uOperation;
	ShFileOp.pFrom = szFiles;
	ShFileOp.pTo = strDestDir;

	if ( diskFile.strWorkDir == strDestDir )
		ShFileOp.fFlags = FOF_RENAMEONCOLLISION;

	pFileExplorer->EnableClose(FALSE);
	SHFileOperation(&ShFileOp);
	pFileExplorer->EnableClose(TRUE);
	pFileExplorer->OnBnClickedButtonRefresh();

	if ( uOperation == FO_MOVE )
	{
		CFileExplorer*	pCutFileExplorer = NULL;
		if ( m_mapPresenter.find( m_pCutList ) != m_mapPresenter.end() )
		{
			pCutFileExplorer =  m_mapPresenter[m_pCutList]->pFileExplorer;
		}

		if ( pCutFileExplorer )
		{
			pCutFileExplorer->OnBnClickedButtonRefresh();
		}

		if( OpenClipboard(NULL) )
		{
			EmptyClipboard();
			CloseClipboard();
		}
	}

	return TRUE;
}

void CDiskFileManager::ShowDirInActiveFileExplorer( const CString& strDir )
{
	if ( m_pActiveFileExplorer && !strDir.IsEmpty() )
	{
		if ( m_pActiveFileExplorer->m_listFile.m_ctrlHeader.m_bEnableSort )
			m_pActiveFileExplorer->ShowDir(strDir, _T(""), TRUE, FALSE);
	}
}

void CDiskFileManager::ShowDirInFirstFileExplorer( const CString& strDir )
{
	m_mapPresenter.begin()->second->pFileExplorer->ShowDir(strDir, _T(""), TRUE, FALSE);
}

void CDiskFileManager::SetTitleWithNetworkInfo()
{
	// Get host name
	WORD wVersionRequested;
	WSADATA wsaData;
	int err; 
	wVersionRequested = MAKEWORD( 2, 2 );
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) 
	{
		return;
	}

	TCHAR szHostName[MAX_HOSTNAME_LEN];
	if (gethostname((char FAR*)szHostName, MAX_HOSTNAME_LEN) == SOCKET_ERROR)
	{
		return;
	}

	CString strIconPath;

	// Get ipaddress and subnet mask
	CString strIPAddress, strIPMask;
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL; 
	DWORD dwRetVal = 0; 
	pAdapterInfo = ( IP_ADAPTER_INFO * ) malloc( sizeof( IP_ADAPTER_INFO ) );
	ULONG ulOutBufLen; 

	ulOutBufLen = sizeof(IP_ADAPTER_INFO); 

	if (GetAdaptersInfo( pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) 
	{ 
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO *) malloc (ulOutBufLen); 
	} 

	if ((dwRetVal = GetAdaptersInfo( pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
	{ 
		pAdapter = pAdapterInfo; 
		while (pAdapter) 
		{ 
			CString strValue = CA2W(pAdapter->IpAddressList.IpAddress.String);
			if ( strValue != _T("0.0.0.0"))
			{
				strIPAddress += strValue;
				strIPAddress += _T(" , ");

				strIPMask += CA2W(pAdapter->IpAddressList.IpMask.String);
				strIPMask += _T(" ");
			}

			pAdapter = pAdapter->Next; 
		} 
	}

	free ( pAdapterInfo );

	CString strTitle;
	strTitle.Format(_M("%s ( Machine Name: %s, IP Address: %s )"), m_strAppName, szHostName, strIPAddress.Left(strIPAddress.GetLength()-3) );
	
	m_pUserWnd->SetWindowText(strTitle);
}

void CDiskFileManager::OnAddToFavorite( CString strCurDir )
{
	CString strTargetDir = strCurDir;
	CString strDefaultName = strCurDir;

	strCurDir.TrimRight(_T('\\'));
	int nToken = strCurDir.ReverseFind(_T('\\'));	
	if (nToken != -1)
		strDefaultName = strCurDir.Right(strCurDir.GetLength()-nToken-1);

	CAddToFavoriteDialog atfd;
	atfd.m_strName = strDefaultName;
	atfd.m_listGroup = m_faviouriteDirDialog.GetGroupList();
	if (atfd.DoModal() != IDOK)
		return;

	m_faviouriteDirDialog.AddFavoriteDir(atfd.m_strGroup, atfd.m_strName, strTargetDir);
}
