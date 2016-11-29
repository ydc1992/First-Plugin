// MyPlugin.cpp : 定义 DLL 应用程序的导出函数。

#include "stdafx.h"
#include <tchar.h>
#include <malloc.h>
#include <stdio.h>

#define MVV_1 0
#define MVV_2 8
#define MVV_3 15
#define MVV_4 0
#define MVV_4a ""

#define FILENAMEPTR(p) (p).FindData.cFileName

// {096AC917-738A-45A1-8F6A-3E43C523236C}
static const GUID guid_PluginGuid =
{ 0x96ac917, 0x738a, 0x45a1,{ 0x8f, 0x6a, 0x3e, 0x43, 0xc5, 0x23, 0x23, 0x6c } };

// {2B829E4A-7707-422F-B063-0D7537130951}
static const GUID guid_PluginMenu =
{ 0x2b829e4a, 0x7707, 0x422f,{ 0xb0, 0x63, 0xd, 0x75, 0x37, 0x13, 0x9, 0x51 } };


PluginStartupInfo psi;
FarStandardFunctions fsf;

void WINAPI GetGlobalInfoW(struct GlobalInfo *Info)
{

	Info->MinFarVersion = /*FARMANAGERVERSION*/
		MAKEFARVERSION(
			FARMANAGERVERSION_MAJOR,
			FARMANAGERVERSION_MINOR,
			FARMANAGERVERSION_REVISION,
			FARMANAGERVERSION_BUILD,
			FARMANAGERVERSION_STAGE);

	Info->Version = MAKEFARVERSION(MVV_1, MVV_2, MVV_3, ((MVV_1 % 100) * 100000) + (MVV_2 * 1000) + (MVV_3 * 10) + (MVV_4 % 10), VS_RELEASE);

	Info->Guid = guid_PluginGuid;
	Info->Title = L"First Plugin";
	Info->Description = L"PE Import/Export browser";
	Info->Author = L"ConEmu.Maximus5@gmail.com";
}

void WINAPI GetPluginInfoW(struct PluginInfo *pi)
{
	static wchar_t* szMenu = L"Impex1";

	pi->StructSize = sizeof(PluginInfo);
	pi->Flags = PF_FULLCMDLINE;
	pi->CommandPrefix = szMenu;

	// 显示菜单
	pi->PluginMenu.Count = 1;
	pi->PluginMenu.Guids = &guid_PluginMenu;
	pi->PluginMenu.Strings = &szMenu;

}

void WINAPI SetStartupInfoW(struct PluginStartupInfo *Info) {
	psi = *Info;
	fsf = *Info->FSF;
	psi.FSF = &fsf;
}

HANDLE WINAPI OpenW(const struct OpenInfo *Info) {
	OPENFROM OpenFrom = Info->OpenFrom;
	INT_PTR Item = Info->Data;

	const HANDLE PanelStop = PANEL_STOP;
	const HANDLE PanelNone = NULL;

	PluginPanelItem* item = NULL;
	LPCTSTR pszFileName = NULL;

	HANDLE hPlugin = PanelNone;
	if (OpenFrom == OPEN_PLUGINSMENU) {
		PanelInfo pi = {};
		pi.StructSize = sizeof(pi);
		// 获取面板信息
		if (psi.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELINFO, 0, &pi)) {
			//是文件面板并且列表不为空
			if (pi.PanelType == PTYPE_FILEPANEL && pi.ItemsNumber > 0) {  
				if (pi.CurrentItem < 0)
					return NULL;

				//获取鼠标指向的那一项
				INT_PTR nSize= psi.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, pi.CurrentItem, NULL);

				item = (PluginPanelItem*)calloc(nSize, 1);
				FarGetPluginPanelItem gpi = { sizeof(gpi),nSize,item };
				//得到面板项的文件信息
				psi.PanelControl(INVALID_HANDLE_VALUE, FCTL_GETPANELITEM, pi.CurrentItem, &gpi);
 				pszFileName = item->FileName;

				TCHAR* pszFull = NULL;
				int nLen = 0;
				if(pszFileName && *pszFileName && _tcscmp(pszFileName, _T("..")) != 0){
					//先获取文件名长度,以免缓冲区不足。
				    nLen = fsf.ConvertPath(CPM_FULL, pszFileName, NULL, 0);
					if (nLen > 0) {
						pszFull = (TCHAR*)calloc(nLen, sizeof(TCHAR));
						fsf.ConvertPath(CPM_FULL, pszFileName, pszFull, nLen);
						if (pszFileName) {
					
						}
					}
				}

			}
		}
	
	}
	else if(OpenFrom == OPEN_ANALYSE) //打开的文件的内容
	{
		
	}
	else if (OpenFrom ==OPEN_COMMANDLINE)
	{

	}


	return hPlugin;
}