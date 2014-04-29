// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "defines.h"
#include "3DDisplay.h"
#include "export.h"

HANDLE mainThread = NULL;
DWORD threadID = 0;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		info.a = 0xbf3c;
		info.b = 0x42ba;
		info.c = 0x0003;
		info.x = 0x7e91;
		info.y = 0x1eb3;
		info.hwi = &intrpt;
		mainThread = CreateThread(NULL, 0, &MainThread, NULL, 0, &threadID);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		if (GetCurrentThread() == mainThread)
			CloseHandle(mainThread);
		break;
	}
	return TRUE;
}

