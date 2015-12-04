#pragma once

#include <map>
using namespace std;

#define _M(X)	(CMultiLanguage::Translate(X))

class CMultiLanguage
{
public:
	CMultiLanguage(void){};

	~CMultiLanguage(void){};

	static map<CString,CString>	m_mapTranslate;
	static BOOL		m_bEnglish;

public:
	static void SetLanguage(BOOL bEnglish);
	static CString Translate(const CString& strKey);
	static void Init();
	static void Clear();
};

class ILanguageUser
{
public:
	virtual void OnLanguageChange(){};
};