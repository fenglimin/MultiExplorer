#include "Stdafx.h"
#include "SocketTool.h"
#include "Stdio.h"

char	CSocketTool::m_strRelayServerIp[] ( "" );
int		CSocketTool::m_nRelayServerPort ( 0 );
int		CSocketTool::m_nTimeOut ( 0 );
int		CSocketTool::m_nDefaultConnectPort ( 0 );

CSocketTool::CSocketTool()
{
	GetSystemDirectory ( m_strSysPath, MAX_PATH );
	GetTempPath ( MAX_PATH, m_strTempPath );
}

CSocketTool::~CSocketTool()
{
}

void CSocketTool::SetRelayServer ( char* strRelayServerIp, int nRelayServerPort )
{
	strcpy_s ( m_strRelayServerIp, strRelayServerIp );
	m_nRelayServerPort = nRelayServerPort;
}

BOOL CSocketTool::Init(int nPort)
{
	m_bServer = TRUE;

	//Variable Define
	WORD wMajorVersion,wMinorVersion;
	WORD wVersionReqd;
	WSADATA lpmyWSAData;
	
	//InI Winsock
	wMajorVersion = 1;
	wMinorVersion = 2;
	wVersionReqd = MAKEWORD ( wMajorVersion, wMinorVersion );

	//Startup WinSock
	int Status = WSAStartup ( wVersionReqd, &lpmyWSAData );	
	if ( Status != 0 )
		return FALSE;

	//Socket
	m_socketListen = socket ( AF_INET, SOCK_STREAM, 0 );
	if ( m_socketListen == INVALID_SOCKET )
		return FALSE;

	m_addrListen.sin_family			= PF_INET;
	m_addrListen.sin_port			= htons ( nPort );
	m_addrListen.sin_addr.s_addr	= INADDR_ANY;
	
	//BIND
	Status = bind ( m_socketListen, (struct sockaddr far *)&m_addrListen,
					sizeof(m_addrListen) );
	if ( Status != 0 )
		return FALSE;
	
	//LISTEN
	Status = listen ( m_socketListen, 1 );
	if ( Status != 0 )
		return FALSE;
	
	m_nPort			= nPort;
	m_bSocketGood	= TRUE;

	int value = MAX_TRANS_UNIT;
	int tmpCode = 0;
	tmpCode = ::setsockopt(m_socketWork, SOL_SOCKET, SO_RCVBUF, (char*)&value, sizeof(value));
	tmpCode = ::setsockopt(m_socketWork, SOL_SOCKET, SO_SNDBUF, (char*)&value, sizeof(value));
	int result = 0;
	int len = 4;
	tmpCode = ::getsockopt((SOCKET)m_socketWork, SOL_SOCKET, SO_RCVBUF, (char*)&result, &len);
	printf("SO_RCVBUF=%d, tmpCode=%d\n", result, tmpCode);
	tmpCode = ::getsockopt((SOCKET)m_socketWork, SOL_SOCKET, SO_SNDBUF, (char*)&result, &len);
	printf("SO_SNDBUF=%d, tmpCode=%d\n", result, tmpCode);

	return TRUE;
}

BOOL CSocketTool::PendingAccept(int nTimeOut)
{
//	if ( IsSocketBlock ( TRUE, m_socketListen, nTimeOut ) )
//		return FALSE;

	//ACCEPT
	int nLen = sizeof ( m_addrListen );
	m_socketWork = accept ( m_socketListen, (struct sockaddr far *)&m_addrListen,
							  &nLen );
	if (m_socketWork == INVALID_SOCKET)
	{
		int err = WSAGetLastError();
		CloseSocket();
		m_bSocketGood = FALSE;
		return FALSE;
	}

	return TRUE;
}

BOOL CSocketTool::RecvIntValue(int &nValue)
{
	if ( IsSocketBlock ( TRUE, m_socketWork ) )
		return FALSE;

	int nFlag, nRecvCount;

	nRecvCount = recv ( m_socketWork, (char*)&nFlag, sizeof(nFlag), 0 );
	if ( nRecvCount == SOCKET_ERROR )
	{
		int e = errno;
		e = WSAGetLastError();
		e = WSANOTINITIALISED;
		CloseSocket();
		m_bSocketGood = FALSE;
		return FALSE;
	}

	nValue = nFlag;

	return TRUE;
}

