#pragma once
#include "2DCacheInterface.h"

template <typename T>
class CTableCache :	public I2DCache<T>
{
public:
	CTableCache()
	{
		m_bProtectDataCache = FALSE;
//		m_nRowCount = 0;
		m_nColCount = 0;
	}

	~CTableCache()
	{
		for ( int i = 0; i < (int)m_vecDataCache.size(); i++ )
		{
			m_vecDataCache[i].clear();
		}
		m_vecDataCache.clear();

		for ( int i = 0; i < (int)m_vecDataCacheColumnOp.size(); i++)
		{
			m_vecDataCacheColumnOp[i].clear();
		}
		m_vecDataCacheColumnOp.clear();

		m_vecRowstatus.clear();
	}

private:
	vector<vector<T>> m_vecDataCache;
	vector<vector<T>> m_vecDataCacheColumnOp;
	vector<RowStatus> m_vecRowstatus; 

	BOOL	m_bProtectDataCache;
	int		m_nColCount;
	int		m_nColCountColumnOp;

	T		m_valueEmpty;

public:
	
	// Define the I2DCache interface
public:
	virtual void		CacheStartProtect(){m_bProtectDataCache=TRUE;};
	virtual void		CacheEndProtect(){m_bProtectDataCache=FALSE;};
	virtual void		CacheSetEmptyValue(T valueEmtpy) { m_valueEmpty = valueEmtpy; }
	virtual void		CacheSetColumnCount(int nColCount){ m_nColCount = nColCount; }
	virtual int			CacheAppendEmptyRow(RowStatus rowStatus);
	virtual void		CacheDeleteRow(int nRow);
	virtual void		CacheSetItemText(int nRow, int nCol, const T& strText);
	virtual T			CacheGetItemText(int nRow, int nCol);
	virtual RowStatus	CacheGetRowstatus(int nRow);
	virtual void		CacheSetRowstatus(int nRow, RowStatus rowstatus);
	virtual void		CacheFilterData(const vector<T>& vecFilter);
	
	virtual void		CacheStartColumnOperation();
	virtual void		CacheEndColumnOperation();
	virtual void		CacheInsertColumn(int nCol);
	virtual void		CacheDeleteColumn(int nCol);
	virtual void		CacheSwapColumn(int nCol1, int nCol2);

	virtual int			CacheGetRowCount();
	virtual int			CacheGetColCount();
	virtual void		CacheClearTable(BOOL bClearData);
};

template <typename T>
int CTableCache<T>::CacheAppendEmptyRow(RowStatus rowStatus)
{
	if ( m_bProtectDataCache )
	{
		return -1;
	}

	vector<T>	vecData;
	for ( int i = 0; i < m_nColCount; i ++ )
	{
		vecData.push_back(m_valueEmpty);
	}
	m_vecDataCache.push_back(vecData);

	m_vecRowstatus.push_back(rowStatus);

	return m_vecDataCache.size()-1;
}

template <typename T>
void CTableCache<T>::CacheDeleteRow(int nRow)
{
	if ( m_bProtectDataCache )
	{
		return;
	}

	if ( nRow >= (int)m_vecDataCache.size() || nRow < 0 )
		return;

	m_vecRowstatus[nRow] = rowDeleted;
}

template <typename T>
void CTableCache<T>::CacheSetItemText(int nRow, int nCol, const T& strText)
{
	if ( m_bProtectDataCache )
	{
		return;
	}

	if ( nRow >= (int)m_vecDataCache.size() || nRow < 0 )
		return;

	if ( nCol >= m_nColCount || m_nColCount < 0 )
		return;


	m_vecDataCache[nRow][nCol] = strText;
}

template <typename T>
T CTableCache<T>::CacheGetItemText(int nRow, int nCol)
{
	if ( nRow >= (int)m_vecDataCache.size() || nRow < 0 )
		return m_valueEmpty;

	if ( nCol >= m_nColCount || m_nColCount < 0 )
		return m_valueEmpty;

	return m_vecDataCache[nRow][nCol];
}

