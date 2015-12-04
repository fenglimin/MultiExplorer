// RunCmd.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "stdlib.h"
#include "stdio.h"

int GetFieldInt(char *strOrg, char chDelim, int nFieldNum);
int GetFieldStr(char *strOrg, char chDelim, int nFieldNum, char *strOut);
BOOL DoDeleteFile ( LPSTR     lpCmdLine );
BOOL UpdateResourceInExe(LPSTR     lpCmdLine);
LPSTR	ReadFileContent ( char* strFileName, int& nFSize );

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	strcat ( lpCmdLine, "*" );
	int nCmdID = GetFieldInt ( lpCmdLine, '*', 0 );
	BOOL bOk = FALSE;

	for ( int i = 0; i < 30; i ++ )
	{
		switch ( nCmdID )
		{
		case 1:		bOk = DoDeleteFile ( lpCmdLine );
					break;
		case 2:		bOk = UpdateResourceInExe(lpCmdLine);
					break;
		}

		if ( bOk )
			return 0;
		else
			Sleep ( 1000 );
	}

	return 0;
}



int GetFieldInt(char *strOrg, char chDelim, int nFieldNum)
{
	int nLen = strlen ( strOrg );
	char	strInt[33];
	int i, nFieldStart = 0, nCurField = 0;
	
	for ( i = 0; i < nLen; i ++ )
	{
		if ( strOrg[i] == chDelim || i == nLen-1 )
		{
			if ( nCurField == nFieldNum )
			{
				if ( i - nFieldStart > 32 )
				{
					return -1;
				}
				memcpy ( strInt, strOrg+nFieldStart, i-nFieldStart );
				strInt[i-nFieldStart] = '\0';

				int nRet = atoi ( strInt );
				return nRet;
			}

			nCurField ++;
			nFieldStart = i+1;
		}
	}
	
	return -1;
}

int GetFieldStr(char *strOrg, char chDelim, int nFieldNum, char *strOut)
{
	int nLen = strlen ( strOrg );
	int i, nFieldStart = 0, nCurField = 0;
	
	strcpy ( strOut, "" );

	for ( i = 0; i < nLen; i ++ )
	{
		if ( strOrg[i] == chDelim  || i == nLen-1 )
		{
			if ( nCurField == nFieldNum )
			{
				memcpy ( strOut, strOrg+nFieldStart, i-nFieldStart );
				strOut[i-nFieldStart] = '\0';
				return 1;
			}

			nCurField ++;
			nFieldStart = i+1;
		}
	}
	
	return -1;

}

BOOL DoDeleteFile ( LPSTR     lpCmdLine )
{
	char strFileFullName[256];

	GetFieldStr ( lpCmdLine, '*', 1, strFileFullName );

	return DeleteFile ( strFileFullName );
}

BOOL UpdateResourceInExe(LPSTR     lpCmdLine)
{
	char strExeFileName[MAX_PATH];
	GetFieldStr(lpCmdLine, '*', 1, strExeFileName);
	HANDLE hUpdateRes = BeginUpdateResource ( strExeFileName, FALSE);
	if ( hUpdateRes == NULL )
	{
		return FALSE;
	}

	char strResType[128];
	char strNewContentFile[128];
	GetFieldStr(lpCmdLine, '*', 2, strResType);
	int nResID = GetFieldInt(lpCmdLine, '*', 3);
	GetFieldStr(lpCmdLine, '*', 4, strNewContentFile);

	int nFSize;
	LPSTR lpBuff = ReadFileContent ( strNewContentFile, nFSize );
	if ( !lpBuff )
		return FALSE;

	BOOL bOk = UpdateResource(hUpdateRes,
		strResType,MAKEINTRESOURCE(nResID),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
		lpBuff, nFSize);

	delete lpBuff;

	if ( !bOk)
	{
		return FALSE;
	}

	if ( !EndUpdateResource(hUpdateRes, FALSE) )
		return FALSE;

	return TRUE;
}


LPSTR ReadFileContent ( char* strFileName, int& nFSize )
{
	FILE* fpIn = fopen ( strFileName, "rb" );
	if ( fpIn == NULL )
		return FALSE;
	
	fseek(fpIn,0,SEEK_END);
	nFSize = (int) ftell(fpIn);
	fseek(fpIn,0,SEEK_SET);
	
	LPSTR lpBuff = new char[nFSize];
	if ( !lpBuff )
		return NULL;
	UINT nRead = fread ( lpBuff, 1, nFSize, fpIn );
	fclose ( fpIn );
	
	return lpBuff;
}