#ifndef _MATERIAL_CB_
#define _MATERIAL_CB_

cbuffer cbMaterial : register(b2)
{
    	// Basic PBR material attribute sets.
	//
    float3 gBaseColor;
    float gOpacity;
	//
    float gMetallic;
    float gRoughness;
    float gSpecularTint;
    float gSpecularStrength;

    float3 gSpecularColor;
    float gAnisotropic;
	
    float gSubsurface;
    float3 gSubsurfaceColor;

    float gClearcoat;
    float gClearcoarGloss;
    float gSheen;
    float gSheenTint;

    float4x4 gMatTransform;

    uint gDiffuseSrvHeapIndex;
    uint gRMOSrvHeapIndex;
    uint gSpecularSrvHeapIndex;
    uint gNormalSrvHeapIndex;
};

#endif