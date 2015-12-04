#pragma once
#include "MultiLanguage.h"
#include "CustomListConstant.h"
#include "2DCacheInterface.h"
#include <map>
#include <vector>
#include <list>
#include <algorithm>


using namespace std;

enum	CellWindowType
{
	cellUnknown				= 0,
	cellTextReadOnly		= 1,
	cellTextEdit			= 2,
	cellTextCombo			= 3,
	cellCtrlCheckBox		= 4,
	cellCtrlRadioButton		= 5,
	cellCtrlButton			= 6,
	cellCtrlPictureButton	= 7,
	cellPictureBMP			= 8,
	cellPictureICON			= 9,
	cellTextTime			= 10,
	cellTextDate			= 11,
	cellTextDateTime		= 12,
	cellTextNotes			= 13,
	cellCtrlFile			= 14,
	cellCtrlDirectory		= 15,
	cellCtrlColor			= 16,
};

enum ColumnCompareType
{	
	compareAsNumber		= 1,
	compareAsText		= 2,
	compareAsTime		= 3,
	compareAsDate		= 4,
	compareAsDateTime	= 5,
};

enum CellFormatLevel
{
	levelList		= 1,
	levelRow		= 2,
	levelCell		= 3,
};

struct CustomComboboxItem
{
public:
	long lKey;
	CString strText;
	CString	strValue;

	CustomComboboxItem(long lInputKey, CString strInputText, CString strInputValue)
	{
		lKey = lInputKey;
		strText = strInputText;
		strValue = strInputValue;
	}
};
typedef std::vector<CustomComboboxItem> CustomComboboxItemList;

struct ListCtrlColumnState
{
	ListCtrlColumnState()
		:m_bVisible(false)
		,m_nOrgWidth(0)
		,m_nOrgPosition(-1)
	{}
	bool m_bVisible;
	int  m_nOrgWidth;	// Width it had before being hidden
	int  m_nOrgPosition;	// Position it had before being hidden
};


struct CustomColumn
{
public:
	CString		strDefaultValue;
	CString		strHeaderCaption;
	CString		strHeaderFilter;
	BOOL		bVisible;
	int			nWidth;
	int         nFormat;
	CString		strValidChars;
	CellWindowType	ctrlType;
	ColumnCompareType compareType;
	CustomComboboxItemList		dropdownValueList;

	// Constructor
	CustomColumn()
	{
		Reset();
	}
	void Reset()
	{
		compareType = compareAsText;
		strHeaderCaption = "";
		bVisible = TRUE;
		nWidth   = 210;
		strValidChars = "";
		ctrlType = cellTextEdit;
		dropdownValueList.clear();
		nFormat = LVCFMT_LEFT;
		strDefaultValue = "";
	}

	void FromString(CString strColumnInfo)
	{
		Reset();

		CString strTemp;
		int nPos = strColumnInfo.Find("*_*");
		strHeaderCaption = strColumnInfo.Left(nPos);
		strColumnInfo = strColumnInfo.Right(strColumnInfo.GetLength()-nPos-3);

		nPos = strColumnInfo.Find("*_*");
		nWidth = atoi(strColumnInfo.Left(nPos));
		strColumnInfo = strColumnInfo.Right(strColumnInfo.GetLength()-nPos-3);

		nPos = strColumnInfo.Find("*_*");
		strTemp = strColumnInfo.Left(nPos);
		if ( strTemp == _T("Number") )
			compareType = compareAsNumber;
		else if ( strTemp == _T("Text") )
			compareType = compareAsText;
		else if ( strTemp == _T("Time") )
			compareType = compareAsTime;
		else if ( strTemp == _T("Date") )
			compareType = compareAsDate;
		else 
			compareType = compareAsDateTime;
		strColumnInfo = strColumnInfo.Right(strColumnInfo.GetLength()-nPos-3);

		nPos = strColumnInfo.Find("*_*");
		strTemp = strColumnInfo.Left(nPos);
		if ( strTemp == _T("Edit") )
			ctrlType = cellTextEdit;
		else if ( strTemp == _T("Combo") )
			ctrlType = cellTextCombo;		
		else if ( strTemp == _T("Notes") )
			ctrlType = cellTextNotes;
		else if ( strTemp == _T("Date") )
			ctrlType = cellTextDate;
		else if ( strTemp == _T("Time") )
			ctrlType = cellTextTime;
		else if ( strTemp == _T("DateTime") )
			ctrlType = cellTextDateTime;
		else
			ctrlType = cellTextReadOnly;
			
		strColumnInfo = strColumnInfo.Right(strColumnInfo.GetLength()-nPos-3);

		nPos = strColumnInfo.Find("*_*");
		strTemp = strColumnInfo.Left(nPos);
		if ( strTemp == _T("Left") )
			nFormat = LVCFMT_LEFT;
		else if ( strTemp == _T("Center") )
			nFormat = LVCFMT_CENTER;
		else
			nFormat = LVCFMT_RIGHT;
		strColumnInfo = strColumnInfo.Right(strColumnInfo.GetLength()-nPos-3);

		nPos = strColumnInfo.Find("*_*");
		strDefaultValue = strColumnInfo.Left(nPos);
		strColumnInfo = strColumnInfo.Right(strColumnInfo.GetLength()-nPos-3);

		nPos = strColumnInfo.Find("*_*");
		strTemp = strColumnInfo.Left(nPos);
		if ( strTemp.GetLength() > 0 && strTemp.GetAt(strTemp.GetLength()-1) != ';' )
		{
			strTemp += _T(";");
		}
		nPos = strTemp.Find(";");
		CString strComboItem;
		while ( nPos != -1 )
		{
			strComboItem = strTemp.Left(nPos);
			dropdownValueList.push_back(CustomComboboxItem(0, strComboItem, strComboItem));
			strTemp = strTemp.Right(strTemp.GetLength()-nPos-1);
			nPos = strTemp.Find(";");
		}
	}

