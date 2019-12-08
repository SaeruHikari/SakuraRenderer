#include "ScreenQuadVertex.hlsl"
#include "Utils.hlsl"
#include "Samplers.hlsl"

Texture2D gInputTexture : register(t0);
Texture2D gHistoryTexture : register(t1);
Texture2D gVelocityBuffer : register(t2);
Texture2D gDepthBuffer : register(t3);

struct TaaOut
{
    float4 Color;
    float4 History;
};

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

float4 PS(VertexOut pin) : SV_Target
{
    TaaOut pout;
    pout.Color = float4(1.f, 0.f, 1.f, 0.f);
    pout.History = float4(1.f, 1.f, 0.f, 0.f);
    return gDepthBuffer.Sample(gsamLinearClamp, pin.TexC);
    return pout.Color;
    //return pout;
}
