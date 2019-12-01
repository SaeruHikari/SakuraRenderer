// Copyright SaeruHikari 2019, PBR Common Utils.
#ifndef __PASS_COMMON__
#define __PASS_COMMON__

// Defaults for number of lights.
#ifndef MaxLights
    #define MaxLights 16
#endif

#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

struct Light
{
	float3 Strength;
	float FalloffStart; // point/spot light only
	float3 Direction;   // directional/spot light only
	float FalloffEnd;   // point/spot light only
	float3 Position;    // point light only
	float SpotPower;    // spot light only
};

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

    float4   gRandSeed;
    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light    gLights[MaxLights];
};

cbuffer cbMaterial : register(b2)
{
    	// Basic PBR material attribute sets.
	//
	float3	gBaseColor;
	float	gOpacity;
	//
	float	gMetallic;
	float	gRoughness;
	float   gSpecularTint;
	float   gSpecularStrength;

	float3	gSpecularColor;
	float 	gAnisotropic; 
	
	float 	gSubsurface;
	float3  gSubsurfaceColor;

	float   gClearcoat;
	float	gClearcoarGloss;
	float   gSheen;
	float 	gSheenTint;

    float4x4 gMatTransform;

    uint gDiffuseSrvHeapIndex;
    uint gRMOSrvHeapIndex;
    uint gSpecularSrvHeapIndex;
    uint gNormalSrvHeapIndex;
};
#endif