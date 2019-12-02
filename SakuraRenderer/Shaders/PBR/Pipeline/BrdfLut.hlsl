// Saeru Hikari
//#define SAKURA_ENABLE_ANISO
#include "VertexCommon.hlsl"
//Include structures and functions for lighting
#include "ShadingUtils.hlsl"

// Constant data that varies per frame.
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    vout.PosH = float4(vin.PosL, 1.0f);

    vout.TexC = vin.TexC;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    //return float4(pin.TexC.rg, 0.f, 1.f);
    return float4(1.f, 1.f, 0.f, 1.f);
}

