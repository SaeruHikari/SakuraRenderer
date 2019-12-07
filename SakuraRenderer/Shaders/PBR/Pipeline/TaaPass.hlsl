#include "ScreenQuadVertex.hlsl"
#include "Utils.hlsl"
#include "Samplers.hlsl"
#include "PBRCommon.hlsl"
#include "PassCommon.hlsl"

cbuffer cbPass : register(b0)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gInvViewProj;
    float3 gEyePosW;
    uint gAddOnMsg;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;

    float4 gAmbientLight;
    float2 gJitter;
    float2 _PassPad_F2;
    // Indices [0, NUM_DIR_LIGHTS) are directional lights;
    // indices [NUM_DIR_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHTS) are point lights;
    // indices [NUM_DIR_LIGHTS+NUM_POINT_LIGHTS, NUM_DIR_LIGHTS+NUM_POINT_LIGHT+NUM_SPOT_LIGHTS)
    // are spot lights for a maximum of MaxLights per object.
    Light gLights[MaxLights];
};

// 1 2
Texture2D gHistoryTexture : register(t0);
// 0
Texture2D gInputTexture : register(t1);
// Velocity
Texture2D gVelocityBuffer : register(t2);
Texture2D gDepthBuffer : register(t3);

float LinearDepth(float depth)
{
    return (depth * gNearZ) / (gFarZ - depth * (gFarZ - gNearZ));
}

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
    vin.PosL.x = vin.PosL.x;
    vin.PosL.y = vin.PosL.y;
    vout.TexC = vin.TexC;
	// Quad covering screen in NDC space.
    vout.PosH = float4(2.0f * vout.TexC.x - 1.0f, 1.0f - 2.0f * vout.TexC.y, 0.0f, 1.0f);

    return vout;
}


float3 ClipAABB(float3 aabbMin, float3 aabbMax, float3 prevSample, float3 avg)
{
    float3 r = prevSample - avg;
    float3 rmax = aabbMax - avg.xyz;
    float3 rmin = aabbMin - avg.xyz;

    const float eps = 0.000001f;

    if (r.x > rmax.x + eps)
        r *= (rmax.x / r.x);
    if (r.y > rmax.y + eps)
        r *= (rmax.y / r.y);
    if (r.z > rmax.z + eps)
        r *= (rmax.z / r.z);

    if (r.x < rmin.x - eps)
        r *= (rmin.x / r.x);
    if (r.y < rmin.y - eps)
        r *= (rmin.y / r.y);
    if (r.z < rmin.z - eps)
        r *= (rmin.z / r.z);

    return avg + r;
}

float Luma4(float3 Color)
{
    return Color.r;
}

// Optimized HDR weighting function.
float HdrWeight4(float3 Color, float Exposure)
{
    return rcp(Luma4(Color) * Exposure + 4.0);
}

static const float VarianceClipGamma = 1.0f;
static const float Exposure = 10;
static const float BlendWeightLowerBound = 0.03f;
static const float BlendWeightUpperBound = 0.12f;
static const float BlendWeightVelocityScale = 100.0f * 60.0f;

float4 PS(VertexOut pin) : SV_Target
{
    int x, y, i;
    float2 velocity;
    float lenVelocity;
    double2 JitteredUV = (double2)pin.TexC + gJitter.xy;
    float2 closestOffset = float2(0.0f, 0.0f);
    float closestDepth = gFarZ;
    for (y = -1; y <= 1; ++y)
    {
        for (x = -1; x <= 1; ++x)
        {
            float2 sampleOffset = float2(x, y) * gInvRenderTargetSize;
            float2 sampleUV = JitteredUV + sampleOffset;
            sampleUV = saturate(sampleUV);

            float NeighborhoodDepthSamp = gDepthBuffer.Sample(gsamLinearClamp, sampleUV).r;
            NeighborhoodDepthSamp = LinearDepth(NeighborhoodDepthSamp);
			if (NeighborhoodDepthSamp > closestDepth)
            {
                closestDepth = NeighborhoodDepthSamp;
                closestOffset = float2(x, y);
            }
        }
    }
    closestOffset *= gInvRenderTargetSize;
    velocity = gVelocityBuffer.Sample(gsamLinearClamp, JitteredUV + closestOffset).rg;
    lenVelocity = length(velocity);
    
    float2 InputSampleUV = lerp(pin.TexC, JitteredUV, 0);
    
    float3 currColor = gInputTexture.Sample(gsamLinearClamp, InputSampleUV).rgb;
    currColor = RGB2YCoCgR(currColor);
    
    float3 prevColor = gHistoryTexture.Sample(gsamLinearClamp, pin.TexC - velocity).rgb;
    prevColor = RGB2YCoCgR(prevColor);

    float3 History;
    uint N = 9;
    float TotalWeight = 0.0f;
    float3 sum = 0.0f;
    float3 m1 = 0.0f;
    float3 m2 = 0.0f;
    float3 neighborMin = float3(9999999.0f, 9999999.0f, 9999999.0f);
    float3 neighborMax = float3(-99999999.0f, -99999999.0f, -99999999.0f);
    float neighborhoodFinalWeight = 0.0f;
    // 3x3 Sampling
    for (y = -1; y <= 1; ++y)
    {
        for (x = -1; x <= 1; ++x)
        {
            i = (y + 1) * 3 + x + 1;
            float2 sampleOffset = float2(x, y) * gInvRenderTargetSize;
            float2 sampleUV = InputSampleUV + sampleOffset;
            sampleUV = saturate(sampleUV);

            float3 NeighborhoodSamp = gInputTexture.Sample(gsamLinearClamp, sampleUV).rgb;
            NeighborhoodSamp = max(NeighborhoodSamp, 0.0f);
			NeighborhoodSamp = RGB2YCoCgR(NeighborhoodSamp);

            neighborMin = min(neighborMin, NeighborhoodSamp);
            neighborMax = max(neighborMax, NeighborhoodSamp);
            neighborhoodFinalWeight = HdrWeight4(NeighborhoodSamp, Exposure);
            m1 += NeighborhoodSamp;
            m2 += NeighborhoodSamp * NeighborhoodSamp;
            TotalWeight += neighborhoodFinalWeight;
            sum += NeighborhoodSamp * neighborhoodFinalWeight;
        }
    }
    float3 Filtered = sum / TotalWeight;
    
    // Variance clip.
    float3 mu = m1 / N;
    float3 sigma = sqrt(abs(m2 / N - mu * mu));
    float3 minc = mu - VarianceClipGamma * sigma;
    float3 maxc = mu + VarianceClipGamma * sigma;

    prevColor = ClipAABB(minc, maxc, prevColor, mu);

    float weightCurr = lerp(BlendWeightLowerBound, BlendWeightUpperBound, saturate(lenVelocity * BlendWeightVelocityScale));
    float weightPrev = 1.0f - weightCurr;

    float RcpWeight = rcp(weightCurr + weightPrev);
    History = (currColor * weightCurr + prevColor * weightPrev) * RcpWeight;
    History.xyz = YCoCgR2RGB(History.xyz);
    return float4(History, 1.f);
}
