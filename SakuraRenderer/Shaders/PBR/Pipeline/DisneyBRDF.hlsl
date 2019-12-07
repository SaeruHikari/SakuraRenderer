#include "BRDF.hlsl"
#include "PBRCommon.hlsl"
#include "Utils.hlsl"

float3 ToLinearSpace(float3 x)
{
    // Default linear tex now.
    return x;
}

float3 AmbientBRDF(float3 ConvColor, float3 PrefilteredColor, float3 BRDFLUT, float3 V, float3 N, Material mat)
{
    float3 Cdlin = ToLinearSpace(mat.BaseColor);
    float3 F0 = lerp(Cdlin, float3(0.04, 0.04, 0.04), 1 - mat.Metallic);
    float3 KS = F_SchlikRoughnessWithNoise(dot(N, V), F0, mat.Roughness);
    float3 KD = 1.f - KS;
    KD = KD * (1.f - mat.Metallic);

    float3 diffuse = ConvColor * Cdlin;
    float3 spec = PrefilteredColor * (BRDFLUT.x * KD + BRDFLUT.y);
    
    return KD * diffuse + spec;
}

float3 Preintegrated_LUT(float2 Enviorfilter_GFD, float3 SpecularColor)
{
    // r + ( 1 - c ) * b
    return Enviorfilter_GFD.x * SpecularColor + Enviorfilter_GFD.y;
}

float4 PreintegratedDGF_LUT(float3 Enviorfilter_GFD, inout float3 EnergyCompensation, float3 SpecularColor)
{
    float3 ReflectionGF = lerp(saturate(50.f * SpecularColor.g) * Enviorfilter_GFD.ggg, Enviorfilter_GFD.rrr, SpecularColor);
    EnergyCompensation = 1.f + SpecularColor * (1.f / Enviorfilter_GFD.r - 1.f);
    return float4(ReflectionGF, Enviorfilter_GFD.b);
}

float3 Fdez_Favg(float3 F0)
{
    return F0 + 1.f / 21.f * (float3(1.f, 1.f, 1.f) - F0);
}

float4 AmbientBRDF_MultiScattering(float3 ConvColor, float3 PrefilteredColor, inout float4 BRDFLUT, float3 V, float3 N, float3 L, Material mat)
{
    BRDFLUT.a = BRDFLUT.b;
    float3 Cdlin = ToLinearSpace(mat.BaseColor);
    float3 F0 = lerp(Cdlin, float3(0.04, 0.04, 0.04), 1 - mat.Metallic);
    float3 KS = F_SchlikRoughnessWithNoise(dot(N, V), F0, max(mat.Roughness, 0.04));
    float3 KD = 1.f - KS;
    KD = KD * (1.f - mat.Metallic);
    
    float3 diffuse = ConvColor * Cdlin;
#if defined(Filament_MS)
    return float4(PreintegratedDGF_LUT(BRDFLUT.rgb, BRDFLUT.rgb, F0).xyz * PrefilteredColor + KD * diffuse * BRDFLUT.a, 1.f);
#else
    float3 specSS = Preintegrated_LUT(BRDFLUT.xy, F0);
    float Ess = BRDFLUT.x + BRDFLUT.y;
    float Ems = 1.0f - Ess;
    float3 Favg = Fdez_Favg(F0);
    float Fms = specSS * Favg / (1.0f - Favg * (1.0f - Ess));
    float3 Lss = specSS;
    float3 Lms = Fms * Ems;
    float3 SpecMs = Lss + Lms;
    return float4(KD * diffuse + SpecMs * PrefilteredColor, SpecMs.x);
#endif
    return float4(0.f, 0.f, 0.f, 0.f);
}

float MultiScatteringDiffuseBRDF(float LoH, float NoL, float NoV, float NoH, float Roughness)
{
    float g = saturate(0.18455f * log(2.f / pow(Roughness, 4.f) - 1.f));
    
    float f0 = LoH + Pow5(1.f - LoH);
    float f1 = (1.f - 0.75f * Pow5(1.f - NoL))
            * (1.f - 0.75f * Pow5(1.f - NoV));
    float t = saturate(2.2f * g - 0.5f);
    float fd = f0 + (f1 - f0) * t;
    float fb = ((34.5f * g - 59.f) * g + 24.5f) * LoH
        * exp2(-max(73.2f * g - 21.2f, 8.9f) * sqrt(NoH));
    return max(fd + fb, 0.f);
}

