#define SAKURACORE_API_EXPORTS
#include "SakuraCore.h"
#include "Framework\Core\SCore.h"

SAKURACORE_API bool __stdcall CreateSakuraCore(UINT GraphicsAPIMask)
{
	return (SakuraCore::SCore::CreateSakuraCore(CORE_GRAPHICS_API_CONF(GraphicsAPIMask)) != nullptr);
}

SAKURACORE_API bool __stdcall InitSakuraGraphicsCore(HWND hwnd, UINT width, UINT height)
{
	return SakuraCore::SCore::GetSakuraCore()->SakuraInitializeGraphicsCore(hwnd, width, height);
}

SAKURACORE_API void __stdcall TickSakuraCore(double deltaTime, UINT coreMask /*= 0*/)
{
	SakuraCore::SCore::GetSakuraCore()->TickSakuraCore(deltaTime, coreMask);
}

SAKURACORE_API void __stdcall MsgSakuraCore(UINT coreMask, UINT MSG, UINT param0, UINT param1, UINT param2)
{
	if ((coreMask & SAKURA_CORE_COMPONENT_MASK_GRAPHICS) != 0)
	{
		SakuraCore::SCore::GetSakuraCore()->MsgSakuraGraphicsCore(MSG, param0, param1, param2);
	}
}



