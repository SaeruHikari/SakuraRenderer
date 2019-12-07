// Copyright SaeruHikari 2019, PBR Common Utils.
#ifndef __VERTEX_DEFINATION__
#define __VERTEX_DEFINATION__
#include "PassCommon.hlsl"
struct VertexIn
{
    float3 PosL    : POSITION;
    float3 VertexColor : COLOR;
    float3 NormalL : NORMAL;
    float2 TexC    : TEXCOORD;
    float3 Tangent : TANGENT;
};

struct VertexOut
{
    float3 PosL    : POSITIONL;
    float4 PosH    : SV_Position;
    float3 PosW    : POSITION;
    float3 NormalW : NORMAL;
    float2 TexC    : TEXCOORD;
    float3 Tangent : TANGENT;
    float3 BiNormal: TBNOUT;
#if defined(TAA_ENABLED)
    float4 CurPosVP    : POSITION1;
    float4 PrevPosVP : POSITION2;
#endif
};

#endif