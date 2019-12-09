/*****************************************************************************************
				             Copyrights   SaeruHikari
Description:	          Master class of Graphics Managers
Details:		This entity pull graphic functions away from our application.
*******************************************************************************************/
#pragma once
#include "Interface/IRuntimeModule.h"
#include <windows.h>
#include "SakuraCoreUtils.h"

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
		virtual void SetDbgHwnd(HWND hwnd)
		{
			mhDbgWnd = hwnd;
		}
		virtual void Clear() = 0;
		virtual void Draw() = 0;
		virtual void Present() = 0;
		virtual void OnResize(UINT Width, UINT Height) = 0;

		virtual void OnMouseDown(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y) = 0;
		virtual void OnMouseMove(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y) = 0;
		virtual void OnMouseUp(SAKURA_INPUT_MOUSE_TYPES btnState, int x, int y) = 0;
		virtual void OnKeyDown(double deltaTime) = 0;
	protected:
		// HWND
		HWND	  mhMainWnd = nullptr;
#if defined(DEBUG) || defined(_DEBUG)
		// HWND
		HWND	  mhDbgWnd = nullptr;
#endif
	};
	
}