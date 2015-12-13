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

	m_bContinueTransfer = TRUE;
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


BOOL CWorkTool::Request_GetClipboardData(CString strIp, int nPort, int nFormat, CString& strErrorMsg)
{
	CSocketTool socketTool;
	socketTool.SetTimeOut(60000);

	BOOL bOptVal = TRUE;
	int bOptLen = sizeof(BOOL);
	setsockopt(socketTool.m_socketWork, IPPROTO_TCP, SO_KEEPALIVE, (char*)&bOptVal, bOptLen);

	if (!socketTool.Connect((LPTSTR)(LPCTSTR)strIp, nPort))
	{
		strErrorMsg.Format(_M("Can Not connect to server, ip address : %s, Port : %d."), strIp, nPort);
		return FALSE;
	}

	if (!socketTool.SendIntValue(NETWORK_GET_CLIPBOARD_DATA))
	{
		strErrorMsg.Format(_M("Send command(%s) failed!"), "NETWORK_GET_CLIPBOARD_DATA");
		return FALSE;
	}

	if (!socketTool.SendIntValue(nFormat))
	{
		strErrorMsg.Format(_M("Send clipboard format failed!"));
		return FALSE;
	}

	CString strActualClipboardFormat;
	if (!socketTool.RecvStrValue(strActualClipboardFormat))
	{
		strErrorMsg.Format(_M("Receive actual  clipboard format failed!"));
		return FALSE;
	}

	if (m_pClientUser == NULL)
	{
		strErrorMsg = _M("Internal error!");
		return FALSE;
	}

	BOOL bIsText = TRUE;
	CString strOutput;
	if (strActualClipboardFormat == _M("Ansi Text") || strActualClipboardFormat == _M("Unicode Text"))
	{
		if (!socketTool.RecvStrValue(strOutput))
		{
			strErrorMsg.Format(_M("Received clipboard failed!"));
			return FALSE;
		}

		m_pClientUser->OnNewMessage(_M("Data format : ") + strActualClipboardFormat, TRUE);
		m_pClientUser->OnNewMessage("\r\n" + strOutput + "\r\n", FALSE);
	}
	else if (strActualClipboardFormat == _M("Directory | File"))
	{
		bIsText = FALSE;
		m_bContinueTransfer = TRUE;
		m_pClientUser->OnNewMessage(_M("Data format : ") + strActualClipboardFormat, TRUE);

		int nDirCount = 0;
		int nFileCount = 0;
		int nTotalSizeInM = 0;
		socketTool.RecvIntValue(nDirCount);
		socketTool.RecvIntValue(nFileCount);
		socketTool.RecvIntValue(nTotalSizeInM);
	
		if (nTotalSizeInM > 100)
		{
			CString strMsg;
			strMsg.Format(_M("Total size of copied files in clipboard is larger than 100M(%dM), continue to copy?"), nTotalSizeInM);
			m_bContinueTransfer = AfxMessageBox(strMsg, MB_YESNO) == IDYES;
		}
		socketTool.SendIntValue((int)m_bContinueTransfer);

		CString strTempCopyPath;
		if (m_bContinueTransfer)
		{
			m_pClientUser->OnStart(bIsText);

			
			if (!CreateTempDirForCopy(socketTool.m_strTempPath, strIp, strTempCopyPath))
			{

			}

			if (nDirCount > 0)
				m_pClientUser->OnNewMessage(_M("Creating directories..."), TRUE);

			// Create directory
			CString strDirName;
			for (int i = 0; i < nDirCount; i++)
			{
				if (!socketTool.SendIntValue(m_bContinueTransfer))
					return FALSE;

				if (!m_bContinueTransfer)
					break;

				if (!socketTool.RecvStrValue(strDirName))
					return FALSE;

				CreateDirectory(strTempCopyPath + strDirName, NULL);

				m_pClientUser->OnNewMessage(strDirName, FALSE);
			}

			if (m_bContinueTransfer)
			{
				if (nFileCount > 0)
					m_pClientUser->OnNewMessage(_M("Copying files..."), TRUE);

				// Copy file
				CString strFileName;
				for (int i = 0; i < nFileCount; i++)
				{
					if (!socketTool.SendIntValue(m_bContinueTransfer))
						return FALSE;

					if (!m_bContinueTransfer)
						break;

					if (!socketTool.RecvStrValue(strFileName))
						return FALSE;

					socketTool.RecvFile((LPTSTR)(LPCTSTR)(strTempCopyPath + strFileName));
					m_pClientUser->OnNewMessage(strFileName, FALSE);
				}
			}

			if (m_bContinueTransfer)
			{
				strOutput.Format(_M("Transfer completed! Total %d directories, %d files, %d MB"), nDirCount, nFileCount, nTotalSizeInM);
				m_pClientUser->OnNewMessage(strOutput, TRUE);
				m_pClientUser->OnNewMessage(_M("Files were copied to ") + strTempCopyPath, TRUE);
			}
			else
			{
				m_pClientUser->OnNewMessage(_M("Transfer stopped by user!"), TRUE);
				m_pClientUser->OnNewMessage(_M("Please check directory ") + strTempCopyPath, TRUE);
			}
		}
		
		strOutput = strTempCopyPath;
	}
	else if (strActualClipboardFormat == _M("No data"))
	{
		// No data wanted
	}


	if (!socketTool.SendIntValue(NETWORK_TASK_END))
	{
		strErrorMsg.Format(_M("Send command(%s) failed!"), "NETWORK_TASK_END");
		return FALSE;
	}

	m_pClientUser->OnComplete(bIsText, strOutput);
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
				int nTotalSizeInM = 0;
				if (m_pServerUser->OnGetAllDirFilesFromClipboard(diskFile, nTotalSizeInM))
				{
					m_socketListen.SendStrValue((LPTSTR)(LPCTSTR)_M("Directory | File"));
					m_socketListen.SendIntValue((int)diskFile.vecDirectory.size());
					m_socketListen.SendIntValue((int)diskFile.vecFile.size());
					m_socketListen.SendIntValue(nTotalSizeInM);
				}

				int nContinue;
				m_socketListen.RecvIntValue(nContinue);
				if (nContinue == 1)
				{
					// Send direcotry name
					for (int i = 0; i < (int)diskFile.vecDirectory.size(); i++)
					{
						m_socketListen.RecvIntValue(nContinue);
						if (!nContinue)
							break;
						
						m_socketListen.SendStrValue(diskFile.vecDirectory[i].GetBuffer(0));
					}

					if (nContinue)
					{
						// Send file
						for (int i = 0; i < (int)diskFile.vecFile.size(); i++)
						{
							m_socketListen.RecvIntValue(nContinue);
							if (!nContinue)
								break;

							m_socketListen.SendStrValue((LPTSTR)(LPCTSTR)diskFile.vecFile[i]);
							m_socketListen.SendFile((LPTSTR)(LPCTSTR)(diskFile.strWorkDir + diskFile.vecFile[i]));
						}
					}

				}
			}
		}
		else
		{
			m_socketListen.SendStrValue((LPTSTR)(LPCTSTR)_M("No data"));
		}


		::CloseClipboard();
	}

	return TRUE;
}

BOOL CWorkTool::CreateTempDirForCopy(CString strSysTempDir, CString strIp, CString& strDestDir)
{
	strDestDir = strSysTempDir + "\\NetClipboard";
	if (!CreateDirectory(strDestDir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
		return FALSE;

	strDestDir += "\\" + strIp;
	if (!CreateDirectory(strDestDir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
		return FALSE;

	strDestDir += "\\" + GetCurrentFormattedTime(TRUE);
	if (!CreateDirectory(strDestDir, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
		return FALSE;

	strDestDir += "\\";

	return TRUE;
}

CString CWorkTool::GetCurrentFormattedTime(BOOL bForFileName)
{
	CString strFormat = "%04d-%02d-%02d %02d-%02d-%02d";
	if (!bForFileName)
	{
		strFormat = "%04d-%02d-%02d %02d:%02d:%02d";
	}

	COleDateTime dt = COleDateTime::GetCurrentTime();
	CString strRet;
	strRet.Format(strFormat, dt.GetYear(), dt.GetMonth(), dt.GetDay(),
		dt.GetHour(), dt.GetMinute(), dt.GetSecond());

	return strRet;
}