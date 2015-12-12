#ifndef _SOCKETTOOL_H_
#define _SOCKETTOOL_H_

#include <winsock.h>

//#define MAX_LEN								102400
#define	MAX_TRANS_UNIT						10240

typedef struct _SOCKET_STREAM_FILE_INFO {

    TCHAR       szFileTitle[128];                   //文件的标题名
    DWORD       dwFileAttributes;                   //文件的属性
    FILETIME    ftCreationTime;                     //文件的创建时间
    FILETIME    ftLastAccessTime;                   //文件的最后访问时间
    FILETIME    ftLastWriteTime;                    //文件的最后修改时间
    DWORD       nFileSizeHigh;                      //文件大小的高位双字
    DWORD       nFileSizeLow;                       //文件大小的低位双字
    DWORD       dwReserved0;                        //保留，为0
    DWORD       dwReserved1;                        //保留，为0

} SOCKET_STREAM_FILE_INFO, * PSOCKET_STREAM_FILE_INFO;

class CSocketTool
{
public:
	CSocketTool();
	~CSocketTool();

public: //静态成员
	static char			m_strRelayServerIp[24];
	static int			m_nRelayServerPort;
	static void			SetRelayServer ( char* strRelayServerIp, int nRelayServerPort );

	static int			m_nTimeOut;
	static void			SetTimeOut ( int nTimeOut );

	static int			m_nDefaultConnectPort;
	static void			SetDefaultConnectPort ( int nDefaultConnectPort );

public:

	struct sockaddr_in	m_addrListen;
	struct sockaddr_in	m_addrServer;

	SOCKET				m_socketWork;
	SOCKET				m_socketListen;

	int					m_nPort;
	char				m_strSysPath[MAX_PATH];
	char				m_strTempPath[MAX_PATH];

	BOOL				m_bServer;
	BOOL				m_bSocketGood;

public:
	BOOL IsSocketBlock( BOOL bCheckRead, SOCKET socketCheck, int nTimeOut = -1);
	BOOL RecvStrAndRelay(CSocketTool *pRelaySocket);
	BOOL RecvIntAndRelay(CSocketTool *pRelaySocket);
		
	BOOL Init ( int nPort );
	void CloseSocket();

	BOOL PendingAccept( int nTimeOut = -1);
	BOOL Connect(char* strHostName, int nPort = 0, BOOL bShowMessageIfError = TRUE );

	BOOL SendFile ( char* strFileFullName );
	BOOL RecvFile ( char* strFileFullName );
	BOOL RecvFileAndRelay ( CSocketTool* pRelaySocket );

	BOOL SendStrValue ( char* strValue, int nLen = -1 );
//	BOOL RecvStrValue ( char* strValue, int& nLen );
	BOOL RecvStrValue ( CString& strValue );

	BOOL SendIntValue ( int nValue );
	BOOL RecvIntValue ( int& nValue );
	

	int GetWorkStatus ( char* strServerIp);

};

#endif
