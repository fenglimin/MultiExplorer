#pragma once

#include "..\CustomListCtrl\CustomListCtrl.h"
#include "..\Socket\WorkTool.h"

// NetworkOperationDialog dialog
class CDiskFileManager;

class CNetworkOperationDialog : public CDialog, public ICustomListUser, public IWorkToolClientUser
{
	DECLARE_DYNAMIC(CNetworkOperationDialog)

public:
	CNetworkOperationDialog(CDiskFileManager* pDiskFileManager, CWnd* pParent = NULL);   // standard constructor
	virtual ~CNetworkOperationDialog();

	void OnGetClipboardData();
	void OnCancelGetClipboardData();
	
	void AppendMessage(CString strMessage, BOOL bAddTimeStamp, BOOL bAppendEndline);

	CCustomListCtrl m_listMachine;
	CDiskFileManager* m_pDiskFileManager;
	int m_nRow;
	BOOL m_bPortChanged;

	BOOL m_bLastOpIsText;
	CString m_strLastOpData;

// Dialog Data
	enum { IDD = IDD_DIALOG_NETWORK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnRowLDblClicked(CListCtrl* pListCtrl, int nRow, int nCol, UINT nFlags, CPoint point);
	virtual BOOL OnNewMessage(const CString& strMessage, BOOL bAddTimeStamp);
	
	virtual BOOL OnEmptyDirReceived(CString strTargetDir, CString strDir);
	virtual BOOL OnStart(BOOL bIsText);
	virtual BOOL OnComplete(BOOL bIsText, CString strData);

	void SaveConfig();
	void LoadConfig();
	



	void SetUIText();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonGetData();
	afx_msg void OnClose();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonPostAction();
};
