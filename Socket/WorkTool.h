#ifndef _WORKTOOL_H_
#define _WORKTOOL_H_

#include "SocketTool.h"
#include "..\MultiExplorer\MultiExplorerDef.h"

#define NETWORK_TASK_END						0
#define NETWORK_GET_CLIPBOARD_DATA				1

class IWorkToolUser
{
public:
	virtual BOOL OnNewMessage(const CString& strMessage, BOOL bAddTimeStamp){ return TRUE; }
	virtual BOOL OnGetAllDirFilesFromClipboard(CDiskFile& diskFile){ return TRUE; }
	virtual BOOL OnEmptyDirReceived(CString strDir){ return TRUE; }
};

class CWorkTool
{
public:
	CWorkTool(IWorkToolUser* pUser);
	~CWorkTool();

	CSocketTool		m_socketTool;
	BOOL			m_bExit;
	IWorkToolUser*	m_pUser;
public:
	BOOL NetPeek_SendFile();
	BOOL NetPeek_RecvFile();
	BOOL NetPeek_Chatting();
	BOOL PendingRead();
	BOOL StartWorking ( int nPort );
	void SetUser(IWorkToolUser* pUser) { m_pUser = pUser; }
	BOOL RecvEmptyDir();

	BOOL Request_GetClipboardData(CString strIp, int nPort, int nFormat, CString& strOutput);
	BOOL Response_GetClipboardData();
};

#endif