/* 调用
	char strValue[MAX_LEN];
	int  nLen = MAX_LEN;
	if ( RecvStrValue ( strValue, nLen ) )
	{
		...
	}
*/
//BOOL CSocketTool::RecvStrValue(char *strValue, int &nLen)
//{
//	int nBufferLen;
//	if ( !RecvIntValue(nBufferLen) )
//		return FALSE;
//
//	if ( nBufferLen > nLen )
//		return FALSE;
//	nLen = nBufferLen;
//
//	lstrcpy ( strValue, "" );
//
//	int nTotalRecv = 0;
//	while ( nTotalRecv < nBufferLen )
//	{
//		int nTryToRecv = MAX_TRANS_UNIT;
//		if ( nLen-nTotalRecv < MAX_TRANS_UNIT )
//			nTryToRecv = nLen-nTotalRecv;
//
//		if ( IsSocketBlock ( TRUE, m_socketWork ) )
//			return FALSE;
//
//		int nRecved = recv ( m_socketWork, strValue+nTotalRecv, nTryToRecv, 0 );
//		if ( nRecved == SOCKET_ERROR )
//		{
//			CloseSocket();
//			m_bSocketGood = FALSE;
//			return FALSE;
//		}
//
//		nTotalRecv += nRecved;
//	}
//
//	strValue[nTotalRecv] = '\0';
//	return TRUE;
//}

BOOL CSocketTool::RecvStrValue(CString& strValue)
{
	int nBufferLen;
	if (!RecvIntValue(nBufferLen))
		return FALSE;

	int nIndex = 0;
	char* pBuffer = new char[nBufferLen + 1];
	//while ( nBufferLen > 0)
	//{
	//	if (IsSocketBlock(TRUE, m_socketWork))
	//		return FALSE;

	//	int nRecved = recv(m_socketWork, &pBuffer[nIndex], nBufferLen, 0);
	//	if (nRecved == SOCKET_ERROR)
	//	{
	//		CloseSocket();
	//		m_bSocketGood = FALSE;
	//		return FALSE;
	//	}

	//	nIndex += nRecved;
	//	nBufferLen -= nRecved;
	//}

	int nTotalRecv = 0;
	while ( nTotalRecv < nBufferLen )
	{
		int nTryToRecv = MAX_TRANS_UNIT;
		if (nBufferLen - nTotalRecv < MAX_TRANS_UNIT)
			nTryToRecv = nBufferLen - nTotalRecv;
	
		if ( IsSocketBlock ( TRUE, m_socketWork ) )
			return FALSE;
	
		int nRecved = recv(m_socketWork, &pBuffer[nTotalRecv], nTryToRecv, 0);
		if ( nRecved == SOCKET_ERROR )
		{
			CloseSocket();
			m_bSocketGood = FALSE;
			return FALSE;
		}
	
		nTotalRecv += nRecved;
	}

	pBuffer[nBufferLen] = '\0';
	strValue = pBuffer;

	delete[] pBuffer;
	return TRUE;
}

BOOL CSocketTool::SendIntValue(int nValue)
{
	if ( IsSocketBlock ( FALSE, m_socketWork ) )
		return FALSE;

	int nSend = send ( m_socketWork, (char*)&nValue, sizeof(nValue), 0 );
	if ( nSend == SOCKET_ERROR )
	{
		int err = WSAGetLastError();
		CloseSocket();
		return FALSE;
	}

	return TRUE;
}

