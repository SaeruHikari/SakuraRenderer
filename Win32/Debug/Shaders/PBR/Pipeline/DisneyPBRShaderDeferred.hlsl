// Saeru Hikari

//#define SAKURA_ENABLE_ANISO

#include "ScreenQuadVertex.hlsl"

#include "DeferredRegisters.hlsl"
#include "PassCommon.hlsl"

#include "Utils.hlsl"

//Include structures and functions for lighting
#include "DisneyBRDF.hlsl"
#include "Samplers.hlsl"
#include "ShadingUtils.hlsl"


float4 PS(VertexOut pin) : SV_Target
{
    float4 Albedo = gGeometryAlbedo.Sample(gsamLinearWrap, pin.TexC);
    float4 WNormal = gGeometryNormal.Sample(gsamLinearWrap, pin.TexC);
    float3 WPos = gGeometryWPos.Sample(gsamLinearWrap, pin.TexC).rgb;
    float4 RMO = gGeometryRMO.Sample(gsamLinearWrap, pin.TexC);
    Material mat;
    mat.BaseColor = Albedo.rgb;
    mat.Opacity = Albedo.a;
    mat.Metallic = RMO.g;
    mat.Roughness = RMO.r;

    //
    mat.SpecularTint = 0;
    mat.SpecularStrength = 0;
    mat.SpecularColor = 0;
    mat.Anisotropic = 0;
    mat.Subsurface = 0;
    mat.SubsurfaceColor = 0;
    mat.Clearcoat = 0;
    mat.ClearcoarGloss = 0;
    mat.Sheen = 0;
    mat.SheenTint = 0;
   
    float3 V = normalize(gEyePosW - WPos.rgb);
    float3 N = normalize(WNormal.rgb);
    float3 X = cross(N, gLights[0].Direction);
    float3 Y = cross(N, V);
    float3 L = -gLights[0].Direction;

    // Recompute Bi-Normal for better details.
    // !!!!!! no tangent info in GBuffer !!!!
    float3 BiNormal = N;
    
    float3 Disney = 0;
    for(int i = 0; i < 0; i++)
    {
        L = -gLights[i].Direction;
        Disney += PI * gLights[i].Strength  
         * DisneyBRDF(L, V, N, X, Y, mat);
    }

    float3 ambientC = WNormal.a * AmbientBRDF(gIBLCubeMap[0].SampleLevel(gsamLinearWrap, N, 0).xyz, V, N, mat);

    float3 FinalColor = Disney.xyz + ambientC.xyz;
    FinalColor = FinalColor / (FinalColor + float3(1.f, 1.f, 1.f));
    FinalColor = pow(FinalColor, 1/2.2);
    //return gIBLCubeMap[2].Sample(gsamLinearWrap, N);
    return float4(FinalColor, 1.f);
}