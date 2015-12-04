#pragma once

// safe release and delete utilities
#define SAFE_RELEASE(x) \
	if (x!=NULL){\
	x->Release();\
	x=NULL;\
	}

#define SAFE_DELETE(x) \
	if (x!=NULL){\
	delete x;\
	x=NULL;\
	}

#define SAFE_FREE(x) \
	if (x!=NULL){\
	::SysFreeString(x);\
	x=NULL;\
	}

#define ASSERT_VALID_STRING( str ) ASSERT( !IsBadStringPtr( str, 0xfffff ) )


// external notify messages
#define MSG_CUSTOM_LIST_CELL_TEXT_CHANGED			120
#define	MSG_CUSTOM_LIST_FILTER_TEXT_CHANGED			121
#define MSG_CUSTOM_LIST_CELL_CTRL_CLICKED			122


#define	FLCN_FILTERCHANGING			101
#define	FLCN_SHOWINGEDIT			103
#define	FLCN_BEGINFILTEREDIT		104
#define	FLCN_ENDFILTEREDIT			105
#define	FLCN_LIST_LBUTTON_CLICK		106
#define FLCN_LIST_LBUTTON_DBCLICK	107
#define	FLCN_SHOWINGCOMBO			110

// const
#define FILTER_NONE				0
#define FILTER_ENABLED			1
#define FILTER_DISABLED			2

// Filter Edit Control ID
#define IDC_ED_EDIT				101

// internal messages
#define	FLM_EDITTEXTCHANGING	(WM_USER + 100)
#define	FLM_EDITTEXTCHANGED		(WM_USER + 101)

#define	FLM_COMBOSELCHANGING	(WM_USER + 102)
#define	FLM_COMBOSELCHANGED		(WM_USER + 103)

#define	FLM_DATETIMECHANGING	(WM_USER + 104)
#define	FLM_DATETIMECHANGED		(WM_USER + 105)

#define WM_SET_COMBOITEMS		(WM_USER + 106)
#define WM_GET_COLVALIDCHARS	(WM_USER + 107)


// UI Style
#define COLOR_ACQ_CORDEREDIT_BK				RGB(255,255,255)
#define COLOR_ACQ_CORDEREDIT_BK_DISABLED	RGB(240,240,240)
#define COLOR_ACQ_CORDEREDIT_TEXT			RGB(255,0,0)
#define COLOR_CHANGED_CELL_TEXT				RGB(255,0,0)
#define COLOR_FILTER_BK						RGB(207,207,207)

#define CTRL_A	0x1
#define CTRL_C	0x3
#define CTRL_V	0x16
#define CTRL_X	0x18

