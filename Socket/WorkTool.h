#ifndef _WORKTOOL_H_
#define _WORKTOOL_H_

#include "SocketTool.h"

#define NETWORK_TASK_END						0
#define NETWORK_GET_CLIPBOARD_DATA				1

class CWorkTool
{
public:
	CWorkTool();
	~CWorkTool();

	CSocketTool			m_socketTool;
	BOOL		m_bExit;
public:
	BOOL NetPeek_SendFile();
	BOOL NetPeek_RecvFile();
	BOOL NetPeek_Chatting();
	BOOL PendingRead();
	BOOL StartWorking ( int nPort );

	BOOL Request_GetClipboardData(CString strIp, int nPort, CString& strOutput);
	BOOL Response_GetClipboardData();
};

#endif