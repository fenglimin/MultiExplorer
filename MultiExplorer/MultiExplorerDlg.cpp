// MultiExplorerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MultiExplorer.h"
#include "MultiExplorerDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetWindowText(_M("About MultiExplorer"));
	SetDlgItemText(IDC_STATIC_VERSION, _M("MultiExplorer Version 1.0"));
	SetDlgItemText(IDC_STATIC_DEVELOPPER, _M("Developer : Sail Feng,  mailflm@163.com"));

	SetDlgItemText(IDOK, _M("OK"));

	return TRUE;
}


// CMultiExplorerDlg dialog




CMultiExplorerDlg::CMultiExplorerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMultiExplorerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMultiExplorerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}
// announce the user defined message:
DECLARE_USER_MESSAGE(WMU_HITCAPBUTTON)

BEGIN_MESSAGE_MAP(CMultiExplorerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
	ON_WM_SIZING()
	ON_WM_SIZE()
	ON_WM_CLOSE()
	ON_REGISTERED_MESSAGE(WMU_HITCAPBUTTON, OnWMU_Hitcapbutton)
END_MESSAGE_MAP()


// CMultiExplorerDlg message handlers

BOOL CMultiExplorerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);


	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	//CCaptionButton::InitCapBtn(m_hWnd);
	//m_btNewExplorer.SetBmpID(1, IDB_BITMAP1, IDB_BITMAP2, TRUE);

	ShowWindow(SW_MAXIMIZE);

	CMultiLanguage::Init();
	m_diskFileManager.Init(this);
	

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);

		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, ID_POPUP_CHANGELAYOUT, _M("Change Layout") + _T("..."));
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, ID_POPUP_IMPORT, _M("Import Configuration") + _T("..."));
		pSysMenu->AppendMenu(MF_STRING, ID_POPUP_EXPORT, _M("Export Configuration") + _T("..."));
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, ID_POPUP_OPTIONS, _M("Options") + _T("..."));
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, ID_POPUP_SAVE_AS, _M("Export New MultiExplorer") + _T("..."));
		pSysMenu->AppendMenu(MF_SEPARATOR);
		pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, _M(strAboutMenu));
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMultiExplorerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	TRACE(_T("%d"), nID);
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if ( nID == ID_POPUP_OPTIONS)
	{
		m_diskFileManager.OnUserOption();
	}
	else if ( nID == ID_POPUP_CHANGELAYOUT)
	{
		m_diskFileManager.OnChangeLayout(FALSE);
	}
	else if ( nID == ID_POPUP_IMPORT )
	{
		m_diskFileManager.OnImport();
	}
	else if ( nID == ID_POPUP_EXPORT)
	{
		m_diskFileManager.OnExport();
	}
	else if ( nID == ID_POPUP_SAVE_AS )
	{
		m_diskFileManager.OnSaveAs();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMultiExplorerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMultiExplorerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CMultiExplorerDlg::Relayout()
{
	if ( !m_hWnd )
		return;

	CRect rcClient;
	GetClientRect(&rcClient);

	m_diskFileManager.Relayout(rcClient, FALSE, FALSE, TRUE);
}


void CMultiExplorerDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);


}

void CMultiExplorerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	Relayout();
	Invalidate(TRUE);
}

LRESULT CMultiExplorerDlg::OnWMU_Hitcapbutton(WPARAM wParam, LPARAM lParam)
{



	return 0; // Allways return 0(LRESULT->void)
}

void CMultiExplorerDlg::OnDestroy() 
{
	CDialog::OnDestroy();
}

void CMultiExplorerDlg::OnCancel()
{
	DoExit();
}

void CMultiExplorerDlg::OnClose()
{
	DoExit();
}

void CMultiExplorerDlg::DoExit()
{
	if ( m_diskFileManager.m_strDefaultIniFile.IsEmpty() )
	{
		BOOL bSave = TRUE;
		if ( m_diskFileManager.m_userOption.bPromptForSave )
		{
			UINT uRet = AfxMessageBox(_M("Do you want to save the changes?"), MB_YESNOCANCEL);
			if ( uRet == IDCANCEL )
				return;

			if ( uRet == IDNO )
			{
				bSave = FALSE;
			}
		}

		ShowWindow(SW_HIDE);

		m_diskFileManager.DoExit(bSave);
	}


	CDialog::OnOK();
}
