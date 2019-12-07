#include "PassCommon.hlsl"
#include "VertexCommon.hlsl"
#include "CommonCBs.hlsl"

VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut)0.0f;

    // Tranform to world space.
    float4 posW = mul(float4(vin.PosL, 1.f), gWorld);
    vout.PosW = posW.xyz;
#if defined(TAA_ENABLED)
    float4 curWorldPos = mul(float4(vin.PosL, 1.f), gWorld);
    float4 prevWorldPos = mul(float4(vin.PosL, 1.f), gPrevWorld);
    vout.CurPosVP = mul(curWorldPos, gUnjitteredViewProj);
    vout.PrevPosVP = mul(prevWorldPos, gPrevViewProj);
#endif
    // Assumes nonumiform scaling; otherwise, need to use inverse-transpose
    // of world matrix.
    vout.NormalW = normalize(mul(vin.NormalL, (float3x3)gWorld));

    // Transform to homogeneous clip space.
    vout.PosH = mul(posW, gViewProj);

    // Output vertex attributes for interpolation across triangle.
    float4 texC = mul(float4(vin.TexC, 0.0f, 1.0f), gTexTransform);
	vout.TexC = texC.xy;
    
    vout.Tangent = normalize(mul(vin.Tangent, (float3x3)gWorld));
    vout.BiNormal = cross(vout.NormalW, vout.Tangent);

    return vout;
}