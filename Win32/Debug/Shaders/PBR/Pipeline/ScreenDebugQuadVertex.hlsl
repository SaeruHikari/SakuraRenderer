// Copyright SaeruHikari 2019, PBR Common Utils.
#ifndef __VERTEX_DEFINATION__
#define __VERTEX_DEFINATION__

struct VertexIn
{
    float3 PosL : POSITION;
    float2 TexC : TEXCOORD;
    int    Type : TYPE;
};

struct VertexOut
{
    float4 PosH : SV_Position;//?
    float2 TexC : TEXCOORD;
    int    Type : TYPE;
};

#endif