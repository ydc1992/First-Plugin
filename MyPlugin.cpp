// MyPlugin.cpp : ���� DLL Ӧ�ó���ĵ���������

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
#define RTTYPE(p,q)  if(p)return q;
#define SAFECLOSEHANDLE(p,q) if(!p){CloseHandle(p);return q;}

//ȫ�ֱ���
AnalyseInfo * gpAnalyInfo = NULL;
//psi �� fsf ������ָ�����ͣ�����ʹ�øñ����ᱨ��
PluginStartupInfo psi;
FarStandardFunctions fsf;

//************************************
// Method:    GetGlobalInfoW
// FullName:  GetGlobalInfoW
// Access:    public
// Returns:   void WINAPI
// Qualifier:  Far���ȼ����������������˵���F3��ʾ�����ݣ���������
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

	Info->Version = Info->MinFarVersion;
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
// Qualifier: ��ȡ�����Ϣ����������
// Parameter: struct PluginInfo * Info
//************************************
void WINAPI  GetPluginInfoW(PluginInfo *Info)
{
	static wchar_t* szMenu;
	szMenu = _T("First Plugin");

	Info->StructSize = sizeof(PluginInfo);
	Info->Flags = PF_FULLCMDLINE;
	Info->CommandPrefix = szMenu;

	//����˵���Ҫ�ڲ���˵�����ʾ�˵��������ʼ�������ȱһ���ɣ���Ȼ�ᱨ��
	Info->PluginMenu.Count = 1;
	Info->PluginMenu.Guids = &guid_PluginMenu;
	Info->PluginMenu.Strings = &szMenu;
}

//************************************
// Method:    SetStartupInfoW
// FullName:  SetStartupInfoW
// Access:    public
// Returns:   void WINAPI
// Qualifier: ���ò��������Ϣ������Ҫ����������
// Parameter: const PluginStartupInfo * Info
//************************************
void WINAPI SetStartupInfoW(const PluginStartupInfo *Info)
{
	//���ݸ�ȫ�ֱ������Ա��ں�����á�
	psi = *Info;
	fsf = *Info->FSF;
	psi.FSF = &fsf;
}

//************************************
// Method:    GetFullPath
// FullName:  GetFullPath
// Access:    public
// Returns:   LPTSTR
// Qualifier: ��õ�ǰѡ������ļ�����·��
//************************************
LPTSTR GetCurrentSelectItemFullPath()
{
	PanelInfo pi;
	wchar_t* lpFullName = NULL;
	//1. ��ȡ�����Ϣ
	RTTYPE(!psi.PanelControl(PANEL_ACTIVE, FCTL_GETPANELINFO, 0, &pi), NULL);

	RTTYPE(!(pi.PanelType == PTYPE_FILEPANEL && pi.ItemsNumber > 0), NULL);
	RTTYPE(pi.CurrentItem < 0, NULL);
	//2. ��ȡ�����

	//2.1 �����size
	INT_PTR size = psi.PanelControl(PANEL_ACTIVE, FCTL_GETPANELITEM, pi.CurrentItem, NULL);
	RETURNULL(size);
	//2.2 ���������ռ�
	PluginPanelItem* ppi = (PluginPanelItem*)malloc(size);
	RETURNULL(ppi);
	// 2.3 �ٴε��ã����ѡ������ϸ��Ϣ
	FarGetPluginPanelItem FGPPI = { sizeof(FarGetPluginPanelItem),size,ppi };
	psi.PanelControl(PANEL_ACTIVE, FCTL_GETPANELITEM, pi.CurrentItem, &FGPPI);

	// 3.�ļ�����·��
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
// Qualifier:�����������ʼ������
// Parameter: const struct OpenInfo * Info
//************************************
HANDLE WINAPI OpenW(const struct OpenInfo *Info)
{
	OPENFROM OpenFrom = Info->OpenFrom;
	wchar_t* pszname = (wchar_t*)malloc(MAX_PATH*2);

	//�Ӳ���˵���
	if (OpenFrom == OPEN_PLUGINSMENU)
	{
		pszname = GetCurrentSelectItemFullPath();
	}
	MessageBox(NULL, pszname, L"Test", 0);
	DUMPFILE(pszname);
	return NULL;
}

//************************************
// Method:    DUMPFILE
// FullName:  DUMPFILE
// Access:    public
// Returns:   bool
// Qualifier: �����ļ�����
// Parameter: LPTSTR lpFileName
//************************************
bool DUMPFILE(LPTSTR lpFileName)
{
	//����ļ����
	HANDLE hFile = CreateFile(lpFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFile);
		return false;
	}
	ULARGE_INTEGER filesize;
	// ��ȡ�ļ���С
	filesize.LowPart = GetFileSize(hFile, &filesize.HighPart);
	//�����ļ�ӳ��
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

