#include "common.hlsli"

// Pixel
float4 main(float4 tc : TEXCOORD0) : COLOR
{
    return tex2Dproj(s_base, tc);
}
