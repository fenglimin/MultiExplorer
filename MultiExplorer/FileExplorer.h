#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "..\CustomListCtrl\CustomListCtrl.h"
#include "DiskFileManager.h"
#include "BtnST.h"

// CFileExplorer dialog

class CFileExplorer : public CDialog
{
	DECLARE_DYNAMIC(CFileExplorer)

public:
	CFileExplorer(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFileExplorer();

// Dialog Data
	enum { IDD = IDD_DIALOG_EXPLORER };

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()

public:
	void OnLanguageChanged();
	void FormatDirName(CString& strDir);
	BOOL ShowDirectory(const CString& strDir, BOOL bAddToHistory);
	CString GetCurDir();
	CString GetFilter(){return  m_strFilter;};
	int GetColumnWidth(int nCol){ return m_listFile.m_ctrlHeader.m_gridColumnsList[nCol].nWidth;};
	void SelectChanged();

public:
	void EnableClose(BOOL bEnable);
	void SetUIText();
	void EnableAllCtrls();
	void DisableAllCtrls();
	void AdjustListColumn();
	void LayoutWnd(CWnd* pWnd, int nLeft, int nRight, int nTop, int nBottom);
	void SetPosition(const CRect& rcWnd, BOOL bEnableClose);
	void InitList();
	void InitButtons();
	void AddDirSelection();
	void ShowDir(const CString& strDir, CString strFilter, BOOL bAddToHistory, BOOL bUpdateDirTree = TRUE);
	int	AppendDir(const CDirectoryInfo& di);
	int AppendFile(const CFileInfo& fi);
	void ShowStatictics(int nDirCount, int nFileCount, int nSelectionCount);

	CButtonST	m_btShowBackward;
	CButtonST	m_btShowForward;
	CButtonST	m_btShowUpward;
	CButtonST	m_btRefresh;
	CButtonST	m_btShowHidden;
	CButtonST	m_btClose;

	CComboBox m_comboDir;
	CEdit m_editSearch;
	CCustomListCtrl m_listFile;
	int	m_nDirCount;
	int	m_nFileCount;
	BOOL m_bShowHidden;
	BOOL m_bClosed;
	BOOL m_bRecursiveSearch;

	int	 m_nLayoutIndex;


	BOOL m_bUpdateIconInProgress;
	BOOL m_bQuickSearching;
	BOOL m_bInitializing;

	CDiskFileManager*	m_pDiskFileManager;
	CString				m_strCurDir;
	afx_msg void OnCbnSelchangeComboDir();
	afx_msg void OnEnChangeEditSearch();
	afx_msg void OnBnClickedButtonUpward();
	afx_msg void OnBnClickedButtonBackward();
	afx_msg void OnBnClickedButtonForward();
	afx_msg void OnBnClickedButtonRefresh();
	CString m_strFilter;
	afx_msg void OnBnClickedCheckShowHidden();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	CStatic m_textStatistics;

	vector<int>	m_vecColumnWidth;
	afx_msg void OnBnClickedButtonQuit();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};