	CString ToString()
	{
		CString strColumnInfo, strCompareType, strCtrlType, strFormat, strComboItems;

		if ( compareType == compareAsNumber )
			strCompareType = _T("Number");
		else if ( compareType == compareAsText )
			strCompareType = _T("Text")			;
		else if ( compareType == compareAsTime)
			strCompareType = _T("Time") ;
		else if ( compareType == compareAsDate )
			strCompareType = _T("Date");
		else 
			strCompareType = _T("DateTime");

		if ( ctrlType == cellTextEdit )
			strCtrlType = _T("Edit");
		else if ( ctrlType == cellTextNotes )
			strCtrlType = _T("Notes");
		else if ( ctrlType == cellTextCombo )
			strCtrlType = _T("Combo");
		else if ( ctrlType == cellTextDate )
			strCtrlType = _T("Date");
		else if ( ctrlType == cellTextTime )
			strCtrlType = _T("Time");
		else if ( ctrlType == cellTextDateTime )
			strCtrlType = _T("DateTime");
		else
			strCtrlType = _T("ReadOnly");

		if ( nFormat == LVCFMT_LEFT )
			strFormat = _T("Left");
		else if ( nFormat == LVCFMT_CENTER )
			strFormat = _T("Center");
		else
			strFormat = _T("Right");

		for ( int i = 0; i < (int)dropdownValueList.size(); i++ )
		{
			strComboItems += dropdownValueList[i].strText + _T(";");
		}

		strColumnInfo.Format(_T("%s*_*%d*_*%s*_*%s*_*%s*_*%s*_*%s*_*"),
			strHeaderCaption, nWidth, strCompareType, strCtrlType, strFormat, strDefaultValue, strComboItems );

		return strColumnInfo;
	}
};

typedef std::vector<CustomColumn> CustomColumnList;



struct CellFormat
{
	// The level of the format ( Cell/Row/List )
	CellFormatLevel formatLevel;

	// Format for the cell text
	COLORREF		colorCellBackground;
	COLORREF		colorText;
	UINT			uTextAlign;
	LONG			lfWeight;
	BOOL			bShowHighlight;

	// Format the the cell window
	CellWindowType	cellType;
	CString			strCellCtrlText;
	CRect			rcCellCtrl;
	CRect			rcCellText;

	BOOL			bShowCtrl;
	BOOL			bShowText;
	BOOL			bCellTextChange;
	BOOL			bEnabled;
	BOOL			bChecked;
	HANDLE			hPicture;



	CellFormat()
	{
		colorText = RGB(0,0,0);
		colorCellBackground = RGB(255,255,255);
		cellType = cellTextReadOnly;
		uTextAlign = DT_LEFT;
		bCellTextChange = FALSE;
		bEnabled = TRUE;
		bShowText = TRUE;
		bShowCtrl = TRUE;
		formatLevel = levelList;
		hPicture = NULL;
		lfWeight = FW_NORMAL;
		bChecked = FALSE;
		bShowHighlight = FALSE;
	}

