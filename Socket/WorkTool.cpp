#include "Stdafx.h"
#include "WorkTool.h"
#include "stdio.h"
#include "stdlib.h"
#include "tchar.h"

CWorkTool::CWorkTool(IWorkToolUser* pUser)
{
	m_bExit = FALSE;
	m_pUser = pUser;
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

BOOL CWorkTool::RecvEmptyDir()
{
	CString strDir;
	if (!m_socketTool.RecvStrValue(strDir))
		return FALSE;

	if (m_pUser)
		return m_pUser->OnEmptyDirReceived(strDir);

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
		if (m_pUser == NULL)
		{
			strOutput = _M("Internal error!");
			return FALSE;
		}

		m_pUser->OnNewMessage(_M("Data format:") + strActualClipboardFormat, TRUE);

		int nDirCount = 0;
		int nFileCount = 0;
		socketTool.RecvIntValue(nDirCount);
		socketTool.RecvIntValue(nFileCount);

		strOutput.Format(_M("\r\nTotal %d directories, %d files"), nDirCount, nFileCount);
		m_pUser->OnNewMessage(strOutput, FALSE);


		int aaa;
		CString strTemp;
		for (int i = 0; i < 13; i++)
		{
			m_socketTool.RecvIntValue(aaa);
			strTemp.Format("%d", aaa);
			m_pUser->OnNewMessage(strTemp, FALSE);
		}
		//socketTool.RecvFile("C:\\11.txt");

		//socketTool.RecvFile("C:\\12.txt");

		//socketTool.RecvFile("C:\\13.txt");

		//socketTool.RecvFile("C:\\14.txt");

		//socketTool.RecvFile("C:\\15.txt");

		//socketTool.RecvFile("C:\\16.txt");

		//socketTool.RecvFile("C:\\17.txt");

		//socketTool.RecvFile("C:\\18.txt");

		//socketTool.RecvFile("C:\\19.txt");

		//socketTool.RecvFile("C:\\20.txt");

		//socketTool.RecvFile("C:\\21.txt");

		//socketTool.RecvFile("C:\\22.txt");

		//socketTool.RecvFile("C:\\23.txt");
		//socketTool.RecvFile("C:\\24.txt");


		
		//CString strDirName;
		//for (int i = 0; i < nDirCount; i++)
		//{
		//	if (!socketTool.RecvStrValue(strDirName))
		//		return FALSE;

		//	m_pUser->OnEmptyDirReceived(strDirName);
		//}

		//CString strFileName;
		//for (int i = 0; i < nFileCount; i++)
		//{
		//	if (!socketTool.RecvStrValue(strFileName))
		//		return FALSE;
		//}

		//CString strFileName;
		//for (int i = 0; i < nCount; i++)
		//{
		//	socketTool.RecvStrValue(strFileName);
		//	if (m_pUser)
		//		m_pUser->OnNewMessage(strFileName, FALSE);
		//}

		m_pUser->OnNewMessage("\r\n", FALSE);
		
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

			m_socketTool.SendStrValue((LPTSTR)(LPCTSTR)_M("Ansi Text"));
			m_socketTool.SendStrValue((LPTSTR)(LPCTSTR)strData);
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

			m_socketTool.SendStrValue((LPTSTR)(LPCTSTR)_M("Unicode Text"));
			m_socketTool.SendStrValue((LPTSTR)(LPCTSTR)strData);
		}
		else if (::IsClipboardFormatAvailable(CF_HDROP))
		{
			if (m_pUser)
			{
				CDiskFile diskFile;
				if (m_pUser->OnGetAllDirFilesFromClipboard(diskFile))
				{
					m_socketTool.SendStrValue((LPTSTR)(LPCTSTR)_M("Directory|File"));
					m_socketTool.SendIntValue((int)diskFile.vecDirectory.size());
					m_socketTool.SendIntValue((int)diskFile.vecFile.size());
				}

				//for (int i = 0; i < 14; i++)
				//{
				//	m_socketTool.SendFile("C:\\nlog.txt");
				//}

				for (int i = 0; i < 13; i++)
				{
					m_socketTool.SendIntValue(i);
				}
				//for (int i = 0; i < (int)diskFile.vecDirectory.size(); i++)
				//{
				//	m_socketTool.SendStrValue(diskFile.vecDirectory[i].GetBuffer(0));
				//	Sleep(2);
				//}

				//for (int i = 0; i < (int)diskFile.vecFile.size(); i++)
				//{
				//	m_socketTool.SendStrValue((LPTSTR)(LPCTSTR)diskFile.vecFile[i]);
				//}
			}
			//UINT cFiles = 0;
			//HDROP hDrop = HDROP(GetClipboardData(CF_HDROP));
			//if (hDrop)
			//{
			//	m_socketTool.SendStrValue((LPTSTR)(LPCTSTR)_M("Directory|File"));

			//	cFiles = DragQueryFile(hDrop, (UINT)-1, NULL, 0);

			//	m_socketTool.SendIntValue((int)cFiles);

			//	char szFile[MAX_PATH];
			//	for (UINT count = 0; count < cFiles; count++)
			//	{
			//		DragQueryFile(hDrop, count, szFile, sizeof(szFile));
			//		m_socketTool.SendStrValue(szFile);
			//	}
			//}
		}

		::CloseClipboard();
	}

	return TRUE;
}