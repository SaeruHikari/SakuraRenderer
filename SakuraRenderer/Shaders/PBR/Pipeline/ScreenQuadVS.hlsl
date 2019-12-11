// Copyright SaeruHikari 2019, PBR Common Utils.
#include "PassCommon.hlsl"
#include "ScreenQuadVertex.hlsl"
#include "CommonCBs.hlsl"
#include "Utils.hlsl"

VertexOut VS_Portable(VertexIn vin)
{
    VertexOut vout;
    vin.PosL.x = vin.PosL.x;
    vin.PosL.y = vin.PosL.y;
    vout.PosH = float4(vin.PosL, 1.0f);
#if defined(REVERSE_Z)
    vout.PosH.z = gFarZ;
#endif
    vout.TexC = vin.TexC;

    // Transform quad corners to view space near plane.
    //float4 ph = mul(vout.PosH, gInvProj);
    //vout.PosV = ph.xyz / ph.w;

    return vout;
}

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    vin.PosL.x = vin.PosL.x;
    vin.PosL.y = vin.PosL.y;
    vout.TexC = vin.TexC;
	// Quad covering screen in NDC space.
    vout.PosH = float4(2.0f * vout.TexC.x - 1.0f, 1.0f - 2.0f * vout.TexC.y, 0.0f, 1.0f);
    
    return vout;
}