BOOL CSocketTool::SendStrValue(char *strValue, int nLen)
{
	if ( nLen == -1 )
		nLen = strlen ( strValue );

	if ( !SendIntValue ( nLen ) )
		return FALSE;

	int nTotalSend = 0;
	while ( nTotalSend < nLen )
	{
		int nTryToSend = MAX_TRANS_UNIT;
		if ( nLen-nTotalSend < MAX_TRANS_UNIT )
			nTryToSend = nLen-nTotalSend;

		if ( IsSocketBlock ( FALSE, m_socketWork ) )
			return FALSE;

		int nSended = send ( m_socketWork, strValue+nTotalSend, nTryToSend, 0 );
		if ( nSended == SOCKET_ERROR )
		{
			int err = WSAGetLastError();
			CloseSocket();
			m_bSocketGood = FALSE;
			return FALSE;
		}

		nTotalSend += nSended;
	}

	return TRUE;
}

 
BOOL CSocketTool::RecvFile( char* strFileFullName )
{
	int nFileLen;
	if ( !RecvIntValue ( nFileLen ) )
		return FALSE;

	if (nFileLen == -1)
		return FALSE;

	HANDLE hFile = CreateFile ( strFileFullName, GENERIC_WRITE, FILE_SHARE_READ, 
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL );
	if ( hFile == INVALID_HANDLE_VALUE )
		return FALSE;

	BYTE* data = new BYTE[MAX_TRANS_UNIT];
	DWORD dwWrite;

	int nTotalRecv = 0;

	while ( nTotalRecv < nFileLen )
	{
		memset(data,0,MAX_TRANS_UNIT);

		int nTryToRecv = MAX_TRANS_UNIT;
		if ( nFileLen-nTotalRecv < MAX_TRANS_UNIT )
			nTryToRecv = nFileLen-nTotalRecv;

		if ( IsSocketBlock ( TRUE, m_socketWork ) )
			return FALSE;

		int nRecved = recv ( m_socketWork, (char*)data, nTryToRecv, 0 );
		if ( nRecved == SOCKET_ERROR )
		{
			CloseSocket();
			m_bSocketGood = FALSE;
			return FALSE;
		}

		WriteFile ( hFile, (void*)data, nRecved, &dwWrite, 0 );
		nTotalRecv += nRecved;
	}

	delete data;

//	SetFileTimeEx ( hFile, &StreamFileInfo, 10000 );
	CloseHandle ( hFile );

	return TRUE;

}

BOOL CSocketTool::SendFile(char *strFileFullName)
{
	WIN32_FIND_DATA         FindFileData;

	FindClose(FindFirstFile(strFileFullName,&FindFileData));
	int nFileLen = FindFileData.nFileSizeLow;

	OFSTRUCT temp;
	HANDLE hFile = (HANDLE)OpenFile ( strFileFullName, &temp, OF_READ );
	if (hFile == INVALID_HANDLE_VALUE)
	{
		SendIntValue(-1);
		return FALSE;
	}

	// Send file length
	if (!SendIntValue(nFileLen))
		return FALSE;


	DWORD dwRead=0;
	DWORD dw;
	BYTE* data = new BYTE[MAX_TRANS_UNIT];
	while ( (int)dwRead < nFileLen )
	{
		memset ( data, 0, MAX_TRANS_UNIT );
		ReadFile ( hFile, data, MAX_TRANS_UNIT, &dw, NULL );

		if ( IsSocketBlock ( FALSE, m_socketWork ) )
			return FALSE;

		int nSended = send( m_socketWork, (char*)data, sizeof(BYTE)*dw, 0);
		if ( nSended == SOCKET_ERROR )
		{
			CloseSocket();
			m_bSocketGood = FALSE;
			return FALSE;
		}
		dwRead += nSended;
	}
	delete[] data;

	CloseHandle ( hFile );

	return TRUE;
}


void CSocketTool::CloseSocket()
{
	if (m_bServer)
	{
		closesocket(m_socketListen);
		m_socketListen = INVALID_SOCKET;
	}

	closesocket ( m_socketWork );
	m_socketWork = INVALID_SOCKET;
}

