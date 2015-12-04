#pragma once

#include "BtnST.h"
#include <list>
#include <vector>
using namespace std;

const UINT BUTTON_COMMAND_ID_BEGIN = WM_USER + 100;
const UINT MAX_BUTTON_COUNT			= 100;

// CToolbarDialog dialog
#include "MultiExplorerDef.h"

struct CToolbarButton
{
	UINT		nButtonID;
	CString		strButtonFile;
	CString		strTooltip;
	CButtonST*	pButton;
	BOOL		bNeedSave;
	BOOL		bDriverButton;
};

class CDiskFileManager;

class CToolbarDialog : public CDialog
{
	DECLARE_DYNAMIC(CToolbarDialog)

public:
	CToolbarDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CToolbarDialog();
	
	list<CToolbarButton*>	m_listQuickLaunchButtons;
	list<CToolbarButton*>	m_listDriverButtons;
	vector<CString>	m_vecDeletedButtonFile;
	CDiskFileManager*	m_pDiskFileManager;
	int m_nNewButtonID;
	int m_nButtonSize;

	void DeleteButton(CToolbarButton* pToolbarButton);
	void DeleteButton(const CString& strButtonFile);
	CToolbarButton* FindButton(list<CToolbarButton*>& listButtons, const POINT& pt);
	CToolbarButton* FindQuickLaunchButton(const POINT& pt);
	CToolbarButton* FindQuickLaunchButton(const CString& strButtonFile);
	void SetDiskFileManager(CDiskFileManager* pDiskFileManager) { m_pDiskFileManager = pDiskFileManager; }
	void RefreshDriverButtons();
	void AppendDriverButtons(const CDiskDriverInfo& di);

	void CheckClick(list<CToolbarButton*>& listButton, UINT uID);

	void DeleteQuickLaunchButtons() { DeleteAllButtons(m_listQuickLaunchButtons); }
	void DeleteDriverButtons() { DeleteAllButtons(m_listDriverButtons); }

	void DeleteAllButtons(list<CToolbarButton*>& listButton);
	void SetPosition(const CRect& rcWnd);
	HICON GetLargeIconFromFile(const CString& strFile);
	void GetButtonRect(int nButtonPos, CRect& rc);
	void HandleRButtonDblClick(POINT pt);
	void HandleRButtonClick( POINT pt );
	void AppendQuickLaunchButton(const CString& strButtonFile, BOOL bNeedSave);

// Dialog Data
	enum { IDD = IDD_DIALOG_TOOLBAR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	afx_msg void OnButtonClick(UINT uID);
	afx_msg void OnTimer(UINT_PTR nTimerID);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
	HICON GetPieIcon(const CDiskDriverInfo& di);
	CString GetDriverTip(const CDiskDriverInfo& di);
public:
	
};
