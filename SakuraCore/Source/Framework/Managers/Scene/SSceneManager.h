#pragma once
#include "Interface/IRuntimeModule.h"
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
		SIndex RegistMesh(SStaticMesh* data, std::string matname = "NULL");
		// Material need to be registed in graphics layer.
		SIndex RegistOpaqueMat(SMaterial* material, std::string name);

		SRenderItem* GetRenderItem(SIndex index);
		SMaterial* GetMaterial(std::string Name);
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
