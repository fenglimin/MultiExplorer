#pragma once


#include "CustomListStruct.h"

///////////////////////////////////////////////////////////////////
// CFilterDateTime
///////////////////////////////////////////////////////////////////
class CFilterDateTime : public CDateTimeCtrl
{
public:
	virtual void PostNcDestroy( ); 
	virtual ~CFilterDateTime();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CFilterDateTime();	

	BOOL m_bNotifySent;
	BOOL m_bEditTime;
	CellWindowType m_cellType; 

protected:
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnDateTimeChanged(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnCloseup(NMHDR *pNMHDR, LRESULT *pResult);
	DECLARE_MESSAGE_MAP()
};


///////////////////////////////////////////////////////////////////
// CFilterEdit
///////////////////////////////////////////////////////////////////
class CFilterEdit : public CEdit
{
public:
	BOOL	m_bNotifySent;
	void	SetValidChars(CString& rstrValidChars);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void PostNcDestroy( ); 
	virtual ~CFilterEdit();
	CFilterEdit();		

	void	CopyToClipboard(const CString& strText);
private:
	CString m_strValidChars;
	CString m_strEditableChars;

protected:
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg LRESULT OnPaste(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCopy(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnCut(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};




///////////////////////////////////////////////////////////////////
// CFilterCombo
///////////////////////////////////////////////////////////////////
class CFilterCombo : public CComboBox
{
public:		
	BOOL m_bNotifySent;
	BOOL	m_bDropdown;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void PostNcDestroy( ); 
	CFilterCombo();
	virtual ~CFilterCombo();
	// Generated message map functions
protected:
	//{{AFX_MSG(CFilterCombo)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnCloseup();
	afx_msg void OnSelectChanged();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};



///////////////////////////////////////////////////////////////////
// CFilterInfo
///////////////////////////////////////////////////////////////////
class CFilterInfo
{
public:
	CFilterInfo(){ m_nStatus=FILTER_ENABLED; };
	CString m_strFilter;
	int		m_nStatus;
};