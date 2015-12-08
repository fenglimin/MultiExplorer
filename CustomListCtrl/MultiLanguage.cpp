#include "stdafx.h"
#include "MultiLanguage.h"

BOOL CMultiLanguage::m_bEnglish = FALSE;
map<CString,CString>	CMultiLanguage::m_mapTranslate;

void CMultiLanguage::SetLanguage(BOOL bEnglish)
{
	m_bEnglish = bEnglish;
}

CString CMultiLanguage::Translate(const CString& strKey)
{
	if ( m_bEnglish )
		return strKey;

	CString	strRet = m_mapTranslate[strKey];
	if ( strRet.IsEmpty())
		strRet = strKey;

	return strRet;
}

void CMultiLanguage::Init()
{
	// Common
	m_mapTranslate["Exit"] = "�˳�";
	m_mapTranslate["Table"] = "��";
	m_mapTranslate["OK"] = "ȷ��";
	m_mapTranslate["Cancel"] = "ȡ��";
	m_mapTranslate["Save"] = "����";
	m_mapTranslate["ALL"] = "ȫ��";
	m_mapTranslate["Notes"] = "�ı�";
	m_mapTranslate["Set Date Range"] = "�������ڷ�Χ";
	m_mapTranslate["Set Time Range"] = "����ʱ�䷶Χ";
	m_mapTranslate["From"] = "��";
	m_mapTranslate["To"] = "��";
	m_mapTranslate["Clear"] = "���";
	m_mapTranslate["Do you want to save the changes?"] = "Ҫ������";
	m_mapTranslate["Prompt For Save"] = "��ʾ����";
	m_mapTranslate["Close"] = "�ر�";
	m_mapTranslate["Export New MultiExplorer"] = "�����µ�MultiExplorer";
	m_mapTranslate["%s ( Machine Name: %s, IP Address: %s )"] = "%s ( ������: %s, IP��ַ: %s )";

	// About dialog
	m_mapTranslate["&About MultiExplorer..."] = "&���� MultiExplorer...";
	m_mapTranslate["About MultiExplorer"] = "���� MultiExplorer";
	m_mapTranslate["MultiExplorer Version 1.0"] = "MultiExplorer �汾1.0";
	m_mapTranslate["Developer : Sail Feng,  mailflm@163.com"] = "���� :	������, mailflm@163.com";

	// File Explorer
	m_mapTranslate["Name"] = "����";
	m_mapTranslate["Size"] = "��С";
	m_mapTranslate["Ext"] = "����";
	m_mapTranslate["Modified"] = "�޸�ʱ��";
	m_mapTranslate["Desktop"] = "����";
	m_mapTranslate["My Documents"] = "�ҵ��ĵ�";
	m_mapTranslate["Backward"] = "ǰһ��";
	m_mapTranslate["Forward"] = "��һ��";
	m_mapTranslate["Upward"] = "����";
	m_mapTranslate["Refresh"] = "ˢ��";
	m_mapTranslate["Show hidden files"] = "��ʾ�����ļ�";
	m_mapTranslate["Total %d directory(s), %d file(s). %d item(s) selected."] = "�ܹ�%d��Ŀ¼, %d���ļ�, %d�ѡ��.";
	 

	// Main Dialog
	m_mapTranslate["Export"] = "����";
	m_mapTranslate["Import"] = "����";
	m_mapTranslate["Export Configuration"] = "����������Ϣ";
	m_mapTranslate["Import Configuration"] = "����������Ϣ";
	m_mapTranslate["Options"] = "ѡ��";
	m_mapTranslate["Change Layout"] = "���Ĳ���";
	m_mapTranslate["%s exist! Please input another name!"] = "%s�Ѵ��ڣ���������֣�";
	m_mapTranslate["Open Dir [%s] In Explorer"] = "��Windows��Դ�������д�Ŀ¼��%s��";
	m_mapTranslate["Open Dir [%s] In Dos Command"] = "��Dos�������д�Ŀ¼��%s��";
	m_mapTranslate["Open Dir [%s] In New Cell"] = "�����Ӵ����д�Ŀ¼��%s��";
	m_mapTranslate["Run File [%s] in Dos Command"] = "��Dos�������������ļ���%s��";
	m_mapTranslate["Open File [%s]"] = "���ļ���%s��";
	m_mapTranslate["Open File [%s] In Notepad"] = "��Notepad�д��ļ���%s��";
	m_mapTranslate["Open File [%s] In Internet Explorer"] = "��Internet Explorer�д��ļ���%s��";
	m_mapTranslate["Open Selected File In UltraEdit"] = "��UltraEdit�д���ѡ�ļ�";
	m_mapTranslate["Open Selected File In Notepad++"] = "��Notepad++�д���ѡ�ļ�";
	m_mapTranslate["Add Selected Files to Toolbar"] = "����ѡ�ļ���ӵ�������";
	m_mapTranslate["Select Left Side to Compare"] = "��Ϊ�Ƚϵ����";
	m_mapTranslate["Compare to [%s] In Beyond Compare"] = "��Beyond Compare�к͡�%s�����бȽ�";
	m_mapTranslate["Compare In Beyond Compare"] = "��Beyond Compare�н��бȽ�";
	m_mapTranslate["Extract To Current Directory"] = "��ѹ����ǰĿ¼";
	m_mapTranslate["Extract To %s"] = "��ѹ����%s��";
	m_mapTranslate["Add To %s"] = "��ӵ���%s��";
	m_mapTranslate["Send To"] = "����";
	m_mapTranslate["Rename"] = "������";
	m_mapTranslate["Rename To %s"] = "������Ϊ��%s��";
	m_mapTranslate["Delete"] = "ɾ��";
	m_mapTranslate["Copy"] = "����";
	m_mapTranslate["Cut"] = "����";
	m_mapTranslate["Paste"] = "ճ��";
	m_mapTranslate["Create a New Directory"] = "�½�һ��Ŀ¼";
	m_mapTranslate["Create a New Text File"] = "�½�һ���ı��ļ�";
	m_mapTranslate["Copy Unicode String To Windows Clipboard"] = "����Unicode�ַ�����Windows������";
	m_mapTranslate["Property"] = "����";
	m_mapTranslate["Can NOT rename file %s to %s!\r\nError : "] = "�޷����ļ���%s�� ������Ϊ ��%s����\r\n����ԭ��";
	m_mapTranslate["Can NOT delete file %s. \r\nReason : %s"] = "�޷�ɾ���ļ���%s����\r\n����ԭ��";
	m_mapTranslate["Are you sure to delete these %d items?"] = "ȷ��Ҫɾ����%d����";
	m_mapTranslate["Are you sure to delete \" %s \" ?"] = "ȷ��Ҫɾ����%s�� ��";
	m_mapTranslate["%s_copy(%d)%s"] = "%s_����(%d)%s";
	m_mapTranslate["Export to file %s successfully!"] = "�ɹ��������ļ� %s��";
	m_mapTranslate["New Folder"] = "�½��ļ���";
	m_mapTranslate["New Text File"] = "�½� �ı��ĵ�";



	// Options
	m_mapTranslate["Property Name"] = "����";
	m_mapTranslate["Property Value"] = "ֵ";
	m_mapTranslate["Language"] = "����";
	m_mapTranslate["Navigate Tree"] = "������";
	m_mapTranslate["Show"] = "��ʾ";
	m_mapTranslate["Width"] = "���ڿ��";
	m_mapTranslate["External Program"] = "�ⲿ����";
	m_mapTranslate["Explorer List Style"] = "�б��ʽ";
	m_mapTranslate["Row Height"] = "�и�";
	m_mapTranslate["Toolbar"] = "������";
	m_mapTranslate["Button Size"] = "��ť��С";
	m_mapTranslate["Auto Load From Desktop"] = "�Զ���������";
	m_mapTranslate["Auto Load From Taskbar"] = "�Զ����ؿ�������";
	m_mapTranslate["Odd Line BK Color(Active)"] = "�����б�����ɫ�����";
	m_mapTranslate["Even Line BK Color(Active)"] = "ż���б�����ɫ�����";
	m_mapTranslate["Odd Line BK Color(Inactive)"] = "�����б�����ɫ��δ���";
	m_mapTranslate["Even Line BK Color(Inactive)"] = "ż���б�����ɫ��δ���";

	// Toolbar
	m_mapTranslate["%s(%s), Total space %s M, Free space %s M"] = "%s(%s), ����%sM, ʣ��ռ�%sM";
	m_mapTranslate["%s(%s), Error getting capacity!"] = "%s(%s), �޷���ȡ����������";

	// Dir Tree
	m_mapTranslate["Control Panel"] = "�������";

	// Favorite dir
	m_mapTranslate["Favorite Directory"] = "�ղؼ�";
	m_mapTranslate["Add to favorite"] = "��ӵ��ղؼ�";
	m_mapTranslate["Group"] = "����";
	m_mapTranslate["Name"] = "����";
	m_mapTranslate["Dir"] = "Ŀ¼";
	m_mapTranslate["Add Dir [%s] To Favorite"] = "��Ŀ¼ [%s] ��ӵ��ղؼ�";
	m_mapTranslate["Delete Group [%s]"] = "ɾ������ [%s]";
	m_mapTranslate["Rename Group [%s]"] = "���������� [%s]";
	m_mapTranslate["Open Dir [%s - %s]"] = "��Ŀ¼ [%s - %s]";
	m_mapTranslate["Delete Dir [%s]"] = "ɾ��Ŀ¼ [%s]";
	m_mapTranslate["Rename Dir [%s]"] = "������Ŀ¼ [%s]";
	m_mapTranslate["Are you sure to delete this %s?"] = "ȷ��Ҫɾ�����%s��";

	// Menu
	m_mapTranslate["Copy"] = "����";
	m_mapTranslate["Delete All Selected Items"] = "ɾ������ѡ�е���";
	m_mapTranslate["Insert Item Before This Row"] = "����һ������";
	m_mapTranslate["Append Item To Bottom"] = "����һ�����ݣ�������棩";
	m_mapTranslate["Move Top"] = "�Ƶ�����";
	m_mapTranslate["Move Up"] = "����";
	m_mapTranslate["Move Down"] = "����";
	m_mapTranslate["Move Bottom"] = "�Ƶ�ĩ��";
	m_mapTranslate["Select All"] = "ȫѡ";
	m_mapTranslate["Select None"] = "ȫ��ѡ";
	m_mapTranslate["Delete All Selected Users"] = "ɾ���û�";
	m_mapTranslate["New User"] = "�����û�";
	m_mapTranslate["Delete All Selected Columns"] = "ɾ������ѡ�е���";
	m_mapTranslate["Insert Column"] = "����һ����";
	m_mapTranslate["Append Column"] = "���һ���У�������棩";
	m_mapTranslate["Save"] = "����";

	// Network Clipboard
	m_mapTranslate["Network Clipboard"] = "���������";
	m_mapTranslate["Ip Address"] = "IP��ַ";
	m_mapTranslate["Port"] = "�˿�";
	m_mapTranslate["Get Clipboard Data"] = "��ȡ����������";
	m_mapTranslate["Please select a machine!"] = "��ѡ��һ̨Զ�˻�����";
	m_mapTranslate["%s Getting clipboard from %s ......\r\n\r\n"] = "%s ���ڴ�%s��ȡ����������......\r\n\r\n";
	m_mapTranslate["Get clipboard ended!"] = "��ȡ���������ݽ�����";
	m_mapTranslate["Can Not connect to server, ip address : %s, Port : %d."] = "�޷����ӵ�Զ�˻�����IP��ַ : %s, �˿� �� %d��";
	m_mapTranslate["Send command(%s) failed!"] = "�������� ( %s )ʧ�ܣ�";
	m_mapTranslate["Received clipboard failed!"] = "���ռ���������ʧ�ܣ�";
	m_mapTranslate["Can NOT start net listener with port %d, please check!"] = "�޷����������˿�%d�����飡";
	m_mapTranslate["Listening Port"] = "���ؼ����˿�";
	m_mapTranslate["Remote Machine"] = "Զ�˻���";
	m_mapTranslate["Listening port was modified, please save the changes and restart application."] = "�˿��Ѹ��ģ��뱣����Ĳ�����Ӧ�á�";
	m_mapTranslate["Use Unicode"] = "ʹ��Unicode��ʽ";
	m_mapTranslate["Append Message"] = "������Ϣ";
}

void CMultiLanguage::Clear()
{
	m_mapTranslate.clear();
}
