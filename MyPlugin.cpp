// MyPlugin.cpp : 定义 DLL 应用程序的导出函数。

#include "stdafx.h"
#include "plugin.hpp"
#include <tchar.h>
#include <malloc.h>
#include <stdio.h>

bool DUMPFILE(LPTSTR lpFileName);


//GUID

// {1D228F40-FE40-4663-92D9-EB184E455AF1}
static const GUID guid_PluginMenu =
{ 0x1d228f40, 0xfe40, 0x4663,{ 0x92, 0xd9, 0xeb, 0x18, 0x4e, 0x45, 0x5a, 0xf1 } };

// {1C6B0907-0B96-47C4-A24B-36A9B184AF25}
static const GUID guid_PluginGuid =
{ 0x1c6b0907, 0xb96, 0x47c4,{ 0xa2, 0x4b, 0x36, 0xa9, 0xb1, 0x84, 0xaf, 0x25 } };

#define SAFEFREE(p)  if(!(p)){free((p));(p) = NULL;}
#define RETURNULL(p) if(!(p))return NULL;
#define RTTYPE(p,q)  if(p)return (q);
#define SAFECLOSEHANDLE(p,q) if(!p){CloseHandle(p);return (q);}

//全局变量
AnalyseInfo * gpAnalyInfo = NULL;
//psi 和 fsf 不能是指针类型，否则使用该变量会报错
PluginStartupInfo psi;
FarStandardFunctions fsf;

//************************************
// Method:    GetGlobalInfoW
// FullName:  GetGlobalInfoW
// Access:    public
// Returns:   void WINAPI
// Qualifier:  Far最先加载这个函数，插件菜单中F3显示的内容，导出函数
// Parameter: struct GlobalInfo * Info
//************************************
void WINAPI GetGlobalInfoW(GlobalInfo *Info)
{
	Info->MinFarVersion = MAKEFARVERSION(
		FARMANAGERVERSION_MAJOR,
		FARMANAGERVERSION_MINOR,
		FARMANAGERVERSION_REVISION,
		FARMANAGERVERSION_BUILD,
		FARMANAGERVERSION_STAGE);

	Info->StructSize = sizeof(GlobalInfo);
	Info->Author = L"Ken";
	Info->Description = L"My First Plugin";
	Info->Title = L"My First Plugin";
	Info->Guid = guid_PluginGuid;
}

//************************************
// Method:    GetPluginInfoW
// FullName:  GetPluginInfoW
// Access:    public
// Returns:   void WINAPI
// Qualifier: 获取插件信息，导出函数
// Parameter: struct PluginInfo * Info
//************************************
void WINAPI  GetPluginInfoW(PluginInfo *Info)
{
	static wchar_t* szMenu;
	szMenu = _T("First Plugin");

	Info->StructSize = sizeof(PluginInfo);
	Info->Flags = PF_FULLCMDLINE;
	Info->CommandPrefix = szMenu;

	//插件菜单，要在插件菜单中显示菜单，必须初始化这三项，缺一不可，不然会报错
	Info->PluginMenu.Count = 1;
	Info->PluginMenu.Guids = &guid_PluginMenu;
	Info->PluginMenu.Strings = &szMenu;
}

//************************************
// Method:    SetStartupInfoW
// FullName:  SetStartupInfoW
// Access:    public
// Returns:   void WINAPI
// Qualifier: 设置插件启动信息，很重要。导出函数
// Parameter: const PluginStartupInfo * Info
//************************************
void WINAPI SetStartupInfoW(const PluginStartupInfo *Info)
{
	//传递给全局变量，以便在后面调用。
	psi = *Info;
	fsf = *Info->FSF;
	psi.FSF = &fsf;
}

//************************************
// Method:    GetFullPath
// FullName:  GetFullPath
// Access:    public
// Returns:   LPTSTR
// Qualifier: 获得当前选中项的文件完整路径
//************************************
LPTSTR GetCurrentSelectItemFullPath()
{
	PanelInfo pi;
	wchar_t* lpFullName = NULL;
	//1. 获取面板信息
	RTTYPE(!psi.PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, &pi), NULL);

	RTTYPE(!(pi.PanelType == PTYPE_FILEPANEL && pi.ItemsNumber > 0), NULL);
	RTTYPE(pi.CurrentItem < 0, NULL);
	//2. 获取面板项

	//2.1 获得其size
	INT_PTR size = psi.PanelControl(PANEL_ACTIVE, FCTL_GETPANELITEM, pi.CurrentItem, NULL);
	RETURNULL(size);
	//2.2 分配面板项空间
	PluginPanelItem* ppi = (PluginPanelItem*)malloc(size);
	RETURNULL(ppi);
	// 2.3 再次调用，获得选中项详细信息
	FarGetPluginPanelItem FGPPI = { sizeof(FarGetPluginPanelItem),size,ppi };
	psi.PanelControl(PANEL_ACTIVE, FCTL_GETPANELITEM, pi.CurrentItem, &FGPPI);

	// 3.文件完整路径
	wchar_t* lpFileName = (wchar_t*)ppi->FileName;
	RTTYPE((lpFileName && *lpFileName && _tcscmp(lpFileName, L"..") == 0), NULL);

	size_t nLen = fsf.ConvertPath(CPM_FULL, lpFileName, NULL, 0);
	RETURNULL(nLen);
	lpFullName = (wchar_t*)malloc(nLen*2);
	fsf.ConvertPath(CPM_FULL, lpFileName, lpFullName, nLen);
	SAFEFREE(ppi);
	return lpFullName;
}

//************************************
// Method:    OpenW
// FullName:  OpenW
// Access:    public
// Returns:   HANDLE WINAPI
// Qualifier:启动插件，开始工作了
// Parameter: const struct OpenInfo * Info
//************************************
HANDLE WINAPI OpenW(const struct OpenInfo *Info)
{
	OPENFROM OpenFrom = Info->OpenFrom;
	wchar_t* pszname = (wchar_t*)malloc(MAX_PATH*2);

	//从插件菜单打开
	if (OpenFrom == OPEN_PLUGINSMENU)
	{
		pszname = GetCurrentSelectItemFullPath();
	}
//	MessageBox(NULL, pszname, L"Test", 0);
	fsf.AddEndSlash(pszname);
	_int64 clock = fsf.FarClock();




	return NULL;
}

//************************************
// Method:    DUMPFILE
// FullName:  DUMPFILE
// Access:    public
// Returns:   bool
// Qualifier: 解析文件数据
// Parameter: LPTSTR lpFileName
//************************************
bool DUMPFILE(LPTSTR lpFileName)
{
	//获得文件句柄
	HANDLE hFile = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return false;
	}
	ULARGE_INTEGER filesize;
	// 获取文件大小
	filesize.LowPart = GetFileSize(hFile, &filesize.HighPart);
	//创建文件映射
	HANDLE hFileMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
	SAFECLOSEHANDLE(hFileMapping, FALSE);
	PBYTE* pbMappedFileBase = (PBYTE*)MapViewOfFile(hFileMapping, FILE_MAP_READ, 0, 0, 0);

	if (pbMappedFileBase == 0)
	{
		CloseHandle(hFileMapping);
		CloseHandle(hFile);
		return FALSE;
	}
	return TRUE;

}

