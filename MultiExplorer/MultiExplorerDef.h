#pragma once

#include <vector>
#include <map>
using namespace std;

struct CDirectoryInfo
{
	CString	strName;
	CString strDate;
};

struct CFileInfo
{
	CString	strName;
	CString	strExt;
	__int64	nSize;
	CString strDate;
};

enum OperationType
{
	operationNull		= 0,
	operationCopy		= 1,
	operationCut		= 2,
	operationPaste		= 3,
	operationDelete		= 4,

};

struct CDiskFile
{
	CString				strWorkDir;
	vector<CString>		vecDirectory;
	vector<CString>		vecFile;

	void Init()
	{
		vecDirectory.clear();
		vecFile.clear();
	}
};

struct RemoteMachine
{
	CString			strIpAddress;
	int				nPort;
};

struct UserOptions
{
	CString		strLanguage;

	CString		strLayout;
	CString		strInternetExplorer;
	CString		strBeyondComparePath;
	CString		strUltraEditPath;
	CString		strNotepadPPPath;
	CString		strWinRarPath;

	CString		strOddLineBKColorActive;
	CString		strEvenLineBKColorActive;
	CString		strOddLineBKColorInactive;
	CString		strEvenLineBKColorInactive;

	int			nRowHeight;
	int			nToolbarButtonSize;
	BOOL		bPromptForSave;
	BOOL		bLoadQuickLaunchFromDesktop;
	BOOL		bLoadQuickLaunchFromTaskbar;

	BOOL		bShowDirTree;
	int			nDirTreeDlgWidth;

	int			nLocalListeningPort;
	vector<RemoteMachine> vecRemoteMachine;

	UserOptions()
	{
		strLanguage				= _T("English");
		strLayout				= _T("1X2;0;1");
		strInternetExplorer		= _T("");
		strBeyondComparePath	= _T("");
		strUltraEditPath		= _T("");
		strNotepadPPPath		= _T("");
		strWinRarPath			= _T("");

		nRowHeight				= 22;
		nToolbarButtonSize		= 36;
		bPromptForSave			= TRUE;

		bLoadQuickLaunchFromTaskbar = FALSE;
		bLoadQuickLaunchFromDesktop = FALSE;
		bShowDirTree = FALSE;
		nDirTreeDlgWidth = 300;

		strOddLineBKColorActive = _T("RGB(239,239,239)");
		strEvenLineBKColorActive = _T("RGB(227,227,227)");

		strOddLineBKColorInactive = _T("RGB(227,227,227)");
		strEvenLineBKColorInactive = _T("RGB(200,200,200)");

		vecRemoteMachine.clear();
	}
};

struct CDefaultDirectory
{
	CString		strWindowsDir;
	CString		strTempDir;
	CString		strDesktopDir;
	CString		strAllUserDesktopDir;
	CString		strMyDocumentsDir;
	CString		strRecyclerDir;
	CString		strAppDataDir;
	CString		strWorkPath;
};

struct CDiskDriverInfo
{
	CString strDriverLetter;
	CString strLabel;
	int		nDriverType;
	__int64	nTotalSpace;
	__int64 nUsedSpace;

	BOOL	bAlive;

};

