#pragma once

#include "..\CustomListCtrl\CustomListCtrl.h"
#include "..\Socket\WorkTool.h"
#include "Ini.h"
#include "CommonTool.h"
#include "OperationDialog.h"
#include "MultiExplorerDef.h"
#include "LayoutManager.h"
#include "ToolbarDialog.h"
#include "DirTreeDialog.h"
#include "FavoriteDirDialog.h"

class CFileExplorer;

struct CDiskFilePresenter
{
	CFileExplorer*	pFileExplorer;
	vector<CString> vecDirHistory;
	int nCursor;
};


class CDiskFileManager : public ICustomListUser, public IWorkToolServerUser
{
public:
	CDiskFileManager(void);
	~CDiskFileManager(void);

	HANDLE m_hRefreshDiskDriveThreadHandle;
	vector<CDiskDriverInfo> m_vecDiskDriverInfo;
	vector<CDiskDriverInfo> m_vecDiskDriverInfoLast;
	CDefaultDirectory	m_defaultDir;
	UserOptions m_userOption;
	CCommonTool	m_commonTool;
	COperationDialog*	m_pOperationDlg;
	CLayoutManager		m_layoutManager;
	CToolbarDialog		m_toolbarDialog;
	CDirTreeDialog		m_dirTreeDialog;
	CFavoriteDirDialog	m_faviouriteDirDialog;
	CString				m_strDefaultIniFile;
	CWorkTool* m_pWorkTool;

	void StartNetWorkerThread();
	void SetWorkToolClientUser(IWorkToolClientUser* pClientUser) { m_pWorkTool->SetClientUser(pClientUser); }
	void ShowDirInActiveFileExplorer(const CString& strDir);
	BOOL StartNewMultiExplorer(const CString& strDir);
	void SetFileExplorerBackground(CFileExplorer* pFileExplorer, BOOL bActive);
	void SetActiveFileExplorer(CFileExplorer* pFileExplorer);
	void SwitchWithActiveFileExplorer(CFileExplorer* pFileExplorer);
	BOOL IsDriverAlive(const CString& strDriverLetter);
	void DeleteQuickLaunchFromWindowDefault();
	void DeleteQuickLaunchFromDesktop();
	void DeleteQuickLaunchFromDir(const CString& strDir);

	void LoadQuickLaunchFromWindowDefault();
	void LoadQuickLaunchFromDesktop();
	void LoadQuickLaunchFromDir(const CString& strDir);

	BOOL ConfirmDelete(const CDiskFile& diskFile);
	BOOL DeleteFile(const CString& strDir, const CString& strFile);
	BOOL DeleteDir(const CString& strDir);

	void GetDriverInfo(LPCTSTR szDriver, CDiskDriverInfo& di, BOOL bNotGetCapacityAndLabel);
	BOOL GetDiskUsageInfo(LPCTSTR szDirectory, __int64& i64Used, __int64& i64Total);
	
	BOOL CopyFile(const CString& strWorkDir, const CString& strDestFile, const CString& strSrcFullPath);
	BOOL CopyDir(const CString& strWorkDir, const CString& strDestDir, const CString& strSrcFullPath);
	BOOL CheckCopyFromClipboard(CDiskFile& diskFile, BOOL& bCut);
	BOOL AddToDiskFile(CDiskFile& diskFile, const CString& strName);
	BOOL CopyOrCutToClipboard(const CDiskFile& diskFile, BOOL bCopy);
	BOOL ConvertToClipboardFormat(const CDiskFile& diskFile, TCHAR* szFiles, UINT& uBufLen);

	void GetDiskFileSelection(CCustomListCtrl* pListCtrl, CDiskFile& diskFile);
	int	 GetFileCount(const CString& strDir);
	int	 GetFileCount(const CDiskFile& diskFile);
	             
	BOOL Relayout(const CRect& rcClient, BOOL bCountChanged, BOOL bMustSelect, BOOL bAdjustListColumn);
	int	 GetDisplayedExplorerCount();
	BOOL ExportIniFile(const CString& strFileName, BOOL bEncrypt);
	void UpdateIniFileInResource();
	BOOL LoadIniFile(CString strFileName, BOOL bEncrypt);


	BOOL Init(CWnd* pUserWnd);
	void InitDefaultDirs();
	void InitDefaultBitmaps();
	CString GetSpecialFolder(UINT uFolderID);
	void DistroyAllCell();

	BOOL HasPrevDir(CListCtrl* pListCtrl);
	BOOL HasNextDir(CListCtrl* pListCtrl);
	void ShowPrevDir(CListCtrl* pListCtrl);
	void ShowNextDir(CListCtrl* pListCtrl);

	CString GetCurrentFormatTime();
	CString GetDefaultRename(const CString& strDir, const CString& strName);
	CString GetDefaultNewName(const CString& strDir, const CString& strDefault, const CString& strDefaultExt);
	void DirShown(CListCtrl* pListCtrl, const CString& strDir);
	BOOL HandleOpen(CListCtrl* pListCtrl, int nRow);
	void AddDiskFilePresenter(CListCtrl* pListCtrl, CFileExplorer* pDiskFileCoordinator);

