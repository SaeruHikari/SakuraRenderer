#pragma once
#include "Framework/Interface/ISObject.h"
#include "Common/EngineDefinations/MeshData.hpp"
#include "Common/HikaUtils/HikaCommonUtils/GeometryGenerator.h"

using namespace SCommon;
using namespace SakuraCore;

namespace SEngine
{
	class SMeshGenerator
	{
		
	};

	class SStaticMesh : SImplements ISSilentObject
	{
	public:
		SStaticMesh(const StaticMeshData& meshData)
		{
			ISSilentObject::ISSilentObject();

		}
		SStaticMesh()
		{
			ISSilentObject::ISSilentObject();
			auto GeoGen = std::make_shared<GeometryGenerator>();
			auto sp = GeoGen->CreateSphere(1.f, 50, 50);
			meshData = std::make_unique<StaticMeshData>(std::move(sp));
		}
		~SStaticMesh()
		{
			meshData.reset();
		}
		auto GetMeshData()
		{
			return meshData.get();
		}
	protected:
		std::unique_ptr<StaticMeshData> meshData;
	};
}
