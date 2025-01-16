
#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/GBufferMaterial.hlsli"


Texture2D t_2288655999264 : register(t0);


GBufferMaterial GetCustomGBufferMaterial(PS_INPUT input)
{
    GBufferMaterial material = GetDefaultGBufferMaterial(input);

float3 c_2288655999264 =  t_2288655999264.Sample(DefaultSampler, input.Tex).rgb;
float c_2288656015824 = 1;
float c_x2288656010448 = 1;
float c_y2288656010448 = 0;

material.albedo = c_2288655999264;
material.metallic = c_x2288656010448;
material.roughness = c_y2288656010448;
material.emissive = c_2288656015824;

    return material;
}

#define GetGBufferMaterial GetCustomGBufferMaterial
#include "../EngineShader/BasePassPS.hlsl"
