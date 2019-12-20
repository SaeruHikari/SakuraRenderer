/*****************************************************************************************
								 Copyrights   SaeruHikari
CreateDate:								2019.10.22
Description:	  Sakura Core Reference, contains core functions and components.
Details:			
*******************************************************************************************/
#pragma once
#include "../../Interface/IRuntimeModule.h"
#include <memory>
#include "../Managers/Graphics/CommonInterface/SakuraGraphicsManagerBase.h"
#include "SakuraCoreUtils.h"
#include "../Managers/Scene/SSceneManager.h"
#include <Common/HikaUtils/HikaCommonUtils/GameTimer.h>

using namespace SScene;

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
		bool SakuraInitScene();
		bool SakuraInitializeGraphicsCore(HWND hwnd, UINT weight, UINT height);

		int Run();
		void MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	public:
		// Tick group.
		void TickSakuraCore(double deltaTime, UINT mask = 0);
		void MsgSakuraGraphicsCore(UINT Msg, UINT param0, UINT param1, UINT param2);
		~SCore() {};


	private:
		bool bRunning = false;
		bool mAppPaused = false;
		std::unique_ptr<GameTimer> mTimer;
		static SCore* mCore;
		std::shared_ptr<SScene::SakuraScene> CurrScene;
		std::shared_ptr<SSceneManager> CurrSceneMng;
		std::unique_ptr<SakuraGraphicsManagerBase> mGraphicsManager;
	};
}


