#ifndef _DEFERRED_REGISTERS_
#define _DEFERRED_REGISTERS_
#include "CommonCbs.hlsl"
Texture2D gGeometryAlbedo : register(t0);
Texture2D gGeometryNormal : register(t1);
Texture2D gGeometryWPos : register(t2);
Texture2D gGeometryRMO : register(t3);
Texture2D gBRDFLUT : register(t4);
TextureCube gIBLCubeMap[6] : register(t5);
#endif