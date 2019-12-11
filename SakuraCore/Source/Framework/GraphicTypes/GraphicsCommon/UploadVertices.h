#pragma once
#include "Common/HikaUtils/HikaCommonUtils/MathHelper.h"

struct ScreenQuadDebugVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 TexC = { 0, 0 };
	int Type = 0;
};

struct ScreenQuadVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 TexC = { 0, 0 };
};

struct StandardVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT4 Color;
	DirectX::XMFLOAT3 Normal = {0, 0, 1};
	DirectX::XMFLOAT2 TexC = {0, 0};
	DirectX::XMFLOAT3 Tangent = {0, 0, 1};
};