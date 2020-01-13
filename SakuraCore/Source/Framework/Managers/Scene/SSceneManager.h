#pragma once
#include "Framework/Interface/IRuntimeModule.h"
#include "Framework/Core/Scene/SakuraScene.h"
#include "Framework/GraphicTypes/RenderScene/SRenderScene.hpp"
#include "Framework/Core/Object/SStaticMesh.h"
using namespace SGraphics;
using namespace SEngine;

namespace SakuraCore
{
	class SSceneManager : SImplements IRuntimeModule
	{
	public:
		SSceneManager();
		SIndex RegistRenderItem(const std::string& geomName, const std::string& drawArgName,
			std::string matname = "DefaultMat", ERenderLayer renderLayer = E_Opaque);
		// Material need to be registed in graphics layer.
		SMaterial* RegistOpaqueMat(const std::string& name, OpaqueMaterial& opaqueMat);

		SRenderItem* GetRenderItem(SIndex index);
		SMaterial* GetMaterial(std::string Name);
		__forceinline SRenderScene* GetRenderScene()
		{
			return renderScene.get();
		}
		virtual bool Initialize()
		{
			return true;
		}
		// Finalize function.
		virtual void Finalize()
		{

		}
		// Tick function, be called per frame.
		virtual void Tick(double deltaTime)
		{

		}
	private:
		std::unique_ptr<SRenderScene> renderScene = nullptr;
	};
}
