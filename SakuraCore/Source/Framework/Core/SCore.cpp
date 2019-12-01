#include "SCore.h"
#include <assert.h>
#define DX_MANAGER_VERSION 0
#if (DX_MANAGER_VERSION == 0)
#include "../Managers\Graphics\D3D12\SDxRendererGM.h"
#define ManagerClass SGraphics::SDxRendererGM
#endif

SakuraCore::SCore* SakuraCore::SCore::mCore = nullptr;
SakuraCore::SCore::SCore(CORE_GRAPHICS_API_CONF gAPI)
{
	if (gAPI == CORE_GRAPHICS_API_CONF::SAKURA_DRAW_WITH_D3D12)
	{
		std::unique_ptr<SGraphics::SakuraGraphicsManagerBase> pD3DGraphicsManager = std::make_unique<ManagerClass>();
		mGraphicsManager = std::move(pD3DGraphicsManager);
	}
	else if (gAPI == CORE_GRAPHICS_API_CONF::SAKURA_DRAW_WITH_VULKAN)
	{
		assert(0);
	}
	else
	{
		assert(0);
	}
};


bool SakuraCore::SCore::SakuraInitializeGraphicsCore(HWND hwnd, UINT width, UINT height)
{
	mGraphicsManager->SetHwnd(hwnd);
	if (mGraphicsManager->Initialize())
		mGraphicsManager->OnResize(width, height);
	else return false;
	return true;
}

bool SakuraCore::SCore::SakuraBindDbgHwnd(HWND hwnd, UINT weight, UINT height)
{
	mGraphicsManager->SetDbgHwnd(hwnd);
	return true;
}

void SakuraCore::SCore::TickSakuraCore(double deltaTime, UINT mask)
{
	if ((mask & SAKURA_CORE_COMPONENT_MASK_GRAPHICS) != 0)
	{
		mGraphicsManager->Tick(deltaTime);
		// Current simply calls draw.
		mGraphicsManager->Draw();
	}
}

void SakuraCore::SCore::MsgSakuraGraphicsCore(UINT Msg, UINT param0, UINT param1, UINT param2)
{
	if ((Msg & SAKURA_GRAPHICS_CORE_MSG_RESIZE) != 0)
	{
		mGraphicsManager->OnResize(param1, param2);
	}
	if ((Msg & SAKURA_GRAPHICS_CORE_MSG_MOUSEPRESS) != 0)
	{
		mGraphicsManager->OnMouseDown((SAKURA_INPUT_MOUSE_TYPES)param0, param0, param1);
	}
	if ((Msg & SAKURA_GRAPHICS_CORE_MSG_MOUSERELEASE) != 0)
	{
		mGraphicsManager->OnMouseUp((SAKURA_INPUT_MOUSE_TYPES)param0, param1, param2);
	}
	if ((Msg & SAKURA_GRAPHICS_CORE_MSG_MOUSEMOVE) != 0)
	{
		mGraphicsManager->OnMouseMove((SAKURA_INPUT_MOUSE_TYPES)param0, param1, param2);
	}
	if ((Msg & SAKURA_GRAPHICS_CORE_MSG_KEYPRESS) != 0)
	{
		mGraphicsManager->OnKeyDown((SAKURA_INPUT_KEY)param0);
	}
}
