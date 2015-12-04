// OperationDialog.cpp : implementation file
//

#include "stdafx.h"
#include "MultiExplorer.h"
#include "OperationDialog.h"


// COperationDialog dialog

IMPLEMENT_DYNAMIC(COperationDialog, CDialog)

COperationDialog::COperationDialog(CWnd* pParent /*=NULL*/)
	: CDialog(COperationDialog::IDD, pParent)
{
	m_bCancelClicked = FALSE;
	m_operationType = operationNull;
}

COperationDialog::~COperationDialog()
{
}

void COperationDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS_OPERATION, m_wndProgressCtrl);
}


BEGIN_MESSAGE_MAP(COperationDialog, CDialog)
	ON_BN_CLICKED(IDCANCEL, &COperationDialog::OnBnClickedCancel)
END_MESSAGE_MAP()


// COperationDialog message handlers
BOOL COperationDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

//	InitList();
	m_wndProgressCtrl.SetStartColor ( RGB(255,0,0) );
	m_wndProgressCtrl.SetEndColor ( RGB(0,255,0) );
	m_wndProgressCtrl.SetTextColor( RGB(0,0,0) );
	m_wndProgressCtrl.SetBkColor( RGB(255,255,255) );

	return TRUE;
}

void COperationDialog::OnBnClickedCancel()
{
	m_bCancelClicked = TRUE;
}

void COperationDialog::Show(int nStart, int nEnd)
{
	Create ( IDD_DIALOG_OPERATION );

	ShowWindow ( SW_SHOW );
	CenterWindow ();
	UpdateWindow ();

	m_nStart = nStart;
	m_nEnd = nEnd;
	m_nPos = nStart;

	m_wndProgressCtrl.SetRange ( m_nStart, m_nEnd );
	m_wndProgressCtrl.ShowPercent ( TRUE );
}

BOOL COperationDialog::Step(const CString& strWorkDir, const CString& strWorkFile)
{
	MSG Message;
	while ( ::PeekMessage ( &Message, NULL, 0, 0, PM_NOREMOVE ) )
	{
		if ( !AfxGetApp()->PumpMessage() )
			::PostQuitMessage(0);
	}

	if ( m_bCancelClicked )
		return FALSE;

	SetDlgItemText ( IDC_STATIC_WORK_DIR, strWorkDir );
	SetDlgItemText ( IDC_STATIC_WORK_FILE, strWorkFile );

	m_wndProgressCtrl.SetPos ( ++m_nPos );

	if ( m_nPos > m_nEnd )
	{
		//Sleep ( 500 );
		//DestroyWindow();
		return FALSE;
	}

	return TRUE;
}
