/*****************************************************************************************
				             Copyrights   SaeruHikari
Description:	          Master class of Graphics Managers
Details:		This entity pull graphic functions away from our application.
*******************************************************************************************/
#pragma once
#include "Framework/Interface/IRuntimeModule.h"
#include <windows.h>
#include "SakuraCoreUtils.h"
#include "Resource/SakuraGraphicsResourceManagerBase.h"
#include "../../Scene/SSceneManager.h"

namespace SakuraCore
{ 
	class SakuraGraphicsManagerBase : SImplements IRuntimeModule
	{
	protected:
		// Hide constructors to prevent unexpected instance for virtual class.
		SakuraGraphicsManagerBase()
		{
			pGraphicsManager = this;
		}
		SakuraGraphicsManagerBase(const SakuraGraphicsManagerBase& rhs) = delete;
		SakuraGraphicsManagerBase& operator=(const SakuraGraphicsManagerBase& rhs) = delete;
	public:
		virtual ~SakuraGraphicsManagerBase() {};
		virtual void SetHwnd(HWND hwnd)
		{
			mhMainWnd = hwnd;
		};
		HWND GetHwnd()
		{
			return mhMainWnd;
		}
		virtual void Clear() = 0;
		virtual void Draw() = 0;
		virtual void Present() = 0;
		virtual void OnResize(UINT Width, UINT Height) = 0;
		virtual SRenderItem* SelectSceneObject(int sx, int sy) = 0;
		virtual void OnMouseDown(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y) = 0;
		virtual void OnMouseMove(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y) = 0;
		virtual void OnMouseUp(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y) = 0;
		virtual void OnKeyDown(double deltaTime) = 0;
		virtual SRenderScene* GetRenderScene()
		{
			return pSceneManager->GetRenderScene();
		}
	protected:
		std::unique_ptr<SGraphics::SakuraGraphicsResourceManagerBase> pGraphicsResourceManager;
		// HWND
		HWND	  mhMainWnd = nullptr;
	};
	
}