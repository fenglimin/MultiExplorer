#pragma once

#include "MultiExplorerDef.h"
#include "GradientProgressCtrl.h"
#include "resource.h"


// COperationDialog dialog

class COperationDialog : public CDialog
{
	DECLARE_DYNAMIC(COperationDialog)

public:
	COperationDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~COperationDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_OPERATION };
	CGradientProgressCtrl	m_wndProgressCtrl;

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCancel();
	void Show(int nStart, int nEnd);
	BOOL Step(const CString& strWorkDir, const CString& strWorkFile);
	void SetOperatioinType(OperationType operationType){ m_operationType = operationType; }
private:
	int m_nStart, m_nEnd, m_nPos;
	BOOL m_bCancelClicked;
	OperationType	m_operationType;
};