	vector<CDiskDriverInfo> GetDiskDriverList(BOOL bNotGetCapacityAndLabel);
	__int64 GetFileSize(CString strFile);
	BOOL GetDirFileList(const CString& strDir, vector<CDirectoryInfo>& vecSubDir, vector<CFileInfo>& vecFile, BOOL bLoadHiddenFile);
	BOOL GetDirFileList(const CString& strDir, vector<CString>& vecSubDir, vector<CString>& vecFile);
	BOOL GetDirFileListRecursive( const CString& strDir, BOOL bRecursive, const CString& strRecursiveDir, vector<CDirectoryInfo>& vecAllDirs, vector<CFileInfo>& vecAllFiles, BOOL bLoadHidden, const CString& strFilterName, const CString& strFilterExt );
	virtual BOOL OnCellTextChanged(CListCtrl* pListCtrl, int nRow, int nCol);
	virtual BOOL OnCellTextChanging(CListCtrl* pListCtrl, int nRow, int nCol, CellFormat* pCellFormat, const CString& strOldValue, CString& strNewValue);
	virtual BOOL OnRowLDblClicked(CListCtrl* pListCtrl, int nRow, int nCol, UINT nFlags, CPoint point);
	virtual BOOL OnRowRClicked(CListCtrl* pListCtrl, int nRow, int nCol, UINT nFlags, CPoint point);
	virtual BOOL OnRowLClicked(CListCtrl* pListCtrl, int nRow, int nCol, UINT nFlags, CPoint point);
	virtual BOOL OnKeyHitted(CListCtrl* pListCtrl, UINT uKey) ;
	virtual BOOL OnVirtualKeyHitted(CListCtrl* pListCtrl, UINT uVirtualKey);
	virtual BOOL OnSort(CListCtrl* pListCtrl, RowItemData* pid1, RowItemData* pid2, int& nCompareResult);

	virtual BOOL OnGetAllDirFilesFromClipboard(CDiskFile& diskFile, int& nTotalSizeInM);

	void OnUserOption();
	void OnOpenDirWithExplorer(const CString& strDir);
	void OnOpenDirWithDosCmd(const CString& strDir);
	void OnOpenFile(const CString& strFile, const CString& strOpener);
	void OnRunFileInDosCmd(const CString strFile, const CString& strDir);
	void OnOpenMultiFile(const CDiskFile& diskFile, const CString& strOpener);
	void OnCompare(const CString& strLeft, const CString& strRight);
	BOOL OnRename(const CString& strOld, const CString& strNew);
	BOOL OnProperty(const CDiskFile& diskFile);
	void OnNewCell(const CString& strDir);
	void OnDeleteCell(CFileExplorer* pFileExplorer, const CString& strNewLayout);
	void OnChangeLayout(BOOL bMustSelect);
	BOOL OnDelete(const CDiskFile& diskFile, CFileExplorer*	pFileExplorer, BOOL bConfirm);
	void OnCopy(const CDiskFile& diskFile);
	void OnCut(const CDiskFile& diskFile, CListCtrl* pListCtrl);
	BOOL OnPaste(CDiskFile& diskFile, const CString& strDestDir, CFileExplorer*	pFileExplorer, BOOL bCut);
	BOOL OnShellFileOperation(CDiskFile& diskFile, const CString& strDestDir, CFileExplorer* pFileExplorer, UINT uOperation);
	BOOL OnListStyleChanged();
	BOOL OnListBackGroundColorChanged();
	BOOL OnImport();
	BOOL OnExport();
	BOOL OnSaveAs();
	BOOL OnExtractFile(const CString& strFile, const CString& strDir);
	BOOL OnCreateRarFile(const CString& strFile, const CDiskFile& diskFile);
	BOOL OnCreateNewDirectory(CFileExplorer* pFileExplorer);
	BOOL OnCreateNewTextFile(CFileExplorer* pFileExplorer);
	void OnAddToToolbar(const CDiskFile& diskFile);

	CString GetCopyName(const CString& strDir, const CString& strName, BOOL bIsDir);

	CBitmap* GetBitmapFromIcon(HICON hIcon);
	CBitmap* GetBitmapFromFile(const CString& strFile);
	HANDLE GetIconFromFile(DWORD dwAttr, const CString& strFile);
	HANDLE GetIconFromCache(const CString& strExtName);

	void StartUpdateIconThread(CFileExplorer* pFileExplorer);
	void DoUpdateIcon(CFileExplorer* pFileExplorer);

	int GetExplorerCount() { return m_mapPresenter.size(); }

	CString	GetAppName();


	void DoRelayout(const CRect& rcClient,const CString& strLayout, BOOL bAdjustListColumn);
	void RebuildBitmapMap();
	CWinThread* StartRefreshDiskDriverThread();

	void DoExit(BOOL bSave);
	BOOL IsRefreshDiskDriverThreadActive();

	void SetTitleWithNetworkInfo();
	void OnAddToFavorite( CString strCurDir );
	void ShowDirInFirstFileExplorer( const CString& strDir );
	CWnd*	m_pUserWnd;
private:

	vector<CRect> m_vecCurrentLayout;
	CFileExplorer*	m_pActiveFileExplorer;
	map<CString, HANDLE> m_mapPicture;
	CListCtrl*		m_pCutList;	// The list that do the cut operation
	
	map<CListCtrl*, CDiskFilePresenter*>	m_mapPresenter;
	map<CString, CBitmap*> m_mapBitmap;
	CString	m_strCompareLeft;
	BOOL	m_bIsCompareLeftDir;
	

	CBitmap*	m_pBitmapExplorer;
	CBitmap*	m_pBitmapCmd;
	CBitmap*	m_pBitmapNotepad;
	CBitmap*	m_pBitmapMultiExplorer;

	CString	m_strAppName;
	CString	m_strIniFile;
	CString m_strRumCmdFile;
	CIni	m_iniFile;
};

struct CUpdateIconThreadParam
{
	CDiskFileManager*	pDiskFileManager;
	CFileExplorer*	pFileExplorer;
};