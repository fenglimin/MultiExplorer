#ifndef _WORKTOOL_H_
#define _WORKTOOL_H_

#include "SocketTool.h"
#include "..\MultiExplorer\MultiExplorerDef.h"

#define NETWORK_TASK_END						0
#define NETWORK_GET_CLIPBOARD_DATA				1



class CWorkTool
{
public:
	CWorkTool(IWorkToolServerUser* pServerUser);
	~CWorkTool();

	CSocketTool		m_socketListen;
	BOOL			m_bExit;
	IWorkToolServerUser*	m_pServerUser;
	IWorkToolClientUser*	m_pClientUser;
public:
	BOOL NetPeek_SendFile();
	BOOL NetPeek_RecvFile();
	BOOL NetPeek_Chatting();
	BOOL PendingRead();
	BOOL StartWorking ( int nPort );
	void SetClientUser(IWorkToolClientUser* pClientUser) { m_pClientUser = pClientUser; }
	BOOL RecvEmptyDir();

	BOOL Request_GetClipboardData(CString strIp, int nPort, int nFormat, CString& strOutput);
	BOOL Response_GetClipboardData();
};

#endif