// Copyright SaeruHikari 2019, basic BRDF functions.
// Include popular D(h), F(l, h) and G(l, v, h) terms.

// a = pow2((roughness + 1) / 2) 
// Unreal 4 take it as a = roughness * roughness.
#include "Utils.hlsl"
#define PI 3.14159265358979323846

float sqr(float x) { return x * x;}

float Pow5(float src)
{
	float res = src * src;
	return res * res * src;
}

// Diffuse

// Disney diffuse: Disney 2012
// [Burley 2012, "Physically-Based Shading at Disney"]
float3 Diffuse_Burley_Disney( float3 DiffuseColor, float Roughness, float NoV, float NoL, float VoH )
{
    Roughness = Roughness * Roughness;
    Roughness = Roughness * Roughness;
	float FD90 = 0.5 + 2 * VoH * VoH * Roughness;
	float FdV = 1 + (FD90 - 1) * Pow5( 1 - NoV );
	float FdL = 1 + (FD90 - 1) * Pow5( 1 - NoL );
	
	return DiffuseColor * ( (1 / PI) * FdV * FdL );
}


// Specular D (D(h))
// Normal Distribution Function.

// D_Blinn_Phong based on halfway-vector.
float D_BlinnPhong(float n, float3 H, float NoH)
{
    float val = pow(max(0, NoH), n);
    val *= (2+n) / (2*PI);
    return val;
}

// GGX: [Walter et al. 2007, "Microfacet models for refraction through rough surfaces"]
float D_GGX(float a2, float3 H, float NoH)
{
    float d = (NoH * a2 - NoH) * NoH + 1; // 2 mad
    return a2 / (PI * d * d);
}

// Anisotropic GGX: [Burley 2012, "Physically-Based Shading at Disney"]
float D_GGXaniso(float anisotropic, float roughness, float NoH, float HoX, float HoY)
{
    float aspect = sqrt(1.0 - 0.9 * anisotropic);
	float roughnessSqr = roughness * roughness;
	float NdotHSqr = NoH * NoH;
	float ax = roughnessSqr / aspect;
	float ay = roughnessSqr * aspect;
	float d = HoX * HoX / (ax * ax) + HoY * HoY / (ay * ay) + NdotHSqr;
	return 1 / (PI * ax * ay * d * d);
}

// GTR: Generalized-Trowbridge-Reitz distribution
float D_GTR1(float alpha, float NoH)
{
    if(alpha >= 1) return 1/PI;
    float a2 = alpha * alpha;
    float cos2th = NoH * NoH;
    float t = (1.0 + (a2 - 1.0) * cos2th);

    return (a2 - 1.0) / (PI * log(a2) * t);
}

// GGX: GTR with γ = 2
float D_GTR2(float a2, float NoH)
{
    float t = 1.0 + (a2 - 1.0) * NoH * NoH;
    return a2 / (PI * t * t);
}

// Anisotropic GTR: [Burley 2012, "Physically-Based Shading at Disney"]
// X would be tangent vector / cross(N, L)
// Y would be bi-normal vector / cross(N, V)
float D_GTR2_aniso(float HoX, float HoY, float NoH, float ax, float ay)
{
    float deno = HoX * HoX / (ax * ax) + HoY * HoY / (ay * ay) + NoH * NoH;
    return 1.0 / (PI * ax * ay * deno * deno);
}



// Specular F (F(l, h))

// Schlik Fesnel: [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
float3 F_Schlik(float VoH)
{
    float Fc = Pow5(saturate(1-VoH));
    return Fc;		// 1 add, 3 mad
    
    // Anything less than 2% is physically impossible and is instead considered to be shadowing
	//return saturate( 50.0 * SpecularColor.g ) * Fc + (1 - Fc) * SpecularColor;
}

// Schlik Fesnel: [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
// This is the unreal4 version which add 2％ noise to the inner area. 
float3 F_SchlikWithNoise(float VoH, float3 SpecularColor)
{
    float Fc = Pow5(1-VoH);
    //return Fc;		// 1 add, 3 mad
    
    // Anything less than 2% is physically impossible and is instead considered to be shadowing
    return (1 - SpecularColor) * Fc + SpecularColor;
}

float3 F_Schlik(float3 F0, float F90, float u)
{
    return F0 + (F90 - F0) * Pow5(1.f - u);
}

//
//
float3 F_SchlikRoughnessWithNoise(float VoH, float3 SpecularColor, float Roughness)
{
    float Fc = Pow5(1 - VoH);
    //return Fc;		// 1 add, 3 mad
    
    // Anything less than 2% is physically impossible and is instead considered to be shadowing
    return (max(1.f - Roughness, SpecularColor) - SpecularColor) * Fc + SpecularColor;
}


// Specular G(G(l, v, h))

// Separable Masking and Shadowing.
// Smith-GGX    Smith-Beckman  Smith-Schlick   Schlick-Beckman  Schlick-GGX

// Smith-GGX: [Smith 1967, "Geometrical shadowing of a random rough surface"]
float G_Smith_GGX( float a2, float NoV, float NoL )
{
    float G_SmV = NoV * sqrt((-NoV * a2 + NoV) * NoV + a2);
    float G_SmL = NoL * sqrt((-NoL * a2 + NoL) * NoL + a2);
    return (0.5 / (G_SmV + G_SmL));
    
	float G_SmithV = NoV + sqrt( NoV * (NoV - NoV * a2) + a2 );
	float G_SmithL = NoL + sqrt( NoL * (NoL - NoL * a2) + a2 );
	return rcp( G_SmithV * G_SmithL );
}

float G_Smith_GGX_aniso(float ax, float ay, float NoL, float NoV, float LoX, float LoY, float VoX, float VoY)
{
    float G_SmithV = (NoV + sqrt( sqr(LoX*ax) + sqr(LoY*ay) + sqr(NoV) ));
    float G_SmithL = (NoL + sqrt( sqr(VoX*ax) + sqr(VoY*ay) + sqr(NoL) ));
    return rcp(G_SmithV * G_SmithL);
}