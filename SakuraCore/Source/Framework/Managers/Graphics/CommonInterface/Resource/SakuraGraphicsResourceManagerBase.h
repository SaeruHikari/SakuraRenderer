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
		template<typename TextureClass, 
			typename std::enable_if<std::is_convertible<TextureClass*, ISTexture*>::value>::type * = nullptr>
		__forceinline TextureClass* LoadTextures(std::wstring Filename, std::string textName)
		{
			return (TextureClass*)LoadTexture(Filename, textName);
		}
		virtual ISTexture* LoadTexture(std::wstring Filename, std::string textName) = 0;
		virtual ISRenderTarget* CreateNamedRenderTarget(std::string resgistName, 
			ISRenderTargetProperties rtProp, std::string targetSrvHeap, std::string targetRtvHeap, SRHIResource* resource = nullptr) = 0;
		virtual ISRenderTarget* CreateNamedRenderTarget(std::string registName,
			ISRenderTargetProperties rtProp, SRHIResource* resource, SResourceHandle srvHandle, SResourceHandle rtvHandle) = 0;
		virtual SResourceHandle* GetResourceHandle(std::string resourceName)
		{
			if (mResources.find(resourceName) != mResources.end())
			{
				return mResources[resourceName]->GetResourceHandle();
			}
		}
		
		__forceinline virtual ISRenderResource* GetNamedResource(const std::string& name)
		{
			if (mResources.find(name) != mResources.end())
				return mResources[name].get();
			else
				return nullptr;
		}

		virtual SResourceHandle* GetRenderTargetHandle(std::string rtName)
		{
			if (mResources.find(rtName) != mResources.end())
			{
#if defined(DEBUG) || defined(_DEBUG)
				return dynamic_cast<ISRenderTarget*>(mResources[rtName].get())->GetRenderTargetHandle();
#else
				return ((ISRenderTarget*)mResources[rtName].get())->GetRenderTargetHandle();
#endif
			}
		}
	protected:
		std::unordered_map<std::string, std::unique_ptr<ISRenderResource>> mResources;
	};
}