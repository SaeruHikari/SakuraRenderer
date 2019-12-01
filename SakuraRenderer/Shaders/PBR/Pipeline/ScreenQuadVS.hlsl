#include "PassCommon.hlsl"
#include "ScreenQuadVertex.hlsl"

VertexOut VS_Portable(VertexIn vin)
{
    VertexOut vout;
    vin.PosL.x = vin.PosL.x;
    vin.PosL.y = vin.PosL.y;
    vout.PosH = float4(vin.PosL, 1.0f);

    vout.TexC = vin.TexC;

    // Transform quad corners to view space near plane.
    float4 ph = mul(vout.PosH, gInvProj);
    vout.PosV = ph.xyz / ph.w;

    return vout;
}

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    vin.PosL.x = vin.PosL.x * 2 -1;
    vin.PosL.y = vin.PosL.y * 2 + 1;
    vout.PosH = float4(vin.PosL, 1.0f);

    vout.TexC = vin.TexC;

    // Transform quad corners to view space near plane.
    float4 ph = mul(vout.PosH, gInvProj);
    vout.PosV = ph.xyz / ph.w;

    return vout;
}