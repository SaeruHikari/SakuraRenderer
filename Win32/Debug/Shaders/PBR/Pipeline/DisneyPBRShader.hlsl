// Saeru Hikari

//#define SAKURA_ENABLE_ANISO

// Forward shading registers
#include "GBufferRegisters.hlsl"
//Include structures and functions for lighting
#include "DisneyBRDF.hlsl"
#include "PassCommon.hlsl"
#include "VertexCommon.hlsl"
#include "Samplers.hlsl"
#include "ShadingUtils.hlsl"


float4 PS(VertexOut pin) : SV_Target
{
    pin.TexC = float2(pin.TexC.x, 1-pin.TexC.y);
    float4 RMO = gTexutres[gRMOSrvHeapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
    float4 Spec = gTexutres[gSpecularSrvHeapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
    float4 BaseColorMap = gTexutres[gDiffuseSrvHeapIndex].Sample(gsamAnisotropicWrap, pin.TexC);
    float3 NormalMap = 
    gTexutres[gNormalSrvHeapIndex].Sample(gsamAnisotropicWrap, pin.TexC).rgb;

    Material mat;
    mat.BaseColor = gBaseColor;
    mat.Opacity = gOpacity;
    mat.Metallic = gMetallic * RMO.g;
    mat.Roughness = gRoughness * RMO.r;
    mat.SpecularTint = gSpecularTint;
    mat.SpecularStrength = 0 * gSpecularStrength * Spec.g;
    mat.SpecularColor = gSpecularColor;
    mat.Anisotropic = gAnisotropic;
    mat.Subsurface = gSubsurface;
    mat.SubsurfaceColor = gSubsurfaceColor;
    mat.Clearcoat = gClearcoat;
    mat.ClearcoarGloss = gClearcoarGloss;
    mat.Sheen = gSheen;
    mat.SheenTint = gSheenTint;
   
    float4 baseColor =
        BaseColorMap * 
        float4(mat.BaseColor, mat.Opacity);
    mat.BaseColor = baseColor.rgb;

    float3 V = normalize(gEyePosW - pin.PosW);
    float3 N = normalize(pin.NormalW);
    float3 X = cross(pin.NormalW, gLights[0].Direction);
    float3 Y = cross(pin.NormalW, V);
    float3 L = -gLights[0].Direction;

    // Construct TBN
    float3x3 TBN = float3x3(pin.Tangent, pin.BiNormal, pin.NormalW);

    // Convert norm texture from tangent space to world space.
    float3 NormalColor = NormalMap;
    NormalColor = (NormalColor - 0.5) * 2;
    N = mul(NormalColor, TBN);

    // Recompute Bi-Normal for better details.
    float3 BiNormal = cross(pin.Tangent, N);

    float3 Disney = 0;
    for(int i = 0; i < 3; i++)
    {
        L = -gLights[i].Direction;
        Disney += 2.2 * PI * gLights[i].Strength  
         * DisneyBRDF(L, V, N, mat);
    }
       
    float4 ambient = float4(0.1f, 0.1f, 0.1f, 0.f);
    float4 FinalColor = float4(Disney.xyz, 1.f);
    
    //return BaseColorMap;
   
    return pow(FinalColor, 1/2.2);
}