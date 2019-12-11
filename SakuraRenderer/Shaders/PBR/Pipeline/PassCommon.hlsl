// Copyright SaeruHikari 2019, PBR Common Utils.
#ifndef __PASS_COMMON__
#define __PASS_COMMON__
#include "Utils.hlsl"
// Defaults for number of lights.
#ifndef MaxLights
    #define MaxLights 16
#endif

#ifndef NUM_DIR_LIGHTS
    #define NUM_DIR_LIGHTS 3
#endif

#ifndef NUM_POINT_LIGHTS
    #define NUM_POINT_LIGHTS 0
#endif

#ifndef NUM_SPOT_LIGHTS
    #define NUM_SPOT_LIGHTS 0
#endif

#ifndef TAA_ENABLED
    #define TAA_ENABLED
#endif

#ifndef REVERSE_Z
    #define REVERSE_Z
#endif

#endif