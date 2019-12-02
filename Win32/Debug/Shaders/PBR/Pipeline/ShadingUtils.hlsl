// Generate TBN in VertexShader state to fetch better performance.
float3x3 GenerateTBN(float3 NormalW, float3 TangentW)
{
    float3 B = cross(NormalW, TangentW);
    return float3x3(TangentW, B, NormalW);
}

float3 TangentNormalToWorldSpace(float3 NormalTangentSpace, float3 NormalW, float3 TangentW)
{
    float3 B = cross(NormalW, TangentW);
    float3x3 TBN = float3x3(TangentW, B, NormalW);
    return mul(NormalTangentSpace, TBN);
}