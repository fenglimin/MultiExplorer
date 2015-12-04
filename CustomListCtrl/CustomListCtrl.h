#pragma once
#include "CustomListHeaderCtrl.h"

class CCustomListCtrl : public CListCtrl, public ICacheList
{
	DECLARE_DYNAMIC(CCustomListCtrl)

public:
	CCustomListCtrl();
	virtual ~CCustomListCtrl();

protected:
	DECLARE_MESSAGE_MAP()
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void PreSubclassWindow();
	virtual void PostNcDestroy( ); 
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg BOOL OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnDateTimeChanged(WPARAM, LPARAM);
	afx_msg LRESULT OnEditTextChanged(WPARAM, LPARAM);
	afx_msg LRESULT OnComboSelChanged(WPARAM, LPARAM);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDestroy();
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnPaint();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
public:
	CCustomListCtrlHeader	m_ctrlHeader;
	CSimpleArray<ListCtrlColumnState>	m_ColumnStates;	
	
	CellFormat	m_defaultListFormat;

	int	m_nNumColumns;

	vector<COLORREF>	m_vecRowColors;


	CFilterEdit*	m_pEdit;
	CFilterCombo*	m_pCombo;
	CFilterDateTime*m_pDateTime;
	CMenu			m_menuContext;

	int				m_nSortColumn;
	BOOL			m_bSortAscending;


	BOOL			m_bFilterInCase;
	BOOL			m_bReadOnly;
	BOOL			m_bEditFirstCellAfterNewRowIsAdded;
	BOOL			m_bEditNextCellAfterReturnIsHit;
	BOOL			m_bDoubleClickEmptyAreaToAddRow;
	BOOL			m_bShowSelection;
	BOOL			m_bIgnoreMouse;
	BOOL			m_bAllowMultlSelection;
	BOOL			m_bHighlightChangedCellText;
	BOOL			m_bSelectThenEdit;
	BOOL			m_bEnablePopupMenu;
	int				m_nRowHeight;

	CString			m_strHighlightText;
	
public:
	virtual void		SetQueryStyle(BOOL bCaseSensitive, BOOL bQueryWhenEditting);
	virtual void		UpdateTableSytle(CustomListStyle& listStyle);
	virtual void		SetEmptyTable(BOOL bEmptyDataCache);
	virtual void		SetCacheData(I2DCache<CString>* p2DCache);
	virtual void		SetTableStyle(CustomListStyle& listStyle);
	virtual void		FilterDataFromCache(const vector<CString>& vecFilter);
	virtual void		FilterDataFromCache();
	virtual void		RebuildContextMenu(const CustomListContextMenu& contextMenu);

	BOOL		OnRowAddedInternal(int nRow);