	~CellFormat()
	{
		if ( hPicture != NULL )
		{
			DeleteObject(hPicture);
			hPicture = NULL;
		}
	}

	CellFormat* CopyFormat(CellFormatLevel formatLevel)
	{
		CellFormat* pClone = new CellFormat();

		(*pClone) = (*this);
		pClone->formatLevel = formatLevel;

		return pClone;
	}

	BOOL SetPicture(HANDLE hPic)
	{
		hPicture = hPic;
		return TRUE;
	}

	BOOL SetPicture(CString strPictureFile)
	{
		if ( strPictureFile.IsEmpty() )
			return FALSE;

		if ( strPictureFile.Right(3).CompareNoCase(_T("bmp")) == 0 )
		{
			hPicture = ::LoadImage(::GetModuleHandle(NULL), 
				strPictureFile,IMAGE_BITMAP,0,0, LR_CREATEDIBSECTION|LR_LOADFROMFILE);
		}
		else if ( strPictureFile.Right(3).CompareNoCase(_T("ico")) == 0 )
		{
			hPicture = ::LoadImage(::GetModuleHandle(NULL), 
				strPictureFile,IMAGE_ICON,0,0, LR_CREATEDIBSECTION|LR_LOADFROMFILE);
		}
		else
		{
			SHFILEINFO    sfi;
			HIMAGELIST hImagelist;
			hImagelist=(HIMAGELIST)SHGetFileInfo(_T(""), 
				0,
				&sfi, 
				sizeof(SHFILEINFO), 
				SHGFI_SYSICONINDEX | SHGFI_SMALLICON ) ;

			CImageList* pImageList = CImageList::FromHandle(hImagelist);

			int nToken1 = strPictureFile.ReverseFind(_T('.'));
			int nToken2 = strPictureFile.ReverseFind(_T('\\'));
			CString strExt, strDot;
			if ( nToken1 > nToken2 )
			{
				strExt = strPictureFile.Right(strPictureFile.GetLength()-nToken1-1);
				strDot = _T(".");
			}
			else
			{
				strExt = _T("file");
				strDot = _T("");
			}

			if ( strExt.CompareNoCase(_T("exe")) != 0 )
			{
				strPictureFile = strDot + strExt;
			}

			SHFILEINFO sInfo;
			SHGetFileInfo(strPictureFile, FILE_ATTRIBUTE_NORMAL, &sInfo, sizeof(sInfo),
				SHGFI_USEFILEATTRIBUTES|SHGFI_SMALLICON|SHGFI_SYSICONINDEX);

			hPicture = pImageList->ExtractIcon(sInfo.iIcon);

			DeleteObject(hImagelist);
		}

		return TRUE;
	}

	BOOL SetPicture(UINT nIDResource )
	{
		if ( cellType == cellPictureICON )
		{
			hPicture = ::LoadImage(::GetModuleHandle(NULL), 
				MAKEINTRESOURCE(nIDResource),IMAGE_ICON,0,0, LR_CREATEDIBSECTION);
		}
		else
		{
			hPicture = ::LoadImage(::GetModuleHandle(NULL), 
				MAKEINTRESOURCE(nIDResource),IMAGE_BITMAP,0,0, LR_CREATEDIBSECTION);
		}
		

		return  TRUE;

	}
};

struct RowItemData
{
	map<int,CellFormat*>	mapCellFormat;
	map<int,CString>		mapCellText;
	map<CString,CString>	mapAdditionalData;
	CellFormat*				pDefaultRowFormat;
	BOOL					bSelected;
	int						nCacheIndex;

	RowItemData()
	{
		pDefaultRowFormat	= NULL;
		bSelected = FALSE;
		nCacheIndex = -1;
	}

	~RowItemData()
	{
		if ( pDefaultRowFormat )
		{
			delete pDefaultRowFormat;
			pDefaultRowFormat = NULL;
		}

		map<int, CellFormat*>::iterator it;
		for (it = mapCellFormat.begin(); it != mapCellFormat.end(); it ++)
		{
			CellFormat* pCellFormat = it->second;
			delete pCellFormat;
		}
		mapCellFormat.clear();
	}

	CellFormat*	FindCellFormat(int nCol, BOOL bUseRowFormatIfNOTFind)
	{
		map<int, CellFormat*>::iterator it = mapCellFormat.find(nCol);
		if ( it == mapCellFormat.end() )
		{
			if ( bUseRowFormatIfNOTFind )
				return pDefaultRowFormat;
			else
				return NULL;
		}
		else
		{
			return it->second;
		}
	}

