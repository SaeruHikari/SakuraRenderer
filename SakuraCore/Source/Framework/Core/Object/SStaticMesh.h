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
	struct SStaticMeshData : public MeshData, SImplements ISSlientObject
	{
		SStaticMeshData() = default;
		SStaticMeshData(MeshData&& meshdata)
		{
			Vertices = std::move(meshdata.Vertices);
			Indices32 = std::move(meshdata.Indices32);
			ISSlientObject::ISSlientObject();
		}
	};
	class SStaticMesh : SImplements ISSlientObject
	{
	public:
		SStaticMesh()
		{
			ISSlientObject::ISSlientObject();
			auto GeoGen = std::make_shared<GeometryGenerator>();
			auto sp = GeoGen->CreateSphere(1.f, 50, 50);
			meshData = std::make_shared<SStaticMeshData>(std::move(sp));
		}
		auto GetMeshData()
		{
			return meshData;
		}
	protected:
		std::shared_ptr<SStaticMeshData> meshData;
	};
}