	void 		DrawAllBelowEmptyCells(CDC* pDC, CRect rcCell, int nRow, int nBottom);
	void 		DrawCellBackground(CDC* pDC, CRect rcCell, int nRow);
	void 		DrawCellBorder(CDC* pDC, CRect rcCell, BOOL bRightMost = FALSE);
	void 		DrawCellBorder(CDC* pDC, int nRow, int nCol);
	void 		DrawCellText(CDC* pDC, CellFormat* pCellFormat, CString strCellText, CRect rcItem, BOOL bSelected );
	void 		DrawCellText(CDC* pDC, CString strCellText, CRect rcItem, UINT uTextAlign, COLORREF colorText, BOOL bSelected, BOOL bCheckSelection, BOOL bShowHighlight);
	void 		DrawCellCtrl(CDC* pDC, CellFormat* pCellFormat, int nRow, int nCol, CRect rcCtrl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	void 		DrawCellBMP(CDC* pDC, CellFormat* pCellFormat, CRect& rcCtrl, BOOL bSetCtrlRect);
	void 		DrawCellICON(CDC* pDC, CellFormat* pCellFormat, CRect& rcCtrl, BOOL bSetCtrlRect);
	void 		DrawCellButton(CDC* pDC, CellFormat* pCellFormat, BOOL bPushed);
	void 		DrawCellPictureButton(CDC* pDC, CellFormat* pCellFormat, BOOL bPushed);

	int			InsertDefaultRow(int nInsertAfter, int nCacheIndex = -1);
	int			InsertEmptyRow(int nInsertAfter, int nCacheIndex = -1, CellFormat* pDefaultRowFormat = NULL);
	int			AppendDefaultRow(int nCacheIndex = -1);
	int			AppendEmptyRow(int nCacheIndex = -1, CellFormat* pDefaultRowFormat = NULL);

	void		CopyTextToClipboard(const CString& strText);
	void		CopyCellTextToClipboard(int nRow, int nCol);
	void		PasteCellTextFromClipboard(int nRow, int nCol);
	void		EnableContextMenuItem(int nRow, int nCol);
	void		OnContextMenuAction(int nMenuID, int nRow, int nCol);
	void		SetContextMenu(const CustomListContextMenu& contextMenu);
	void		SetDefaultFormatAndValue(int nItem);
	void		SetCell(int nRow, int nCol, CString strCellText, CellFormat* pCellFormat = NULL);
	void		SetCellFormat(int nRow, int nCol, CellFormat* pCellFormat);
	CellFormat*	GetCellFormat(int nRow, int nCol, BOOL bGetForUpdate = FALSE);
	CellFormat* GetRowFormat(int nRow, BOOL bGetForUpdate = FALSE);
	BOOL		SetCellText(int nRow, int nCol, const CString& strCellText, BOOL bUpdateCache, BOOL bMarkChanged);
	BOOL		SetItemText(int nRow, int nCol, CString strCellText);
	void		SetRowStatus(int nRow, BOOL bEnable, BOOL bRepaint);
	BOOL		ConsistentRowSelection(int nRow);
	void		SetAdditionalRowData(int nRow, CString strKey, CString strData);
	void		SetAdditionalRowData(int nRow, CString strKey, int nData);
	CString		GetAdditionalRowData(int nRow, CString strKey);
	BOOL		SwapRow(int nRow1, int nRow2);
	vector<int>	FindRowByCellText(int nCol, CString strMatchText, BOOL bMatchCase, BOOL bMatchWholeWord);
	vector<int> FindSelectedRow();
	void		SetColumnWidth(int nCol, int nWidth);

	void		SetFilterWhenEditting(BOOL bFilterWhenEditting) { m_ctrlHeader.SetFilterWhenEditting(bFilterWhenEditting); }
	void		SetFilterInCase(BOOL bFilterInCase){ m_bFilterInCase = bFilterInCase;}
	void		SetReadOnly(BOOL bReadOnly) { m_bReadOnly = bReadOnly; }
	void		SetEditFirstCellAfterNewRowIsAdded(BOOL bEditFirstCellAfterNewRowIsAdded) { m_bEditFirstCellAfterNewRowIsAdded = bEditFirstCellAfterNewRowIsAdded; }
	void		SetEditNextCellAfterReturnIsHit(BOOL bEditNextCellAfterReturnIsHit){ m_bEditNextCellAfterReturnIsHit = bEditNextCellAfterReturnIsHit;}
	void		SetEnablePopupMenu(BOOL bEnablePopupMenu) { m_bEnablePopupMenu = bEnablePopupMenu; }
	void		SetClickEmptyAreaToAddRow(BOOL bClickEmptyAreaToAddRow) { m_bDoubleClickEmptyAreaToAddRow = bClickEmptyAreaToAddRow; }
	void		SetSelectThenEdit(BOOL bSelectThenEdit) { m_bSelectThenEdit = bSelectThenEdit;}
	void		SetHighlightChangedCellText(BOOL bHighlightChangedCellText) { m_bHighlightChangedCellText = bHighlightChangedCellText; }
	void		SetIgnoreMouse(BOOL bIgnoreMouse) { m_bIgnoreMouse = bIgnoreMouse; }
	void		SetUser(ICustomListUser* pUser) { m_pUser = pUser; }
	void		SetShowSelection(BOOL bShowSelectionBackground) { m_bShowSelection = bShowSelectionBackground; }
	void		SetMultipleSelection(BOOL bMultiple);
	void 		SetHeaderHeight(int nHeight) { m_ctrlHeader.m_Height = nHeight; }
	void 		SetRowHeigt(int nHeight);
	void 		SetGridLineColor(COLORREF colorHorLine, COLORREF colorVerLine);
	void 		EnableFilter(BOOL bEnabled) { m_ctrlHeader.EnableFilter(bEnabled);}
	void 		AddDefaultRowColor(COLORREF colorRow) { m_vecRowColors.push_back(colorRow);}
	void		AddDefaultRowColor(const CString& strRGB) { m_vecRowColors.push_back(StringToColorRef(strRGB));	}
	void		SetFontHeight(int nFontHeight) { m_nFontHeight = nFontHeight; }
	void		EnableSort(BOOL bSort) { m_ctrlHeader.EnableSort(bSort);}
	void		SetAllItemSelected(BOOL bSelected);

	BOOL		DeleteItem( int nItem, BOOL bDeleteFromCache );
	BOOL		DeleteAllItems(BOOL bDeleteFromCache);
	void		FreeItemMemory( const int nItem, BOOL bDeleteFromCache );
	void 		DeleteAllSelectedItems(BOOL bDeleteFromCache, BOOL bNotifyUser);

	void		SetColumnInfo(CString strColumnInfo, CString strColumnOrder);
	void		SetColumnInfo(CustomColumnList& headerList, CString strColumnOrder);
	void		SetColumnOrder(CString strColumnOrder);
	void		GetColumnInfo(CString &strColumnInfo, CString &strColumnOrder);
	BOOL		SetHeadings( const CString& strHeadings );
	void		InsertColumnState(int nCol, bool bVisible, int nOrgWidth);

	void		OnCellCtrlHiding(BOOL bValidate, CString& strNewValue);
	void		OnCellCtrlHidden(BOOL bReturnIsHit);
	BOOL		EditNextEditableCell(int nRow, int nCol);
	BOOL		StartCellEdit(int nRow, int nCol);
	void		ShowEdit(int nRow, int nCol, UINT nMaxLen, CString& rStrValidChars);;
	void		HideEdit(BOOL bValidate);
	void 		ShowCombo(int nRow, int nCol,CStringList& rComboItemsList);
	void 		HideCombo(BOOL bValidate);
	void		ShowDateTime(int nRow, int nCol, BOOL bShowTimePart);
	void		HideDateTime(BOOL bValidate);
	void		ShowNotes( int nRow, int nCol );
	void		RefreshRow(int nRow);

	void		HitTest(LV_HITTESTINFO&  ht);
	BOOL		CheckClick(int& nRow, int& nCol, CellFormat** pCellFormat);

	int			GetSortColumnID(){ return m_nSortColumn; }
	void		Sort( int iColumn, BOOL bAscending );
	void		SortPrevious();
	int			NumberCompare( LPCTSTR pszNumber1, LPCTSTR pszNumber2 );
	int			DateCompare( const CString& strDate1, const CString& strDate2, ColumnCompareType iType);
	static int CALLBACK CompareFunction( LPARAM lParam1, LPARAM lParam2, LPARAM lParamData );
	ColumnCompareType IsDateTime( LPCTSTR pszText );
	void		HideAllCellCtrl(BOOL bValidate);
	
	void		LoadDefaultIcon();
	BOOL		GetEditRect(int nRow, int nCol, CRect& rcEdit);

	CString		ColorRefToString(COLORREF cr);
	COLORREF	StringToColorRef(const CString& strRGB);
	CString		BrowserForFolder(HWND hwndOwner, const char* strTitle, CString strInitDir);
	HFONT		SetTextFontWeight(CDC* pDC, LONG lfWeight);
private:
	HICON		m_hFolder;
	int			m_nFontHeight;
	BOOL		m_bEndEditSent;
	BOOL		m_bCellButtonClicked;
	int			m_nEditRow;
	int			m_nEditCol;
	HFONT		m_hCellTextFont;
	CString		m_strEmptyMarkupText;
	CPen		m_penHorLine;
	CPen		m_penVerLine;
	CFont		m_FilterFont;
	BOOL		m_bChangeCellTextRejected;	// If is FALSE, the changed was discarded and the cell text remain unchanged.
	BOOL		m_bShowTimeCtrlForDateTimeCell;

	ICustomListUser*	m_pUser;
	I2DCache<CString>*  m_p2DCache;

};
