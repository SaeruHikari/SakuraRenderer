#include "BRDF.hlsl"
#include "PBRCommon.hlsl"

float3 ToLinearSpace(float3 x)
{
    return pow(x, 2.2);
}

float3 AmbientBRDF(float3 IBLColor, float3 V, float3 N, Material mat)
{
    float3 Cdlin = ToLinearSpace(mat.BaseColor);
    float3 F0 = lerp(Cdlin, float3(0.04, 0.04, 0.04), 1 - mat.Metallic);
    float3 KS = F_SchlikRoughnessWithNoise(dot(N, V), F0, mat.Roughness);
    float3 KD = 1.f - KS;
    KD = KD * (1.f - mat.Metallic);

    return KD * IBLColor * Cdlin;
}

// X/Y : Anisotropic parameters.
// X would be tangent vector / cross(N, L)
// Y would be bi-normal vector / cross(N, V)
float3 DisneyBRDF(float3 L, float3 V, float3 N, float3 X, float3 Y, Material mat)
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
    float3 Cspec0 = lerp(mat.SpecularStrength * 0.08 * lerp(float3(1, 1, 1), Ctint, mat.SpecularColor), Cdlin, mat.Metallic);
    float3 CSheen = lerp(float3(1, 1, 1), Ctint, mat.SheenTint);

    // Diffuse fresnel - go from 1 at normal incidence to .5 at grazing
    // and mix in diffuse retro-reflection based on roughness.
    float FL = F_Schlik(NoL);
    float FV = F_Schlik(NoV);
    float Fd90 = 0.5 + 2 * LoH * LoH * mat.Roughness;
    float Fd = lerp(1.0, Fd90, FL) * lerp(1.0, Fd90, FV);

	// Based on Hanrahan-Krueger brdf approximation of isotropic bssrdf
    // 1.25 scale is used to (roughly) preserve albedo
    // Fss90 used to "flatten" retroreflection based on roughness
    float Fss90 = LoH * LoH * mat.Roughness;
    float Fss = lerp(1.0, Fss90, FL) * lerp(1.0, Fss90, FV);
    float ss = 1.25 * (Fss * (1 / (NoL + NoV) - 0.5) + 0.5);

    // Specular.
    float aspect = sqrt(1 - mat.Anisotropic * 0.9);
    float a = pow(((mat.Roughness + 1)/2), 2);
    float ax = max(0.001, a / aspect);
    float ay = max(0.001, a * aspect);
    float Ds;

    // Ds : have an aniso version.
#if defined(SAKURA_ENABLE_ANISO)
    Ds = D_GTR2_aniso(dot(H, X), dot(H, Y), NoH, ax, ay);
#else
    Ds = D_GTR2(a, NoH);
#endif

    float FH = F_Schlik(LoH);
    float3 Fs = lerp(Cspec0, float3(1, 1, 1), FH);
    float Gs;
    // Gs: have an aniso version.
#if defined(SAKURA_ENABLE_ANISO)
    Gs = G_Smith_GGX_aniso(ax, ay, NoL, NoV, dot(L, X), dot(L, Y), dot(V, X), dot(V, Y));
#else
    Gs = G_Smith_GGX(a*a, NoV, NoL);
#endif

    // sheen.
    float3 Fsheen = FH * mat.Sheen * CSheen;

    // Clearcoat (ior = 1.5 -> F0 = 0.04)
    float Dr = D_GTR1(lerp(0.1, 0.001, mat.ClearcoarGloss), NoH);
    float Fr = lerp(0.04, 1.0, FH);
    float Gr = G_Smith_GGX(0.25, NoV, NoL);

    // Diff
    float3 Brdf = ((1/PI) * lerp(Fd, ss, mat.Subsurface) * Cdlin + Fsheen)
             * (1 - mat.Metallic);
    // Spec
    Brdf += 
    (Gs * Fs * Ds + mat.Clearcoat * Gr * Fr * Dr)/(4 * max(NoV * NoL, 0.01f));
    
    //return Gs * Fs * Ds * max(0, NoL);

    return Brdf * max(0, NoL);
}