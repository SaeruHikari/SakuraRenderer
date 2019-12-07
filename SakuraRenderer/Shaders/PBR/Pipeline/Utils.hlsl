// Copyright SaeruHikari 2019, PBR Common Utils.
#ifndef __UTILS_DEFINATION__
#define __UTILS_DEFINATION__

#define Filament_MS
#define PI 3.1415926535898 

// for velocity rendering, motionblur and temporal AA
// velocity needs to support -2..2 screen space range for x and y
// texture is 16bit 0..1 range per channel
float2 EncodeVelocityToTexture(float2 In)
{
    // 0.499f is a value smaller than 0.5f to avoid using the full range to use the clear color (0,0) as special value
    // 0.5f to allow for a range of -2..2 instead of -1..1 for really fast motions for temporal AA
    return In * (0.499f * 0.5f) + 32767.0f / 65535.0f;
}
// see EncodeVelocityToTexture()
float2 DecodeVelocityFromTexture(float2 In)
{
    const float InvDiv = 1.0f / (0.499f * 0.5f);
    // reference
//    return (In - 32767.0f / 65535.0f ) / (0.499f * 0.5f);
    // MAD layout to help compiler
    return In * InvDiv - 32767.0f / 65535.0f * InvDiv;
}

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

inline int ihash(int n)
{
    n = pow((n << 13), n);
    return (n * (n * n * 15731 + 789221) + 1376312589) & 2147483647;
}

inline float frand(int n)
{
    return ihash(n) / 2147483647.0;
}

inline float2 cellNoise(int2 p, float4 RandSeed)
{
    int i = p.y * 256 + p.x;
    return sin(float2(frand(i), frand(i + 57)) * RandSeed.xy + RandSeed.zw);
}

float4x4 RotateX(float theta)
{
    float4x4 res = 0;
    res[0][0] = 1;
    res[3][3] = 1;
    res[1][1] = cos(theta);
    res[1][2] = -sin(theta);
    res[2][1] = sin(theta);
    res[2][2] = cos(theta);
    return res;
}

float4x4 RotateY(float theta)
{
    float4x4 res = 0;
    res[1][1] = 1;
    res[3][3] = 1;
    res[0][0] = cos(theta);
    res[0][2] = sin(theta);
    res[2][0] = -sin(theta);
    res[2][2] = cos(theta);
    return res;
}

float4x4 RotateZ(float theta)
{
    float4x4 res = 0;
    res[2][2] = 1;
    res[3][3] = 1;
    res[0][0] = cos(theta);
    res[0][1] = -sin(theta);
    res[1][0] = sin(theta);
    res[1][1] = cos(theta);
    return res;
}

// 8x TAA
static const float Halton_2[8] =
{
    0.0,
		-1.0 / 2.0,
		1.0 / 2.0,
		-3.0 / 4.0,
		1.0 / 4.0,
		-1.0 / 4.0,
		3.0 / 4.0,
		-7.0 / 8.0
};

// 8x TAA
static const float Halton_3[8] =
{
    -1.0 / 3.0,
		1.0 / 3.0,
		-7.0 / 9.0,
		-1.0 / 9.0,
		5.0 / 9.0,
		-5.0 / 9.0,
		1.0 / 9.0,
		7.0 / 9.0
};

float3 RGB2YCoCgR(float3 rgbColor)
{
    float3 YCoCgRColor;

    YCoCgRColor.y = rgbColor.r - rgbColor.b;
    float temp = rgbColor.b + YCoCgRColor.y / 2;
    YCoCgRColor.z = rgbColor.g - temp;
    YCoCgRColor.x = temp + YCoCgRColor.z / 2;

    return YCoCgRColor;
}

float3 YCoCgR2RGB(float3 YCoCgRColor)
{
    float3 rgbColor;

    float temp = YCoCgRColor.x - YCoCgRColor.z / 2;
    rgbColor.g = YCoCgRColor.z + temp;
    rgbColor.b = temp - YCoCgRColor.y / 2;
    rgbColor.r = rgbColor.b + YCoCgRColor.y;

    return rgbColor;
}

float3 ACESToneMapping(float3 color, float3 adapted_lum)
{
    const float A = 2.51f;
    const float B = 0.03f;
    const float C = 2.43f;
    const float D = 0.59f;
    const float E = 0.14f;

    color *= adapted_lum;
    return (color * (A * color + B)) / (color * (C * color + D) + E);
}

#endif