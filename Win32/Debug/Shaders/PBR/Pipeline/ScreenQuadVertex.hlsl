// Copyright SaeruHikari 2019, PBR Common Utils.
#ifndef __VERTEX_DEFINATION__
#define __VERTEX_DEFINATION__

struct VertexIn
{
    float3 PosL : POSITION;
    float2 TexC : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_Position;
    float2 TexC : TEXCOORD;
    //float3 PosV : POSITION;
};

#endif