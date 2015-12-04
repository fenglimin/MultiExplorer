// MultiExplorer.h : main header file for the MULTIEXPLORER application
//

#if !defined(AFX_MULTIEXPLORER_H__F74FFBF6_009B_4CC7_A3B2_DAFFBD31B9A9__INCLUDED_)
#define AFX_MULTIEXPLORER_H__F74FFBF6_009B_4CC7_A3B2_DAFFBD31B9A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CMultiExplorerApp:
// See MultiExplorer.cpp for the implementation of this class
//

class CMultiExplorerApp : public CWinApp
{
public:
	CMultiExplorerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiExplorerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CMultiExplorerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MULTIEXPLORER_H__F74FFBF6_009B_4CC7_A3B2_DAFFBD31B9A9__INCLUDED_)