BOOL CSocketTool::Connect(char *strHostName, int nPort, BOOL bShowMessageIfError)
{
	m_bServer = FALSE;
	//Variable Define
	WORD wMajorVersion,wMinorVersion;
	WORD wVersionReqd;
	WSADATA lpmyWSAData;
	
	//InI Winsock
	wMajorVersion = 1;
	wMinorVersion = 2;
	wVersionReqd = MAKEWORD ( wMajorVersion, wMinorVersion );

	//Startup WinSock
	int Status = WSAStartup ( wVersionReqd, &lpmyWSAData );	
	if ( Status != 0 )
		return FALSE;

	char     server_address[256];
	PHOSTENT HostInfo;

	char strNextHostName[32];
	int  nNextHostPort;

	if ( nPort == 0 )
		nPort = m_nDefaultConnectPort;

	//如果设置了转发的Server
	if ( strlen ( m_strRelayServerIp ) != 0 )
	{
		strcpy_s ( strNextHostName, m_strRelayServerIp );
		nNextHostPort = m_nRelayServerPort;
	}
	else
	{
		strcpy_s ( strNextHostName, strHostName );
		nNextHostPort = nPort;
	}


	if ( ( HostInfo = gethostbyname(strNextHostName)) != NULL )
    		strcpy_s ( server_address, inet_ntoa (*(struct in_addr *)*HostInfo->h_addr_list) );
	else
	{
		if ( bShowMessageIfError )
		{
			char strShow[128];
			strcpy_s ( strShow, "无法从机器名[" );
			strcat_s ( strShow, strNextHostName );
			strcat_s ( strShow, "]解析出IP地址!" );

			MessageBox ( NULL, strShow, "错误", MB_OK );
		}
		return FALSE;
	}

	//Socket
	m_socketWork = socket ( AF_INET, SOCK_STREAM, 0 );
	if ( m_socketWork == INVALID_SOCKET )
		return FALSE;

	m_addrServer.sin_family			= PF_INET;
	m_addrServer.sin_port			= htons ( nNextHostPort );
	m_addrServer.sin_addr.s_addr	= inet_addr(server_address);
	
	int nRet = connect ( m_socketWork, (struct sockaddr*)&m_addrServer, 
						 sizeof(m_addrServer) );
	if ( nRet != 0 )
	{
		if ( bShowMessageIfError )
		{
			char show[256];
			int nError = (int )WSAGetLastError ();
			sprintf_s ( show, "无法与目标机器 【%s】建立连接!错误代码%d", strNextHostName, nError );
			MessageBox ( NULL, show, "错误", MB_OK );
			return FALSE;
		}
	}
	
	//如果设置了转发的Server
	if ( strlen ( m_strRelayServerIp ) != 0 )
	{
		SendStrValue ( strHostName );
		SendIntValue ( nPort );
	}

	m_nPort			= nNextHostPort;
	m_bSocketGood	= TRUE;

	int value = MAX_TRANS_UNIT;
	int tmpCode = 0;
	tmpCode = ::setsockopt(m_socketWork, SOL_SOCKET, SO_RCVBUF, (char*)&value, sizeof(value));
	tmpCode = ::setsockopt(m_socketWork, SOL_SOCKET, SO_SNDBUF, (char*)&value, sizeof(value));
	int result = 0;
	int len = 4;
	tmpCode = ::getsockopt((SOCKET)m_socketWork, SOL_SOCKET, SO_RCVBUF, (char*)&result, &len);
	printf("SO_RCVBUF=%d, tmpCode=%d\n", result, tmpCode);
	tmpCode = ::getsockopt((SOCKET)m_socketWork, SOL_SOCKET, SO_SNDBUF, (char*)&result, &len);
	printf("SO_SNDBUF=%d, tmpCode=%d\n", result, tmpCode);

	return TRUE;
}

int CSocketTool::GetWorkStatus(char *strServerIp)
{
	WIN32_FIND_DATA     FindFileData;
	char				strCommandFile[64];

	strcpy_s ( strCommandFile, "\\\\" );
	strcat_s ( strCommandFile, strServerIp );
	strcat_s ( strCommandFile, "\\Public\\Command.txt" );

	HANDLE hFile = FindFirstFile ( strCommandFile, &FindFileData );
	if ( hFile == NULL )
		return -1;

	FindClose ( hFile );

	int nFileLen = FindFileData.nFileSizeLow;

	OFSTRUCT temp;
	hFile = (HANDLE)OpenFile ( strCommandFile, &temp, OF_READ );
	if ( !hFile )
		return -1;

	char* strContent = new char ( nFileLen );
	memset ( (void*)strContent, 0, nFileLen );

	DWORD dw;
	ReadFile ( hFile, strContent, nFileLen, &dw, NULL );
	CloseHandle ( hFile );

	char* strFind = strstr ( strContent, strServerIp );
	if ( strFind == NULL )
		return -1;

	int nLen = strlen ( strServerIp ) + 3;
	if ( strFind[nLen] == '0')
		return 0;

	return 1;

}

