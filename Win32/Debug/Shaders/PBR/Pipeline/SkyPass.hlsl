#include "VertexCommon.hlsl"
#include "Utils.hlsl"
#include "Samplers.hlsl"

struct Light
{
	float3 Strength;
	float FalloffStart; // point/spot light only
	float3 Direction;   // directional/spot light only
	float FalloffEnd;   // point/spot light only
	float3 Position;    // point light only
	float SpotPower;    // spot light only
};

TextureCube gCubeMap : register(t0);

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
	vout.PosH = mul(posW, gViewProj).xyww;

    return vout;
}

float RadicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

float2 Hammersley(uint i, uint N)
{
    return float2(float(i) / float(N), RadicalInverse_VdC(i));
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness;
    //float a = pow(((roughness + 1) / 2), 2);
    
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	
	// from spherical coordinates to cartesian coordinates - halfway vector
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
	
	// from tangent-space H vector to world-space sample vector
    float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
	
    float3 sampleVec = tangent * H.x + bitangent * H.y + N * H.z;
    return normalize(sampleVec);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    //float a = pow(((roughness + 1) / 2), 2);
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}


float4 PS(VertexOut pin) : SV_Target
{
    return gCubeMap.SampleLevel(gsamLinearWrap, pin.PosL, 1);
}

float4 PS_Conv(VertexOut pin): SV_Target
{
    if(gAddOnMsg == 0)
    {
        float3 N = normalize(pin.PosL);
        float3 irradiance = float3(0.f, 0.f, 0.f);
        float3 up = float3(0.f, 1.f, 0.f);
        float3 right = cross(up, N);
        up = cross(N, right);
  
        float sampleDelta = 0.025;
        float nrSamples = 0.0;

        for (float phi = 0.0; phi < 2 * PI; phi = phi + sampleDelta)
        {
            for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)
            {
            // spherical to cartesian (in tangent space)
                float3 tangentSample
            = float3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // tangent space to world
                float3 sampleVec =
             tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

                irradiance = gCubeMap.SampleLevel(gsamLinearWrap, sampleVec, 0).xyz *
             cos(theta) * sin(theta) + irradiance;
                nrSamples = nrSamples + 1;
            }
        }
        return float4(irradiance / nrSamples, 1.f);
    }
    else
    {
        float3 N = normalize(pin.PosL);
        float3 R = N;
        float3 V = R;
    
        uint sampleC = 512;
        float3 prefilteredC = float3(0.f, 0.f, 0.f);
        float totalWeight = 0;
    
        for (uint i = 0u; i < sampleC; ++i)
        {
            float roughness = (gAddOnMsg - 1) / 4;
            float2 Xi = Hammersley(i, sampleC);
            float3 H = ImportanceSampleGGX(Xi, N, roughness);
            float3 L = normalize(2.0 * dot(V, H) * H - V);
        
            float NoL = max(dot(N, L), 0.f);

            if (NoL > 0)
            {
            // sample from the environment's mip level based on roughness/pdf
                float D = DistributionGGX(N, H, roughness);
                float NdotH = max(dot(N, H), 0.0);
                float HdotV = max(dot(H, V), 0.0);
                float pdf = D * NdotH / (4.0 * HdotV) + 0.0001;

                float resolution = 512.0; // resolution of source cubemap (per face)
                float saTexel = 4.0 * PI / (6.0 * resolution * resolution);
                float saSample = 1.0 / (float(sampleC) * pdf + 0.0001);

                prefilteredC = prefilteredC + gCubeMap.SampleLevel(gsamLinearWrap, L, (gAddOnMsg - 1)).rgb * NoL;
                totalWeight = totalWeight + NoL;
            }
        }
        prefilteredC = prefilteredC / totalWeight;
        return float4(prefilteredC, 1.f);
    }
}