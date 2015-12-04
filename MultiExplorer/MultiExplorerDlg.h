// MultiExplorerDlg.h : header file
//

#pragma once

#include "DiskFileManager.h"
#include "FileExplorer.h"
#include "CaptionButton.h"

// CMultiExplorerDlg dialog
class CMultiExplorerDlg : public CDialog
{
// Construction
public:
	CMultiExplorerDlg(CWnd* pParent = NULL);	// standard constructor

	CDiskFileManager m_diskFileManager;

	void Relayout();
	void DoExit();
// Dialog Data
	enum { IDD = IDD_MULTIEXPLORER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	CCaptionButton	m_btNewExplorer;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnWMU_Hitcapbutton(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg void OnCancel();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	
};
