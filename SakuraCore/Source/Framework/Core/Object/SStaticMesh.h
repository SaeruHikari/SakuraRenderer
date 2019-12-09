#pragma once
#include "Interface/ISObject.h"
#include "Common/EngineDefinations/MeshData.hpp"
#include "Common/HikaUtils/HikaCommonUtils/GeometryGenerator.h"

using namespace SCommon;
using namespace SakuraCore;

namespace SEngine
{
	class SMeshGenerator
	{
		
	};

	class SStaticMesh : SImplements ISSlientObject
	{
	public:
		SStaticMesh()
		{
			ISSlientObject::ISSlientObject();
			auto GeoGen = std::make_shared<GeometryGenerator>();
			auto sp = GeoGen->CreateSphere(1.f, 50, 50);
			meshData = std::make_unique<StaticMeshData>(std::move(sp));
		}
		auto GetMeshData()
		{
			return meshData.get();
		}
	protected:
		std::unique_ptr<StaticMeshData> meshData;
	};
}
