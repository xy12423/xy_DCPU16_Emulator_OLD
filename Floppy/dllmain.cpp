// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "defines.h"
#include "floppy.h"
#include "export.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		info.a = 0x24C5;
		info.b = 0x4FD5;
		info.c = 0x000B;
		info.x = 0x7E91;
		info.y = 0x1EB3;
		info.hwi = &intrpt;
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

