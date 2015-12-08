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
	m_mapTranslate["Exit"] = "退出";
	m_mapTranslate["Table"] = "表单";
	m_mapTranslate["OK"] = "确定";
	m_mapTranslate["Cancel"] = "取消";
	m_mapTranslate["Save"] = "保存";
	m_mapTranslate["ALL"] = "全部";
	m_mapTranslate["Notes"] = "文本";
	m_mapTranslate["Set Date Range"] = "设置日期范围";
	m_mapTranslate["Set Time Range"] = "设置时间范围";
	m_mapTranslate["From"] = "从";
	m_mapTranslate["To"] = "到";
	m_mapTranslate["Clear"] = "清除";
	m_mapTranslate["Do you want to save the changes?"] = "要保存吗？";
	m_mapTranslate["Prompt For Save"] = "提示保存";
	m_mapTranslate["Close"] = "关闭";
	m_mapTranslate["Export New MultiExplorer"] = "导出新的MultiExplorer";
	m_mapTranslate["%s ( Machine Name: %s, IP Address: %s )"] = "%s ( 机器名: %s, IP地址: %s )";

	// About dialog
	m_mapTranslate["&About MultiExplorer..."] = "&关于 MultiExplorer...";
	m_mapTranslate["About MultiExplorer"] = "关于 MultiExplorer";
	m_mapTranslate["MultiExplorer Version 1.0"] = "MultiExplorer 版本1.0";
	m_mapTranslate["Developer : Sail Feng,  mailflm@163.com"] = "作者 :	冯利民, mailflm@163.com";

	// File Explorer
	m_mapTranslate["Name"] = "名称";
	m_mapTranslate["Size"] = "大小";
	m_mapTranslate["Ext"] = "类型";
	m_mapTranslate["Modified"] = "修改时间";
	m_mapTranslate["Desktop"] = "桌面";
	m_mapTranslate["My Documents"] = "我的文档";
	m_mapTranslate["Backward"] = "前一个";
	m_mapTranslate["Forward"] = "后一个";
	m_mapTranslate["Upward"] = "向上";
	m_mapTranslate["Refresh"] = "刷新";
	m_mapTranslate["Show hidden files"] = "显示隐藏文件";
	m_mapTranslate["Total %d directory(s), %d file(s). %d item(s) selected."] = "总共%d个目录, %d个文件, %d项被选中.";
	 

	// Main Dialog
	m_mapTranslate["Export"] = "导出";
	m_mapTranslate["Import"] = "导入";
	m_mapTranslate["Export Configuration"] = "导出配置信息";
	m_mapTranslate["Import Configuration"] = "导入配置信息";
	m_mapTranslate["Options"] = "选项";
	m_mapTranslate["Change Layout"] = "更改布局";
	m_mapTranslate["%s exist! Please input another name!"] = "%s已存在！请更改名字！";
	m_mapTranslate["Open Dir [%s] In Explorer"] = "在Windows资源管理器中打开目录【%s】";
	m_mapTranslate["Open Dir [%s] In Dos Command"] = "在Dos命令行中打开目录【%s】";
	m_mapTranslate["Open Dir [%s] In New Cell"] = "在新子窗口中打开目录【%s】";
	m_mapTranslate["Run File [%s] in Dos Command"] = "在Dos命令行中运行文件【%s】";
	m_mapTranslate["Open File [%s]"] = "打开文件【%s】";
	m_mapTranslate["Open File [%s] In Notepad"] = "在Notepad中打开文件【%s】";
	m_mapTranslate["Open File [%s] In Internet Explorer"] = "在Internet Explorer中打开文件【%s】";
	m_mapTranslate["Open Selected File In UltraEdit"] = "在UltraEdit中打开所选文件";
	m_mapTranslate["Open Selected File In Notepad++"] = "在Notepad++中打开所选文件";
	m_mapTranslate["Add Selected Files to Toolbar"] = "将所选文件添加到工具条";
	m_mapTranslate["Select Left Side to Compare"] = "设为比较的左边";
	m_mapTranslate["Compare to [%s] In Beyond Compare"] = "在Beyond Compare中和【%s】进行比较";
	m_mapTranslate["Compare In Beyond Compare"] = "在Beyond Compare中进行比较";
	m_mapTranslate["Extract To Current Directory"] = "解压到当前目录";
	m_mapTranslate["Extract To %s"] = "解压到【%s】";
	m_mapTranslate["Add To %s"] = "添加到【%s】";
	m_mapTranslate["Send To"] = "发送";
	m_mapTranslate["Rename"] = "重命名";
	m_mapTranslate["Rename To %s"] = "重命名为【%s】";
	m_mapTranslate["Delete"] = "删除";
	m_mapTranslate["Copy"] = "拷贝";
	m_mapTranslate["Cut"] = "剪切";
	m_mapTranslate["Paste"] = "粘贴";
	m_mapTranslate["Create a New Directory"] = "新建一个目录";
	m_mapTranslate["Create a New Text File"] = "新建一个文本文件";
	m_mapTranslate["Copy Unicode String To Windows Clipboard"] = "拷贝Unicode字符串到Windows剪贴板";
	m_mapTranslate["Property"] = "属性";
	m_mapTranslate["Can NOT rename file %s to %s!\r\nError : "] = "无法将文件【%s】 重命名为 【%s】！\r\n错误原因：";
	m_mapTranslate["Can NOT delete file %s. \r\nReason : %s"] = "无法删除文件【%s】！\r\n错误原因：";
	m_mapTranslate["Are you sure to delete these %d items?"] = "确定要删除这%d项吗？";
	m_mapTranslate["Are you sure to delete \" %s \" ?"] = "确定要删除【%s】 吗？";
	m_mapTranslate["%s_copy(%d)%s"] = "%s_备份(%d)%s";
	m_mapTranslate["Export to file %s successfully!"] = "成功导出到文件 %s！";
	m_mapTranslate["New Folder"] = "新建文件夹";
	m_mapTranslate["New Text File"] = "新建 文本文档";



	// Options
	m_mapTranslate["Property Name"] = "属性";
	m_mapTranslate["Property Value"] = "值";
	m_mapTranslate["Language"] = "语言";
	m_mapTranslate["Navigate Tree"] = "导航树";
	m_mapTranslate["Show"] = "显示";
	m_mapTranslate["Width"] = "窗口宽度";
	m_mapTranslate["External Program"] = "外部工具";
	m_mapTranslate["Explorer List Style"] = "列表格式";
	m_mapTranslate["Row Height"] = "行高";
	m_mapTranslate["Toolbar"] = "工具条";
	m_mapTranslate["Button Size"] = "按钮大小";
	m_mapTranslate["Auto Load From Desktop"] = "自动加载桌面";
	m_mapTranslate["Auto Load From Taskbar"] = "自动加载快速启动";
	m_mapTranslate["Odd Line BK Color(Active)"] = "奇数行背景颜色（激活）";
	m_mapTranslate["Even Line BK Color(Active)"] = "偶数行背景颜色（激活）";
	m_mapTranslate["Odd Line BK Color(Inactive)"] = "奇数行背景颜色（未激活）";
	m_mapTranslate["Even Line BK Color(Inactive)"] = "偶数行背景颜色（未激活）";

	// Toolbar
	m_mapTranslate["%s(%s), Total space %s M, Free space %s M"] = "%s(%s), 容量%sM, 剩余空间%sM";
	m_mapTranslate["%s(%s), Error getting capacity!"] = "%s(%s), 无法获取磁盘容量！";

	// Dir Tree
	m_mapTranslate["Control Panel"] = "控制面板";

	// Favorite dir
	m_mapTranslate["Favorite Directory"] = "收藏夹";
	m_mapTranslate["Add to favorite"] = "添加到收藏夹";
	m_mapTranslate["Group"] = "分组";
	m_mapTranslate["Name"] = "名字";
	m_mapTranslate["Dir"] = "目录";
	m_mapTranslate["Add Dir [%s] To Favorite"] = "把目录 [%s] 添加到收藏夹";
	m_mapTranslate["Delete Group [%s]"] = "删除分组 [%s]";
	m_mapTranslate["Rename Group [%s]"] = "重命名分组 [%s]";
	m_mapTranslate["Open Dir [%s - %s]"] = "打开目录 [%s - %s]";
	m_mapTranslate["Delete Dir [%s]"] = "删除目录 [%s]";
	m_mapTranslate["Rename Dir [%s]"] = "重命名目录 [%s]";
	m_mapTranslate["Are you sure to delete this %s?"] = "确定要删除这个%s吗？";

	// Menu
	m_mapTranslate["Copy"] = "拷贝";
	m_mapTranslate["Delete All Selected Items"] = "删除所有选中的行";
	m_mapTranslate["Insert Item Before This Row"] = "插入一行数据";
	m_mapTranslate["Append Item To Bottom"] = "增加一行数据（在最后面）";
	m_mapTranslate["Move Top"] = "移到顶端";
	m_mapTranslate["Move Up"] = "上移";
	m_mapTranslate["Move Down"] = "下移";
	m_mapTranslate["Move Bottom"] = "移到末端";
	m_mapTranslate["Select All"] = "全选";
	m_mapTranslate["Select None"] = "全不选";
	m_mapTranslate["Delete All Selected Users"] = "删除用户";
	m_mapTranslate["New User"] = "增加用户";
	m_mapTranslate["Delete All Selected Columns"] = "删除所有选中的列";
	m_mapTranslate["Insert Column"] = "插入一个列";
	m_mapTranslate["Append Column"] = "添加一个列（在最后面）";
	m_mapTranslate["Save"] = "保存";

	// Network Clipboard
	m_mapTranslate["Network Clipboard"] = "网络剪贴板";
	m_mapTranslate["Ip Address"] = "IP地址";
	m_mapTranslate["Port"] = "端口";
	m_mapTranslate["Get Clipboard Data"] = "获取剪贴板内容";
	m_mapTranslate["Please select a machine!"] = "请选择一台远端机器！";
	m_mapTranslate["Getting clipboard from %s ......"] = "正在从%s获取剪贴板数据......";
	m_mapTranslate["Get clipboard ended!"] = "获取剪贴板数据结束！";
	m_mapTranslate["Can Not connect to server, ip address : %s, Port : %d."] = "无法连接到远端机器，IP地址 : %s, 端口 ： %d。";
	m_mapTranslate["Send command(%s) failed!"] = "发送命令 ( %s )失败！";
	m_mapTranslate["Received clipboard failed!"] = "接收剪贴板数据失败！";
	m_mapTranslate["Can NOT start net listener with port %d, please check!"] = "无法启动监听端口%d，请检查！";
	m_mapTranslate["Listening Port"] = "本地监听端口";
	m_mapTranslate["Remote Machine"] = "远端机器";
	m_mapTranslate["Listening port was modified, please save the changes and restart application."] = "端口已更改，请保存更改并重启应用。";
	m_mapTranslate["Use Unicode"] = "使用Unicode格式";
	m_mapTranslate["Append Message"] = "附加消息";
}

void CMultiLanguage::Clear()
{
	m_mapTranslate.clear();
}
