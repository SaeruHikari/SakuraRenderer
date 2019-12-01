/*****************************************************************************************
								 Copyrights   SaeruHikari
CreateDate:								2019.10.30
Description:				Sakura scene node properties.
Details:				Includes transform(loc, rot, scale) etc... 
*******************************************************************************************/
#pragma once
#include <DirectXMath.h>

using namespace DirectX;

namespace SakuraCore
{
	struct STransform
	{
	public:
		STransform() = default;
		STransform(const XMFLOAT3&& _Loc, const XMFLOAT3&& _Rot, const XMFLOAT3&& _Scale)
			:Location(_Loc), Rotation(_Rot), Scale(_Scale)
		{

		}
		XMFLOAT3 Location = { 0, 0, 0 };
		XMFLOAT3 Rotation = { 0, 0, 0 };
		XMFLOAT3 Scale = { 1, 1, 1 };
	};
}
