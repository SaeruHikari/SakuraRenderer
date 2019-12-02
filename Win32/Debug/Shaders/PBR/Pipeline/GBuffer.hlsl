// Saeru Hikari
#include "PBRCommon.hlsl"
#include "PassCommon.hlsl"
#include "VertexCommon.hlsl"
#include "GBufferRegisters.hlsl"
#include "Samplers.hlsl"

struct GBuffer
{
    float4 Albedo : ALBEDO;
    float4 Normal : WNORMAL;
    float4 WPos : WPOS;
    float4 RoughnessMetallicOcclusion : RMO;
};

GBuffer PS(VertexOut pin) : SV_Target
{
    pin.TexC = float2(pin.TexC.x, 1-pin.TexC.y);
    GBuffer gBuffer;
	
    float3 normalFromTex =
     2 * (gTexutres[gNormalSrvHeapIndex].Sample(gsamAnisotropicWrap, pin.TexC).rgb - 0.5);
    float4 RMO = gTexutres[gRMOSrvHeapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
    
    // Construct TBN
    float3x3 TBN = float3x3(pin.Tangent, pin.BiNormal, pin.NormalW);
    float3 N = mul(normalFromTex, TBN);
    
    gBuffer.Albedo =   
        gTexutres[gDiffuseSrvHeapIndex].Sample(gsamAnisotropicWrap, pin.TexC) 
            * float4(gBaseColor, gOpacity);
    
    gBuffer.Normal = float4(N, 1.f);
    gBuffer.WPos = float4(pin.PosW, 0.f);
    
     float Metallic =
        gMetallic *  RMO.g;
    float Roughness = 
        gRoughness * RMO.r;
    float Occlusion = 
        gRoughness * RMO.b;
    gBuffer.RoughnessMetallicOcclusion
        = float4(Roughness, Metallic, Occlusion, 1.f);

    return gBuffer;
}