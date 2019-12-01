/*****************************************************************************************
								 Copyrights   SaeruHikari
CreateDate:								2019.10.22
Description:	  Sakura Core Reference, contains core functions and components.
Details:			
*******************************************************************************************/
#pragma once
#include "../../Interface/IRuntimeModule.h"
#include <memory>
#include "../Managers/Graphics/SakuraGraphicsManagerBase.h"
#include "SakuraCoreUtils.h"

namespace SakuraCore {

	class SCore
	{
	private:
		SCore(CORE_GRAPHICS_API_CONF gAPI);
		SCore(const SCore& rhs) = delete;
		SCore& operator=(const SCore& rhs) = delete;
	public:
		static SCore* CreateSakuraCore(CORE_GRAPHICS_API_CONF gAPI)
		{
			if (mCore == nullptr)
				mCore = new SCore(gAPI);
			return mCore;
		};
		static SCore* GetSakuraCore()
		{
			assert(mCore != nullptr);
			return mCore;
		}
	public:
		bool SakuraInitializeGraphicsCore(HWND hwnd, UINT weight, UINT height);
		bool SakuraBindDbgHwnd(HWND hwnd, UINT weight, UINT height);
		
	public:
		// Tick group.
		void TickSakuraCore(double deltaTime, UINT mask = 0);
		void MsgSakuraGraphicsCore(UINT Msg, UINT param0, UINT param1, UINT param2);
		~SCore() {};


	private:
		static SCore* mCore;
		std::unique_ptr<SGraphics::SakuraGraphicsManagerBase> mGraphicsManager;
	};
}


