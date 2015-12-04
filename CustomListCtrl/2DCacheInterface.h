#pragma once

#include <map>
#include <vector>
#include <list>
#include <algorithm>
using namespace std;

enum RowStatus
{
	rowUnknown			= 0,
	rowNew				= 1,
	rowLoaded			= 2,
	rowHidden			= 3,
	rowDeleted			= 4,
};

template <typename T>
class I2DCache
{
public:
	virtual ~I2DCache(){};
public:
	virtual void		CacheStartProtect(){};
	virtual void		CacheEndProtect(){};
	virtual void		CacheSetEmptyValue(T valueEmtpy){};
	virtual void		CacheSetColumnCount(){};
	virtual int			CacheAppendEmptyRow(RowStatus rowStatus){return -1;};
	virtual void		CacheDeleteRow(int nRow){};
	virtual void		CacheSetItemText(int nRow, int nCol, const T& strText){};
	virtual T			CacheGetItemText(int nRow, int nCol) = 0;
	virtual RowStatus	CacheGetRowstatus(int nRow){return rowUnknown;};
	virtual void		CacheSetRowstatus(int nRow, RowStatus rowstatus){};
	virtual void		CacheFilterData(const vector<T>& vecFilter){};

	virtual void		CacheStartColumnOperation(){};
	virtual void		CacheEndColumnOperation(){};
	virtual void		CacheInsertColumn(int nCol){};
	virtual void		CacheDeleteColumn(int nCol){};
	virtual void		CacheSwapColumn(int nCol1, int nCol2){};

	virtual int			CacheGetRowCount(){return -1;};
	virtual int			CacheGetColCount(){return -1;};
	virtual void		CacheClearTable(BOOL bClearData){};
};
