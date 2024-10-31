#include "common.hlsli"

struct v2p
{
    float2 tc0 : TEXCOORD0; // base
    float2 tc1 : TEXCOORD1; // lmap
    float2 tc2 : TEXCOORD2; // hemi
    float3 tc3 : TEXCOORD3; // env
    float4 c0 : COLOR0;
    float3 c1 : COLOR1;
    //	float  fog	:FOG;
};

uniform samplerCUBE s_env;

// Pixel
float4 main(v2p I) : COLOR
{
    float4 t_base = tex2D(s_base, I.tc0);
    float4 t_lmap = tex2D(s_lmap, I.tc1);
    float4 t_env = texCUBE(s_env, I.tc3);

    // lighting
    float3 l_base = t_lmap.rgb; // base light-map
    float3 l_hemi = I.c0 * p_hemi(I.tc2); // hemi
    float3 l_sun = I.c1 * t_lmap.a; // sun color
    float3 light = L_ambient + l_base + l_sun + l_hemi;

    // final-color
    float3 base = lerp(t_env, t_base, t_base.a);
    float3 final = light * base * 2.0f;

    // out
    return float4(final.r, final.g, final.b, t_base.a * I.c0.a * I.c0.a);
}
