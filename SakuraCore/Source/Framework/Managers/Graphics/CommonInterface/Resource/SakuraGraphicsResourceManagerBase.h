#pragma once
#include <map>
#include "Framework/GraphicTypes/GraphicsCommon/GraphicsConfigs.h"
#include "Interface/IRuntimeModule.h"
#include "Framework/GraphicTypes/GraphicsInterface/ISRenderTarget.h"
#include "Framework/GraphicTypes/GraphicsInterface/ISTexture.h"



namespace SGraphics
{
	class SakuraGraphicsResourceManagerBase : public SakuraCore::IRuntimeModule
	{
	protected:
		SakuraGraphicsResourceManagerBase()
			:IRuntimeModule()
		{
			
		}
		SakuraGraphicsResourceManagerBase(const SakuraGraphicsResourceManagerBase& rhs) = delete;
		SakuraGraphicsResourceManagerBase& operator=(const SakuraGraphicsResourceManagerBase& rhs) = delete;
	public:
		~SakuraGraphicsResourceManagerBase() {};
		// Initialize function.
		virtual bool Initialize() = 0;
		// Finalize function.
		virtual void Finalize() = 0;
		// Tick function, be called per frame.
		virtual void Tick(double deltaTime) = 0;
		//
		virtual bool LoadTextures(std::wstring Filename, std::string registName) = 0;
		
		virtual ISTexture* GetTexture(std::string registName) = 0;
		virtual int RegistNamedRenderTarget(std::string resgistName, 
			ISRenderTargetProperties rtProp, std::string targetSrvHeap = "NULL", std::string targetRtvHeap = "NULL") = 0;
		virtual ISRenderTarget* GetRenderTarget(std::string registName) = 0;
	};

}