template <typename T>
RowStatus CTableCache<T>::CacheGetRowstatus(int nRow)
{
	if ( nRow >= (int)m_vecRowstatus.size() || nRow < 0 )
		return rowUnknown;

	return m_vecRowstatus[nRow];
}

template <typename T>
void CTableCache<T>::CacheSetRowstatus(int nRow, RowStatus rowstatus)
{
	if ( nRow >= (int)m_vecRowstatus.size() || nRow < 0 )
		return;

	m_vecRowstatus[nRow] = rowstatus;
}

template <typename T>
void CTableCache<T>::CacheStartColumnOperation()
{
	m_vecDataCacheColumnOp = m_vecDataCache;
	m_nColCountColumnOp = m_nColCount;
}

template <typename T>
void CTableCache<T>::CacheEndColumnOperation()
{
	m_vecDataCache = m_vecDataCacheColumnOp;
	m_nColCount = m_nColCountColumnOp;
}

template <typename T>
void CTableCache<T>::CacheInsertColumn(int nCol)
{
	m_nColCountColumnOp++;
	int nCount = m_vecDataCacheColumnOp.size();
	if ( nCount == 0 )
		return;

	int nSubCount = m_vecDataCacheColumnOp[0].size();
	ASSERT(nSubCount==m_nColCountColumnOp-1);

	if ( nCol == nSubCount)
	{
		// We are inserting a new column to the last, so only need to append an empty value
		for ( int i = 0; i < nCount; i ++ )
		{
			m_vecDataCacheColumnOp[i].push_back(m_valueEmpty);
		}
	}
	else
	{
		for ( int i = 0; i < nCount; i ++ )
		{
			m_vecDataCacheColumnOp[i].push_back(m_vecDataCacheColumnOp[i][nSubCount-1]);
			for ( int j = nSubCount-1; j > nCol; j -- )
			{
				m_vecDataCacheColumnOp[i][j] = m_vecDataCacheColumnOp[i][j-1];
			}

			m_vecDataCacheColumnOp[i][nCol] = m_valueEmpty;
		}
	}
}

template <typename T>
void CTableCache<T>::CacheDeleteColumn(int nCol)
{
	m_nColCountColumnOp--;
	int nCount = m_vecDataCacheColumnOp.size();
	if ( nCount == 0 )
		return;

	int nSubCount = m_vecDataCacheColumnOp[0].size();
	ASSERT(nSubCount==m_nColCountColumnOp+1);
	for ( int i = 0; i < nCount; i ++ )
	{
		for ( int j = nCol; j < nSubCount-1; j ++ )
		{
			m_vecDataCacheColumnOp[i][j] = m_vecDataCacheColumnOp[i][j+1];
		}

		m_vecDataCacheColumnOp[i].pop_back();
	}
}

template <typename T>
void CTableCache<T>::CacheSwapColumn(int nCol1, int nCol2)
{
	int nCount = m_vecDataCacheColumnOp.size();
	CString strTemp;
	for ( int i = 0; i < nCount; i ++ )
	{
		strTemp = m_vecDataCacheColumnOp[i][nCol1];
		m_vecDataCacheColumnOp[i][nCol1] = m_vecDataCacheColumnOp[i][nCol2];
		m_vecDataCacheColumnOp[i][nCol2] = strTemp;
	}
}

template <typename T>
void CTableCache<T>::CacheClearTable(BOOL bClearData)
{
	if ( bClearData )
	{
		m_vecDataCache.clear();
		m_vecDataCacheColumnOp.clear();
	}
}

template <typename T>
int	CTableCache<T>::CacheGetRowCount()
{
	return m_vecDataCache.size();
}

template <typename T>
int CTableCache<T>::CacheGetColCount()
{
	return m_nColCount;
}

template <typename T>
void CTableCache<T>::CacheFilterData(const vector<T>& vecFilter)
{
	
}