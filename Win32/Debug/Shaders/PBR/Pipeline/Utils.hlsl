// Copyright SaeruHikari 2019, PBR Common Utils.
#ifndef __UTILS_DEFINATION__
#define __UTILS_DEFINATION__

#define Filament_MS
#define PI 3.1415926535898 

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
#endif