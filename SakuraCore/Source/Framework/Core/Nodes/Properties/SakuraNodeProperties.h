/*****************************************************************************************
								 Copyrights   SaeruHikari
CreateDate:								2019.10.30
Description:				Sakura scene node properties.
Details:				Includes transform(loc, rot, scale) etc... 
*******************************************************************************************/
#pragma once
#include <DirectXMath.h>
#include "Framework/Interface/ISObject.h"

using namespace DirectX;

namespace SakuraMath
{
	struct SVector : public DirectX::XMFLOAT3
	{
		SVector() = default;
		SVector(float _x, float _y, float _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}
		void operator*=(const SVector& toMul)
		{
			auto vec = XMLoadFloat3(this);
			auto vec2 = XMLoadFloat3(&toMul);	
			vec2 = vec* vec2;
			XMStoreFloat3(this, vec2);
		}
		SVector& operator*(const SVector& toMul)
		{
			SVector result = {0, 0, 0};
			auto vec = XMLoadFloat3(this);
			auto vec2 = XMLoadFloat3(&toMul);
			vec2 = vec * vec2;
			XMStoreFloat3(&result, vec2);
			return result;
		}
	};
}

namespace SakuraCore
{
	struct STransform : public ISSilentObject
	{
	public:
		STransform() = default;
		STransform(const float& lx, const float& ly, const float& lz,
			const float& rx, const float& ry, const float& rz,
			const float& sx, const float& sy, const float& sz,
			const float& ox, const float& oy, const float& oz)
		{
			Location.x = lx; Location.y = ly; Location.z = lz;
			Rotation.x = rx; Rotation.y = ry; Rotation.z = rz;
			Scale.x = sx; Scale.y = sy; Scale.z = sz;
			Origin.x = ox; Origin.y = oy; Origin.z = oz;
		}
		STransform(const SakuraMath::SVector&& _Loc, const SakuraMath::SVector&& _Rot, const SakuraMath::SVector&& _Scale)
			:Location(_Loc), Rotation(_Rot), Scale(_Scale)
		{
			
		}
		__forceinline void set(const float& lx, const float& ly, const float& lz,
			const float& rx, const float& ry, const float& rz,
			const float& sx, const float& sy, const float& sz,
			const float& ox, const float& oy, const float& oz)
		{
			Location.x = lx; Location.y = ly; Location.z = lz;
			Rotation.x = rx; Rotation.y = ry; Rotation.z = rz;
			Scale.x = sx; Scale.y = sy; Scale.z = sz;
			Origin.x = ox; Origin.y = oy; Origin.z = oz;
		}
		SakuraMath::SVector Location = { 0, 0, 0 };
		SakuraMath::SVector Rotation = { 0, 0, 0 };
		SakuraMath::SVector Scale = { 1, 1, 1 };
		SakuraMath::SVector Origin = { 0, 0, 0 };
	};
}