float Diffuse_RenormalizeBurley(float LoH, float NoL, float NoV, float Roughness)
{
    Roughness = Roughness * Roughness;
    Roughness = Roughness * Roughness;
    float EnergyBias = lerp(0, 0.5, Roughness);
    float EnergyFactor = lerp(1, 1 / 0.662, Roughness);
    float F90 = EnergyBias + 2 * pow(LoH, 2) * Roughness;
    float LightScatter = F_Schlik(1, F90, NoL);
    float ViewScatter = F_Schlik(1, F90, NoV);
    return LightScatter * ViewScatter * EnergyFactor;
}

// X/Y : Anisotropic parameters.
// X would be tangent vector / cross(N, L)
// Y would be bi-normal vector / cross(N, V)
float3 DisneyBRDF_MultiScattering(float3 L, float3 V, float3 N, Material mat, float3 MultiScattering)
{
    float NoL = dot(N, L);
    float NoV = dot(N, V);
    if(NoL < 0 || NoV < 0) return float3(0, 0, 0);
    float3 H = normalize(L + V);
    float NoH = dot(N, H);
    float LoH = dot(L, H);

    float3 Cdlin = ToLinearSpace(mat.BaseColor);
    float Cdlum = .3*Cdlin.r + .6*Cdlin.g  + .1*Cdlin.b; // luminance approx.

    float3 Ctint =  Cdlum > 0 ? Cdlin/Cdlum : float3(1, 1, 1); // normalize lum. to isolate hue+sat
    float3 F0 = lerp(mat.SpecularStrength * 0.08 * lerp(float3(1, 1, 1), Ctint, mat.SpecularColor), Cdlin, mat.Metallic);
    float3 CSheen = lerp(float3(1, 1, 1), Ctint, mat.SheenTint);

    // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
    // and mix in diffuse retro-reflection based on roughness.
    
    //: Disney Diffuse
    //float FL = F_Schlik(NoL);
    //float FV = F_Schlik(NoV);
    //float Fd90 = 0.5 + 2 * LoH * LoH * mat.Roughness;
    //float Fd = lerp(1.0, Fd90, FL) * lerp(1.0, Fd90, FV);
    
    // : MultiScattrering Diffuse
    //float Fd = MultiScatteringDiffuseBRDF(LoH, NoL, NoV, NoH, mat.Roughness);
    
    // : Renormalized Burley, Frostbite.
    float Fd = Diffuse_RenormalizeBurley(LoH, NoL, NoV, mat.Roughness);
    mat.Roughness = max(0.04, mat.Roughness);
    // Specular.
    //float a = pow(((mat.Roughness + 1)/2), 2);
    // Frostbite Solution
    float a = mat.Roughness * mat.Roughness;
    a = a * a;
    
    //D
    float Ds = D_GTR2(a, NoH);
    //F
    float FH = F_Schlik(LoH);
    float3 Fs = lerp(F0, float3(1, 1, 1), FH);
    //G
    float Gs = G_Smith_GGX(a, NoV, NoL);

    // sheen.
    float3 Fsheen = FH * mat.Sheen * CSheen;

    // Clearcoat (ior = 1.5 -> F0 = 0.04)  DFG
    float Dr = D_GTR1(lerp(0.1, 0.001, mat.ClearcoarGloss), NoH);
    float Fr = lerp(0.04, 1.0, FH);
    float Gr = G_Smith_GGX(0.25, NoV, NoL);

    // Diff
    float3 Brdf = ((1 / PI) * (Fd * Cdlin + Fsheen) * (1 - mat.Metallic));
    MultiScattering.xyz = float3(1, 1, 1);
    // Spec
    Brdf = 
    Brdf + (MultiScattering.xyz * (Gs * Fs * Ds + mat.Clearcoat * Gr * Fr * Dr));
    //return (MultiScatteringDiffuseBRDF(LoH, NoL, NoV, NoH, mat.Roughness) ) * max(0, NoL);
    //return Gs * Fs * Ds * max(0, NoL);
    return Brdf * max(NoL, 0.f);
}

float3 DisneyBRDF(float3 L, float3 V, float3 N, Material mat)
{
    return DisneyBRDF_MultiScattering(L, V, N, mat, float3(1.f, 1.f, 1.f));
}