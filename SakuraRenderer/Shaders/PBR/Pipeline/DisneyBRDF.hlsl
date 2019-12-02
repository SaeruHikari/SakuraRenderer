#include "BRDF.hlsl"
#include "PBRCommon.hlsl"

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

float4 PreintegratedDGF_LUT(float3 Enviorfilter_GFD, float3 SpecularColor)
{
    // r + ( 1 - c ) * b
    float3 ReflectionGF = lerp(saturate(50 * SpecularColor.g) * Enviorfilter_GFD.ggg, Enviorfilter_GFD.rrr, SpecularColor);
    return float4(ReflectionGF, Enviorfilter_GFD.b);
}

float4 AmbientBRDF_MultiScattering(float3 ConvColor, float3 PrefilteredColor, float4 BRDFLUT, float3 V, float3 N, Material mat)
{
    float3 Cdlin = ToLinearSpace(mat.BaseColor);
    float3 F0 = lerp(Cdlin, float3(0.04, 0.04, 0.04), 1 - mat.Metallic);
    float3 KS = F_SchlikRoughnessWithNoise(dot(N, V), F0, mat.Roughness);
    float3 KD = 1.f - KS;
    KD = KD * (1.f - mat.Metallic);
    
    BRDFLUT = PreintegratedDGF_LUT(BRDFLUT.xyz, F0);
    float3 diffuse = ConvColor * Cdlin;
    float3 spec = PrefilteredColor * BRDFLUT.xyz;
    spec = 0;
    return float4(KD * diffuse * BRDFLUT.a + spec, BRDFLUT.a);
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

// X/Y : Anisotropic parameters.
// X would be tangent vector / cross(N, L)
// Y would be bi-normal vector / cross(N, V)
float3 DisneyBRDF(float3 L, float3 V, float3 N, Material mat)
{
    float NoL = max(dot(N, L), 0.0001);
    float NoV = max(dot(N, V), 0.0001);
    if(NoL < 0 || NoV < 0) return float3(0, 0, 0);
    float3 H = normalize(L + V);
    float NoH = max(dot(N, H), 0.0001);
    float LoH = max(dot(L, H), 0.0001);

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
    float Fd = MultiScatteringDiffuseBRDF(LoH, NoL, NoV, NoH, mat.Roughness);

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
    float Gs = G_Smith_GGX(a * a, NoV, NoL);

    // sheen.
    float3 Fsheen = FH * mat.Sheen * CSheen;

    // Clearcoat (ior = 1.5 -> F0 = 0.04)  DFG
    float Dr = D_GTR1(lerp(0.1, 0.001, mat.ClearcoarGloss), NoH);
    float Fr = lerp(0.04, 1.0, FH);
    float Gr = G_Smith_GGX(0.25, NoV, NoL);

    // Diff
    float3 Brdf = ((1 / PI) * (Fd * Cdlin + Fsheen) * (1 - mat.Metallic));

    // Spec
    Brdf += 
    (Gs * Fs * Ds + mat.Clearcoat * Gr * Fr * Dr)/(4 * max(NoV * NoL, 0.01f));
    
    //return (MultiScatteringDiffuseBRDF(LoH, NoL, NoV, NoH, mat.Roughness) ) * max(0, NoL);
    //return Gs * Fs * Ds * max(0, NoL);
    return Brdf * max(0, NoL);
}