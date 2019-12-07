#include "PassCommon.hlsl"
#include "VertexCommon.hlsl"
#include "Samplers.hlsl"
#include "CommonCBs.hlsl"

float2 PS(VertexOut pin) :SV_Target
{
    float4 prevPos = pin.PrevPosVP;
    prevPos = prevPos / prevPos.w;
    float4 curPos = pin.CurPosVP;
    curPos = curPos / curPos.w;
    //negate Y because world coord and tex coord have different Y axis.
    prevPos.xy = prevPos.xy / float2(2.0f, -2.0f) + float2(0.5f, 0.5f); 
    //negate Y because world coord and tex coord have different Y axis.
    curPos.xy = curPos.xy / float2(2.0f, -2.0f) + float2(0.5f, 0.5f);
    return float2(curPos.x - prevPos.x, curPos.y - prevPos.y);
}