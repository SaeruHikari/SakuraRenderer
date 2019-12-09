/*****************************************************************************************
								 Copyrights   SaeruHikari
CreateDate:								2019.10.30
Description:				Sakura scene node properties.
Details:				Includes transform(loc, rot, scale) etc... 
*******************************************************************************************/
#pragma once
#include <DirectXMath.h>
#include "Interface/ISObject.h"

using namespace DirectX;

namespace SakuraMath
{
	typedef DirectX::XMFLOAT3 SVector;
}

namespace SakuraCore
{
	struct STransform : public ISSlientObject
	{
	public:
		STransform() = default;
		STransform(const SakuraMath::SVector&& _Loc, const SakuraMath::SVector&& _Rot, const SakuraMath::SVector&& _Scale)
			:Location(_Loc), Rotation(_Rot), Scale(_Scale)
		{
			
		}
		SakuraMath::SVector Location = { 0, 0, 0 };
		SakuraMath::SVector Rotation = { 0, 0, 0 };
		SakuraMath::SVector Scale = { 1, 1, 1 };
	};
}
