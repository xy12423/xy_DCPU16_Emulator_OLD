// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "defines.h"
#include "clock.h"
#include "export.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		info.a = 0xB402;
		info.b = 0x12D0;
		info.c = 0x0001;
		info.x = 0x2EBC;
		info.y = 0x6F78;
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