	void SetCellFormat(int nCol, CellFormat* pCellFormat)
	{
		map<int, CellFormat*>::iterator it = mapCellFormat.find(nCol);
		if ( it != mapCellFormat.end() )
		{
			// The format is set before, delete it first
			CellFormat* pOldFormat = it->second;
			ASSERT(pOldFormat);
			delete pOldFormat;
			mapCellFormat.erase(it);
		}

		// Set it if the given format is NOT NULL
		if ( pCellFormat != NULL )
		{
			ASSERT(pCellFormat->formatLevel == levelCell);
			mapCellFormat[nCol] = pCellFormat;
		}
	}

	void SetCellText(int nCol, CString strText)
	{
		mapCellText[nCol] = strText;
	}

	CString GetCellText(int nCol)
	{
		return mapCellText[nCol];
	}
};

// notify struct
struct NMCUSTOMHDR : public NMHDR
{
	int		nRow;
	int		nCol;
	char*	szNewText;
	char*	szOldText;
};

enum CustomListMenuID
{
	menuSeperator					= 0,
	menuDeleteAllSelectedItem		= 1,
	menuInsertItemBeforeThisRow		= 2,
	menuAppendItemToBottom			= 3,
	menuMoveItemTop					= 4,
	menuMoveItemUp					= 5,
	menuMoveItemDown				= 6,
	menuMoveItemBottom				= 7,
	menuCopy						= 8,
	menuSelectAll					= 9,
	menuSelectNone					= 10,
};

struct CustomListMenuItem
{
	CustomListMenuID	nMenuID;
	CString				strMenuText;

	void Reset()
	{
		nMenuID = menuSeperator;
		strMenuText = _T("");
	}
};

struct CustomListContextMenu
{
	vector<CustomListMenuItem> vecContextMenu;

	void Clear()
	{
		vecContextMenu.clear();
	}

	void CreateDefaultMenu()
	{
		FromString(_T("08Copy*_*00*_*01Delete All Selected Items*_*00*_*02Insert Item Before This Row*_*03Append Item To Bottom*_*00*_*04Move Top*_*05Move Up*_*06Move Down*_*07Move Bottom*_*00*_*09Select All*_*10Select None*_*"));
	}

	BOOL FromString(CString strMenuInfo)
	{
		CustomListMenuItem menuItem;
		CString strMenuItemInfo;
		int nLen;

		int nPos = strMenuInfo.Find("*_*");
		while ( nPos != -1 )
		{
			menuItem.Reset();
			strMenuItemInfo = strMenuInfo.Left(nPos);
			nLen = strMenuItemInfo.GetLength();
			if (  nLen < 2 )
				return FALSE;

			menuItem.nMenuID = (CustomListMenuID)atoi(strMenuItemInfo.Left(2));	// Max 99 menu items
			menuItem.strMenuText = strMenuItemInfo.Right(nLen-2);
			vecContextMenu.push_back(menuItem);

			strMenuInfo = strMenuInfo.Right(strMenuInfo.GetLength()-nPos-3);
			nPos = strMenuInfo.Find("*_*");
		}

		return TRUE;
	}

	CString ToString()
	{
		CString strMenuInfo, strMenuItemInfo;
		int nCount = vecContextMenu.size();
		for ( int i = 0; i < nCount; i ++ )
		{
			strMenuItemInfo.Format(_T("%02d%s*_*"), vecContextMenu[i].nMenuID, vecContextMenu[i].strMenuText);
			strMenuInfo += strMenuItemInfo;
		}
		
		return strMenuInfo;
	}
};

struct CustomListStyle
{
	CustomListContextMenu contextMenu;
	vector<COLORREF>	vecRowColors;
	CustomColumnList	gridColumnsList;
	CString				strColumnOrder;

	BOOL			bEditFirstCellAfterNewRowIsAdded;
	BOOL			bEditNextCellAfterReturnIsHit;
	BOOL			bClickEmptyAreaToAddRow;
	BOOL			bShowSelection;
	BOOL			bIgnoreMouse;
	BOOL			bAllowMultlSelection;
	BOOL			bHighlightChangedCellText;
	BOOL			bClickSelectedCellToEdit;
	BOOL			bEnablePopupMenu;
	BOOL			bEnableFilter;

