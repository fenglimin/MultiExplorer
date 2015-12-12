#include "Stdafx.h"
#include "WorkTool.h"
#include "stdio.h"
#include "stdlib.h"
#include "tchar.h"

CWorkTool::CWorkTool(IWorkToolServerUser* pServerUser)
{
	m_bExit = FALSE;
	m_pServerUser = pServerUser;
	m_pClientUser = NULL;
}

CWorkTool::~CWorkTool()
{
}


BOOL CWorkTool::StartWorking( int nPort )
{
	CSocketTool::SetTimeOut(60000);

	if ( !m_socketListen.Init ( nPort ) )
		return FALSE;

	while ( !m_bExit )
	{
		// Accept connecting
		if ( !m_socketListen.PendingAccept() )
		{
			if ( m_socketListen.m_bSocketGood )
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
		m_socketListen.CloseSocket();

		if (!m_socketListen.Init(nPort))
			return FALSE;

/*		if ( !bOk )
		{
			closesocket ( m_socketListen.m_socketListen );
			m_socketListen.m_socketListen = INVALID_SOCKET;

			if ( m_socketListen.m_bSocketGood )
				return TRUE;

			if ( !m_socketListen.Init(m_socketListen.m_nPort) )
				return FALSE;
		}*/	
	}

	return TRUE;
}

BOOL CWorkTool::PendingRead()
{
	int nFlag;
	if ( !m_socketListen.RecvIntValue ( nFlag ) )
		return FALSE;
	
	switch ( nFlag )
	{
	case NETWORK_GET_CLIPBOARD_DATA: Response_GetClipboardData();
		break;
	}

	if ( m_socketListen.m_bSocketGood )
	{
		int nTaskEndFlag;
		m_socketListen.RecvIntValue(nTaskEndFlag);
		if (nTaskEndFlag != NETWORK_TASK_END)
			return FALSE;
	}

	return TRUE;
}

//BOOL CWorkTool::NetPeek_Chatting()
//{
//	char strValue[256];
//	int  nLen = 256;
//
//	BOOL bOk = m_socketListen.RecvStrValue ( strValue, nLen );
////	if ( bOk )
////		MessageBox ( NULL, strValue, "Chat", MB_OK );
//	
//	return bOk;
//}


//BOOL CWorkTool::NetPeek_RecvFile()
//{
//	char strFileFullName[MAX_PATH];
//	int  nLen = MAX_PATH;
//	if (!m_socketListen.RecvStrValue(strFileFullName, nLen))
//		return FALSE;
//
//	return m_socketListen.RecvFile(strFileFullName);
//}
//
//BOOL CWorkTool::NetPeek_SendFile()
//{
//	char strFileFullName[MAX_PATH];
//	int  nFileLen = MAX_PATH;
//
//	if (!m_socketListen.RecvStrValue(strFileFullName, nFileLen))
//		return FALSE;
//
//	return m_socketListen.SendFile(strFileFullName);
//}

BOOL CWorkTool::RecvEmptyDir()
{
	CString strDir;
	if (!m_socketListen.RecvStrValue(strDir))
		return FALSE;

	if (m_pClientUser)
		return m_pClientUser->OnEmptyDirReceived(strDir);

	return FALSE;
}

BOOL CWorkTool::Request_GetClipboardData(CString strIp, int nPort, int nFormat, CString& strOutput)
{
	CSocketTool socketTool;
	socketTool.SetTimeOut(60000);

	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);
	setsockopt(socketTool.m_socketWork, IPPROTO_TCP, SO_KEEPALIVE, (char*)&bOptVal, bOptLen);

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

	CString strActualClipboardFormat;
	if (!socketTool.RecvStrValue(strActualClipboardFormat))
	{
		strOutput.Format(_M("Receive actual  clipboard format failed!"));
		return FALSE;
	}

	if (strActualClipboardFormat == _M("Ansi Text") || strActualClipboardFormat == _M("Unicode Text"))
	{
		if (!socketTool.RecvStrValue(strOutput))
		{
			strOutput.Format(_M("Received clipboard failed!"));
			return FALSE;
		}

		strOutput = _M("Data format:") + strActualClipboardFormat + "\r\n\r\n" + strOutput;
	}
	else if (strActualClipboardFormat == _M("Directory|File"))
	{
		if (m_pClientUser == NULL)
		{
			strOutput = _M("Internal error!");
			return FALSE;
		}

		m_pClientUser->OnNewMessage(_M("Data format:") + strActualClipboardFormat, TRUE);

		int nDirCount = 0;
		int nFileCount = 0;
		socketTool.RecvIntValue(nDirCount);
		socketTool.RecvIntValue(nFileCount);

		strOutput.Format(_M("\r\nTotal %d directories, %d files"), nDirCount, nFileCount);
		m_pClientUser->OnNewMessage(strOutput, FALSE);

		// Create directory
		CString strDirName;
		for (int i = 0; i < nDirCount; i++)
		{
			if (!socketTool.RecvStrValue(strDirName))
				return FALSE;

			m_pClientUser->OnEmptyDirReceived("C:\\temp\\" + strDirName);
		}

		CString strFileName;
		for (int i = 0; i < nFileCount; i++)
		{
			if (!socketTool.RecvStrValue(strFileName))
				return FALSE;
			m_pClientUser->OnEmptyDirReceived(strFileName);

			socketTool.RecvFile((LPTSTR)(LPCTSTR)("C:\\temp\\" + strFileName));
		}

		m_pClientUser->OnNewMessage("\r\n", FALSE);
		
		strOutput = "";
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
	m_socketListen.RecvIntValue(nFormat);

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

			m_socketListen.SendStrValue((LPTSTR)(LPCTSTR)_M("Ansi Text"));
			m_socketListen.SendStrValue((LPTSTR)(LPCTSTR)strData);
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

			m_socketListen.SendStrValue((LPTSTR)(LPCTSTR)_M("Unicode Text"));
			m_socketListen.SendStrValue((LPTSTR)(LPCTSTR)strData);
		}
		else if (::IsClipboardFormatAvailable(CF_HDROP))
		{
			if (m_pServerUser)
			{
				CDiskFile diskFile;
				if (m_pServerUser->OnGetAllDirFilesFromClipboard(diskFile))
				{
					m_socketListen.SendStrValue((LPTSTR)(LPCTSTR)_M("Directory|File"));
					m_socketListen.SendIntValue((int)diskFile.vecDirectory.size());
					m_socketListen.SendIntValue((int)diskFile.vecFile.size());
				}

				for (int i = 0; i < (int)diskFile.vecDirectory.size(); i++)
				{
					m_socketListen.SendStrValue(diskFile.vecDirectory[i].GetBuffer(0));
				}

				for (int i = 0; i < (int)diskFile.vecFile.size(); i++)
				{
					m_socketListen.SendStrValue((LPTSTR)(LPCTSTR)diskFile.vecFile[i]);
					m_socketListen.SendFile((LPTSTR)(LPCTSTR)(diskFile.strWorkDir + diskFile.vecFile[i]));
				}
			}
		}

		::CloseClipboard();
	}

	return TRUE;
}