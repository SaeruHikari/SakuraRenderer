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
		SRenderItem* AddOpaque(SStaticMesh* data);
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
