#pragma once
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>

#include "SakuraCoreUtils.h"

#ifdef SAKURACORE_API_EXPORTS
#define SAKURACORE_API __declspec(dllexport)
#else
#define SAKURACORE_API __declspec(dllimport)
#endif

extern "C"
{
	///<summary>
	///	0000 DX12
	/// 0010 VULKAN
	///</summary>
	SAKURACORE_API bool __stdcall CreateSakuraCore(UINT GraphicsAPIMask);
}


extern "C"
{
	///<summary>
	/// Initialize Sakura Graphics Core.
	///</summary>
	SAKURACORE_API bool __stdcall InitSakuraGraphicsCore(HWND hwnd, UINT width, UINT height);
}

extern "C"
{
	///<summary>
	/// Call Sakura Core Tick.
	/// mask(1 to remove spec work entity from this ticking, default permit all): 
	/// graphics core (0x01)
	///</summary>
	SAKURACORE_API void __stdcall TickSakuraCore(double deltaTime, UINT coreMask = SAKURA_CORE_COMPONENT_MASK_ALL);
}

extern "C"
{
	///<summary>
	/// Send MSG to Sakura Cores.
	///</summary>
	SAKURACORE_API void __stdcall MsgSakuraCore(UINT coreMask, UINT MSG, UINT param0, UINT param1, UINT param2);
}

extern "C"
{
	///<summary>
	/// Call before Init Sakura Graphics Core.
	/// only use it for debugging.
	/// 1. basic bllin-phong 
	/// 2. with translucent blend and alpha-clip.
	///</summary>
	SAKURACORE_API void __stdcall SwitchSGraphicsManagerVersion(UINT mangerMask);
}

extern "C"
{
	///<summary>
	/// Run with windows default msg loop. 
	///</summary>
	SAKURACORE_API int __stdcall Run();
}

extern "C"
{
	///<summary>
	/// Run with windows default msg loop. 
	///</summary>
	SAKURACORE_API void __stdcall MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
}