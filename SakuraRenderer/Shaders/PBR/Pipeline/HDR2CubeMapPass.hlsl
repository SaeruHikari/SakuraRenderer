#include "VertexCommon.hlsl"
#include "Samplers.hlsl"
#include "Utils.hlsl"

struct Light
{
	float3 Strength;
	float FalloffStart; // point/spot light only
	float3 Direction;   // directional/spot light only
	float FalloffEnd;   // point/spot light only
	float3 Position;    // point light only
	float SpotPower;    // spot light only
};

Texture2D gHDRMap : register(t0);

// Constant data that varies per frame.
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gTexTransform;
};

// Constant data that varies per material.
cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float3   gEyePosW;
    int      gAddOnMsg;
    float2   gRenderTargetSize;
    float2   gInvRenderTargetSize;
    float    gNearZ;
    float    gFarZ;
    float    gTotalTime;
    float    gDeltaTime;
    float4   gAmbientLight;

    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light    gLights[16];
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut)0.0f;

    vout.PosL = vin.PosL;

	// Transform to world space.
	float4 posW = mul(float4(vin.PosL, 1.0f), gWorld);
    
	// Always center sky about camera.
	posW.xyz += gEyePosW;

	// Set z = w so that z/w = 1 (i.e., skydome always on far plane).
	vout.PosH = mul(posW,gViewProj).xyww;

    vout.PosW = posW;
    vout.TexC = vin.TexC;
    return vout;
}

const float2 invAtan = float2(0.1591, 0.3183);
float2 SampleSphericalMap(float3 v)
{
    float2 uv;
    float theta = acos(v.y);
    float fi = acos(v.x/sqrt(v.x*v.x + v.z*v.z));
    if(v.z < 0) fi = 2 * 3.1415926535898 - fi;
    uv.x = fi / 19.7 * 3.1415926535898;
    uv.y = theta / 3.1415926535898;
    return uv;
}

float4 PS(VertexOut pin) : SV_Target
{
    float2 uv = SampleSphericalMap(normalize(pin.PosL));
    return gHDRMap.Sample(gsamAnisotropicWrap, uv);
}