#include "Stdafx.h"
#include "WorkTool.h"
#include "stdio.h"
#include "stdlib.h"
#include "tchar.h"

CWorkTool::CWorkTool()
{
	m_bExit = FALSE;
}

CWorkTool::~CWorkTool()
{
}


BOOL CWorkTool::StartWorking( int nPort )
{
	CSocketTool::SetTimeOut ( 15 );

	if ( !m_socketTool.Init ( nPort ) )
		return FALSE;

	while ( !m_bExit )
	{
		// Accept connecting
		if ( !m_socketTool.PendingAccept() )
		{
			if ( m_socketTool.m_bSocketGood )
			{
				Sleep ( 500 );
				continue;
			}
			else
			{
				WSACleanup();
				return FALSE;
			}
		}

		// GetMesage and process
		BOOL bOk = PendingRead();

		// Release connecting
		closesocket ( m_socketTool.m_socketWork );

		if ( !bOk )
		{
			closesocket ( m_socketTool.m_socketListen );

			if ( m_socketTool.m_bSocketGood )
				return TRUE;

			if ( !m_socketTool.Init(m_socketTool.m_nPort) )
				return FALSE;
		}	
	}

	return TRUE;
}

BOOL CWorkTool::PendingRead()
{
	int nFlag;
	if ( !m_socketTool.RecvIntValue ( nFlag ) )
		return FALSE;
	
	switch ( nFlag )
	{
	case NETWORK_GET_CLIPBOARD_DATA: Response_GetClipboardData();
		break;
	}

	if ( m_socketTool.m_bSocketGood )
	{
		int nTaskEndFlag;
		m_socketTool.RecvIntValue(nTaskEndFlag);
		if (nTaskEndFlag != NETWORK_TASK_END)
			return FALSE;
	}

	return TRUE;
}

BOOL CWorkTool::NetPeek_Chatting()
{
	char strValue[256];
	int  nLen = 256;

	BOOL bOk = m_socketTool.RecvStrValue ( strValue, nLen );
//	if ( bOk )
//		MessageBox ( NULL, strValue, "Chat", MB_OK );
	
	return bOk;
}


BOOL CWorkTool::NetPeek_RecvFile()
{
	char strFileFullName[MAX_PATH];
	int  nLen = MAX_PATH;
	if (!m_socketTool.RecvStrValue(strFileFullName, nLen))
		return FALSE;

	return m_socketTool.RecvFile(strFileFullName);
}

BOOL CWorkTool::NetPeek_SendFile()
{
	char strFileFullName[MAX_PATH];
	int  nFileLen = MAX_PATH;

	if (!m_socketTool.RecvStrValue(strFileFullName, nFileLen))
		return FALSE;

	return m_socketTool.SendFile(strFileFullName);
}

BOOL CWorkTool::Request_GetClipboardData(CString strIp, int nPort, int nFormat, CString& strOutput)
{
	CSocketTool socketTool;

	if (!socketTool.Connect((LPTSTR)(LPCTSTR)strIp, nPort))
	{
		strOutput.Format(_M("Can Not connect to server, ip address : %s, Port : %d."), strIp, nPort);
		return FALSE;
	}

	if (!socketTool.SendIntValue(NETWORK_GET_CLIPBOARD_DATA))
	{
		strOutput.Format(_M("Send command(%s) failed!"), "NETWORK_GET_CLIPBOARD_DATA");
		return FALSE;
	}

	if (!socketTool.SendIntValue(nFormat))
	{
		strOutput.Format(_M("Send clipboard format failed!"));
		return FALSE;
	}

	if (!socketTool.RecvStrValue(strOutput))
	{
		strOutput.Format(_M("Received clipboard failed!"));
		return FALSE;
	}

	if (!socketTool.SendIntValue(NETWORK_TASK_END))
	{
		strOutput.Format(_M("Send command(%s) failed!"), "NETWORK_TASK_END");
		return FALSE;
	}

	return TRUE;
}

BOOL CWorkTool::Response_GetClipboardData()
{
	int nFormat = CF_TEXT;
	m_socketTool.RecvIntValue(nFormat);

	CString strData;
	if (::OpenClipboard(NULL))
	{
		if ( nFormat == CF_TEXT && ::IsClipboardFormatAvailable(CF_TEXT))
		{
			HANDLE hData = ::GetClipboardData(CF_TEXT);
			if (hData != NULL)
			{
				strData = (LPCSTR) ::GlobalLock(hData);
				::GlobalUnlock(hData);
			}
		}
		else if (nFormat == CF_UNICODETEXT && ::IsClipboardFormatAvailable(CF_UNICODETEXT))
		{
			HANDLE hData = ::GetClipboardData(CF_UNICODETEXT);
			if (hData != NULL)
			{
				WCHAR* pWchar = (WCHAR*) ::GlobalLock(hData);
				::GlobalUnlock(hData);

				strData = CString(pWchar);
			}
		}

		::CloseClipboard();
	}

	m_socketTool.SendStrValue((LPTSTR)(LPCTSTR)strData);

	return TRUE;
}