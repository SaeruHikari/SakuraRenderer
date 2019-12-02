// Saeru Hikari

//#define SAKURA_ENABLE_ANISO

#include "ScreenDebugQuadVertex.hlsl"

#include "DeferredRegisters.hlsl"
#include "PassCommon.hlsl"

//Include structures and functions for lighting
#include "DisneyBRDF.hlsl"
#include "Samplers.hlsl"
#include "ShadingUtils.hlsl"

VertexOut VS(VertexIn vin) 
{
    VertexOut vout;

    vin.PosL.x = (vin.PosL.x * 2 - 1)/6 - 0.83 + 0.33333*(vin.Type%6);
    vin.PosL.y = (vin.PosL.y * 2 + 1)/6 - 0.83 + 0.33333*(vin.Type/6);

    vout.PosH = float4(vin.PosL, 1.0f);

    vout.TexC = vin.TexC;
    vout.Type = vin.Type;

    return vout;
}

float4 PS_GBufferDebugging(VertexOut pin) : SV_Target
{
    float4 Albedo = gGeometryAlbedo.Sample(gsamLinearWrap, pin.TexC);
    float4 WNormal = gGeometryNormal.Sample(gsamLinearWrap, pin.TexC);
    float3 WPos = gGeometryWPos.Sample(gsamLinearWrap, pin.TexC).rgb;
    float4 RMO = gGeometryRMO.Sample(gsamLinearWrap, pin.TexC);
    float4 res = float4(0, 0, 0, 0);
    if(pin.Type == 0) res = Albedo;
    if(pin.Type == 1) res = WNormal;
    if(pin.Type == 2) res = float4(RMO.rrrr);
    if(pin.Type == 3) res = float4(RMO.gggg);
    if(pin.Type == 4) res = float4(RMO.bbbb); 
    //return float4(pin.TexC.rg, 0.f, 1.f);
    return res;
}