	int				nRowHeight;
	int				nHeaderHeight;

	void Reset()
	{
		contextMenu.Clear();
		vecRowColors.clear();
		gridColumnsList.clear();
		strColumnOrder = _T("");

		bEditFirstCellAfterNewRowIsAdded = TRUE;
		bEditNextCellAfterReturnIsHit = FALSE;
		bClickEmptyAreaToAddRow = FALSE;
		bShowSelection = TRUE;
		bIgnoreMouse = FALSE;
		bAllowMultlSelection = FALSE;
		bHighlightChangedCellText = TRUE;
		bClickSelectedCellToEdit = FALSE;
		bEnableFilter = TRUE;

		nRowHeight = 24;
		nHeaderHeight = 48;
	}

	void AddColumn(CString strColumnInfo)
	{
		CustomColumn		gridColumn;
		gridColumn.FromString(strColumnInfo);
		gridColumnsList.push_back(gridColumn);

		CString strOrder;
		strOrder.Format(_T("%d;"), gridColumnsList.size()-1);
		strColumnOrder += strOrder;
	}
};

class ICustomListUser
{
public:
	virtual BOOL OnCellTextChanged(CListCtrl* pListCtrl, int nRow, int nCol){ return TRUE; };
	virtual BOOL OnCellEditorDisplayed(CListCtrl* pListCtrl, int nRow, int nCol, CellFormat* pCellFormat, int& nMaxLen, CString& strValidChars){ return TRUE; };
	virtual BOOL OnCellComboDisplayed(CListCtrl* pListCtrl, int nRow, int nCol, CellFormat* pCellFormat, CStringList& strComboItem){ return TRUE; };
	virtual BOOL OnFilterTextChanged(CListCtrl* pListCtrl, int nCol, const char* szNewText){ return TRUE; };
	virtual BOOL OnCellTextChanging(CListCtrl* pListCtrl, int nRow, int nCol, CellFormat* pCellFormat, const CString& strOldValue, CString& strNewValue){ return TRUE; };
	virtual BOOL OnCellChangeCancelled(CListCtrl* pListCtrl, int nRow, int nCol, CellFormat* pCellFormat, const CString& strOldValue, CString& strNewValue){ return TRUE; };
	virtual BOOL OnCellCtrlClicked(CListCtrl* pListCtrl, int nRow, int nCol, CellFormat* pCellFormat){ return TRUE; };
	virtual BOOL OnRowLClicked(CListCtrl* pListCtrl, int nRow, int nCol, UINT nFlags, CPoint point){ return TRUE; };
	virtual BOOL OnRowLDblClicked(CListCtrl* pListCtrl, int nRow, int nCol, UINT nFlags, CPoint point){ return TRUE; };
	virtual BOOL OnRowRClicked(CListCtrl* pListCtrl, int nRow, int nCol, UINT nFlags, CPoint point){ return TRUE; };
	virtual BOOL OnRowDeleting(CListCtrl* pListCtrl, int nRow) { return TRUE; }
	virtual BOOL OnRowDeleted(CListCtrl* pListCtrl, int nRow) { return TRUE; }
	virtual BOOL OnRowSwapped(CListCtrl* pListCtrl, int nRow1, int nRow2) { return TRUE; }
	virtual BOOL OnRowAdded(CListCtrl* pListCtrl, int nRow) { return TRUE; }
	virtual BOOL OnKeyHitted(CListCtrl* pListCtrl, UINT uKey) { return TRUE; }
	virtual BOOL OnVirtualKeyHitted(CListCtrl* pListCtrl, UINT uVirtualKey) { return TRUE; }
	virtual BOOL OnSort(CListCtrl* pListCtrl, RowItemData* pid1, RowItemData* pid2, int& nCompareResult) { return FALSE; }
};


class ICacheList
{
public:
	virtual void		UpdateTableSytle(CustomListStyle& listStyle){};
	virtual void		SetEmptyTable(BOOL bEmptyDataCache){};
	virtual void		SetCacheData(I2DCache<CString>* pCache){};
	virtual void		SetTableStyle(CustomListStyle& listStyle){};
	virtual void		FilterDataFromCache(){};
	virtual void		FilterDataFromCache(const vector<CString>& vecFilter){};
	virtual void		SetQueryStyle(BOOL bCaseSensitive, BOOL bQueryWhenEditting){};
	virtual void		RebuildContextMenu(const CustomListContextMenu& contextMenu){};
};