BOOL CSocketTool::RecvFileAndRelay(CSocketTool *pRelaySocket)
{
	int nFileLen;
	if ( !RecvIntValue ( nFileLen ) )
		return FALSE;

	//如果文件大小<512k，则直接在内存中转发数据
	if ( nFileLen < 512000 )
	{
		BYTE* data = new BYTE[nFileLen];
		memset(data,0,nFileLen);
		int nTotalRecv = 0;

		while ( nTotalRecv < nFileLen )
		{
			int nTryToRecv = MAX_TRANS_UNIT;
			if ( nFileLen-nTotalRecv < MAX_TRANS_UNIT )
				nTryToRecv = nFileLen-nTotalRecv;

			if ( IsSocketBlock ( TRUE, m_socketWork ) )
				return FALSE;

			int nRecved = recv ( m_socketWork, (char*)(data+nTotalRecv), nTryToRecv, 0 );
			if ( nRecved == SOCKET_ERROR )
			{
				CloseSocket();
				m_bSocketGood = FALSE;
				return FALSE;
			}

			nTotalRecv += nRecved;
		}

		pRelaySocket->SendIntValue ( nFileLen );
			
		int nTotalSend = 0;
		while ( nTotalSend < nFileLen )
		{
			int nTryToSend = MAX_TRANS_UNIT;
			if ( nFileLen-nTotalSend < MAX_TRANS_UNIT )
				nTryToSend = nFileLen-nTotalSend;

			if ( IsSocketBlock ( FALSE, m_socketWork ) )
				return FALSE;

			int nSended = send( pRelaySocket->m_socketWork, (char*)(data+nTotalSend), sizeof(BYTE)*nTryToSend, 0);
			if ( nSended == SOCKET_ERROR )
			{
				pRelaySocket->CloseSocket();
				m_bSocketGood = FALSE;
				return FALSE;
			}

			nTotalSend += nSended;
		}

		delete data;
	}
	else
	{
		char strTempFile[128];
		strcpy_s ( strTempFile, m_strTempPath );
		strcat_s ( strTempFile, "\\~F00L00M1" );

		HANDLE hFile = CreateFile ( strTempFile, GENERIC_WRITE, FILE_SHARE_READ, 
			NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL );
		if ( hFile == INVALID_HANDLE_VALUE )
			return FALSE;

		BYTE* data = new BYTE[MAX_TRANS_UNIT];
		DWORD dwWrite;

		int nTotalRecv = 0;

		while ( nTotalRecv < nFileLen )
		{
			memset(data,0,MAX_TRANS_UNIT);

			int nTryToRecv = MAX_TRANS_UNIT;
			if ( nFileLen-nTotalRecv < MAX_TRANS_UNIT )
				nTryToRecv = nFileLen-nTotalRecv;

			if ( IsSocketBlock ( TRUE, m_socketWork ) )
				return FALSE;

			int nRecved = recv ( m_socketWork, (char*)data, nTryToRecv, 0 );
			if ( nRecved == SOCKET_ERROR )
			{
				CloseSocket();
				m_bSocketGood = FALSE;
				return FALSE;
			}

			WriteFile ( hFile, (void*)data, nRecved, &dwWrite, 0 );
			nTotalRecv += nRecved;
		}

		delete data;

	//	SetFileTimeEx ( hFile, &StreamFileInfo, 10000 );
		CloseHandle ( hFile );

		pRelaySocket->SendFile ( strTempFile );
		DeleteFile ( strTempFile );
	}

	return TRUE;
}


BOOL CSocketTool::IsSocketBlock( BOOL bCheckRead, SOCKET socketCheck, int nTimeOut )
{
	//TIMEVAL tv;
	//tv.tv_usec = 0;

	//if ( nTimeOut == -1 )
	//	if ( m_nTimeOut == 0 )
	//		return FALSE;
	//	else
	//		tv.tv_sec = m_nTimeOut;
	//else
	//	tv.tv_sec = nTimeOut;
	//	

	//FD_SET fd	= { 1, socketCheck };

	//int nRet;
	//if ( bCheckRead ) // listen recv recvfrom
	//	nRet = select ( 0, &fd, NULL, NULL, &tv );
	//else // connect send sendto
	//	nRet = select ( 0, NULL, &fd, NULL, &tv );

	//return ( nRet == 0 );

	return FALSE;
}

void CSocketTool::SetTimeOut ( int nTimeOut )
{
	m_nTimeOut = nTimeOut;
}

void CSocketTool::SetDefaultConnectPort ( int nDefaultConnectPort )
{
	m_nDefaultConnectPort = nDefaultConnectPort;
}