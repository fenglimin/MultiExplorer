#ifndef _SOCKETTOOL_H_
#define _SOCKETTOOL_H_

#include <winsock.h>

#define MAX_LEN								102400
#define	MAX_TRANS_UNIT						4096

typedef struct _SOCKET_STREAM_FILE_INFO {

    TCHAR       szFileTitle[128];                   //�ļ��ı�����
    DWORD       dwFileAttributes;                   //�ļ�������
    FILETIME    ftCreationTime;                     //�ļ��Ĵ���ʱ��
    FILETIME    ftLastAccessTime;                   //�ļ���������ʱ��
    FILETIME    ftLastWriteTime;                    //�ļ�������޸�ʱ��
    DWORD       nFileSizeHigh;                      //�ļ���С�ĸ�λ˫��
    DWORD       nFileSizeLow;                       //�ļ���С�ĵ�λ˫��
    DWORD       dwReserved0;                        //������Ϊ0
    DWORD       dwReserved1;                        //������Ϊ0

} SOCKET_STREAM_FILE_INFO, * PSOCKET_STREAM_FILE_INFO;

class CSocketTool
{
public:
	CSocketTool();
	~CSocketTool();

public: //��̬��Ա
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
	BOOL RecvStrValue ( char* strValue, int& nLen );
	BOOL RecvStrValue ( CString& strValue );

	BOOL SendIntValue ( int nValue );
	BOOL RecvIntValue ( int& nValue );
	

	int GetWorkStatus ( char* strServerIp);

};

#endif
