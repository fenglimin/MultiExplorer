// MultiExplorer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "MultiExplorer.h"
#include "MultiExplorerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMultiExplorerApp

BEGIN_MESSAGE_MAP(CMultiExplorerApp, CWinApp)
	//{{AFX_MSG_MAP(CMultiExplorerApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiExplorerApp construction

CMultiExplorerApp::CMultiExplorerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CMultiExplorerApp object

CMultiExplorerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CMultiExplorerApp initialization

BOOL CMultiExplorerApp::InitInstance()
{
	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

	CString strCmdLine = GetCommandLine();

	int nToken = strCmdLine.Find(_T("\" "));
	CString strIniFile = strCmdLine.Right(strCmdLine.GetLength()-nToken-3);
	strIniFile = strIniFile.Left(strIniFile.GetLength()-1);

	CMultiExplorerDlg dlg;
	dlg.m_diskFileManager.m_strDefaultIniFile = strIniFile;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
