#if !defined(AFX_ENHPROGRESSCTRL_H__12909D73_C393_11D1_9FAE_8192554015AD__INCLUDED_)
#define AFX_ENHPROGRESSCTRL_H__12909D73_C393_11D1_9FAE_8192554015AD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// EnhProgressCtrl.h : header file
//



// GradientProgressCtrl.h : header file
//
// Written by matt weagle (mweagle@redrose.net)
// Copyright (c) 1998.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. If 
// the source code in  this file is used in any commercial application 
// then a simple email would be nice.
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage whatsoever.
//
// Thanks to Chris Maunder (Chris.Maunder@cbr.clw.csiro.au) for the 
// foregoing disclaimer.


class CMyMemDC : public CDC
{
public:

	// constructor sets up the memory DC
	CMyMemDC(CDC* pDC) : CDC()
	{
		ASSERT(pDC != NULL);

		m_pDC = pDC;
		m_pOldBitmap = NULL;
		m_bMemDC = !pDC->IsPrinting();

		if (m_bMemDC)    // Create a Memory DC
		{
			pDC->GetClipBox(&m_rect);
			CreateCompatibleDC(pDC);
			m_bitmap.CreateCompatibleBitmap(pDC, m_rect.Width(), m_rect.Height());
			m_pOldBitmap = SelectObject(&m_bitmap);
			SetWindowOrg(m_rect.left, m_rect.top);
		}
		else        // Make a copy of the relevent parts of the current DC for printing
		{
			m_bPrinting = pDC->m_bPrinting;
			m_hDC       = pDC->m_hDC;
			m_hAttribDC = pDC->m_hAttribDC;
		}
	}

	// Destructor copies the contents of the mem DC to the original DC
	~CMyMemDC()
	{
		if (m_bMemDC) 
		{    
			// Copy the offscreen bitmap onto the screen.
			m_pDC->BitBlt(m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(),
				this, m_rect.left, m_rect.top, SRCCOPY);

			//Swap back the original bitmap.
			SelectObject(m_pOldBitmap);
		} else {
			// All we need to do is replace the DC with an illegal value,
			// this keeps us from accidently deleting the handles associated with
			// the CDC that was passed to the constructor.
			m_hDC = m_hAttribDC = NULL;
		}
	}

	// Allow usage as a pointer
	CMyMemDC* operator->() {return this;}

	// Allow usage as a pointer
	operator CMyMemDC*() {return this;}

private:
	CBitmap  m_bitmap;      // Offscreen bitmap
	CBitmap* m_pOldBitmap;  // bitmap originally found in CMemDC
	CDC*     m_pDC;         // Saves CDC passed in constructor
	CRect    m_rect;        // Rectangle of drawing area.
	BOOL     m_bMemDC;      // TRUE if CDC really is a Memory DC.
};

/////////////////////////////////////////////////////////////////////////////
// CGradientProgressCtrl window

class CGradientProgressCtrl : public CProgressCtrl
{
// Construction
public:
	CGradientProgressCtrl();

// Attributes
public:
// Attributes
	
	void SetRange(int nLower, int nUpper);
	int SetPos(int nPos);
	int SetStep(int nStep);
	int StepIt(void);

// Operations
public:
	
	// Set Functions
	void SetTextColor(COLORREF color)	{m_clrText = color;}
	void SetBkColor(COLORREF color)		 {m_clrBkGround = color;}
	void SetStartColor(COLORREF color)	{m_clrStart = color;}
	void SetEndColor(COLORREF color)	{m_clrEnd = color;}
	void SetPercentPrefix(CString strPercentPrefix) { m_strMessage = strPercentPrefix; }
	void SetMaxSteps(int nMaxSteps) { m_nMaxSteps = nMaxSteps; }

	// Show the percent caption
	void ShowPercent(BOOL bShowPercent = TRUE)	{m_bShowPercent = bShowPercent;}
	void ShowMessage(CString strMessage, COLORREF crBKColor);
	
	// Get Functions
	COLORREF GetTextColor(void)	{return m_clrText;}
	COLORREF GetBkColor(void)		 {return m_clrBkGround;}
	COLORREF GetStartColor(void)	{return m_clrStart;}
	COLORREF GetEndColor(void)	{return m_clrEnd;}


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGradientProgressCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CGradientProgressCtrl();

	// Generated message map functions
protected:
	void DrawGradient(CPaintDC *pDC, const RECT &rectClient, const int &nMaxWidth);	
	int m_nLower, m_nUpper, m_nStep, m_nCurrentPosition;
	COLORREF	m_clrStart, m_clrEnd, m_clrBkGround, m_clrText;
	BOOL m_bShowPercent;
	CString m_strMessage;
	CFont m_font;
	int		m_nMaxSteps;
	COLORREF	m_crBKColor;
	//{{AFX_MSG(CGradientProgressCtrl)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ENHPROGRESSCTRL_H__12909D73_C393_11D1_9FAE_8192554015AD__INCLUDED_)
