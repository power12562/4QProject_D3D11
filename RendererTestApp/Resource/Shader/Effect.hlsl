
#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/GBufferMaterial.hlsli"


Texture2D t_1729983980768 : register(t0);


GBufferMaterial GetCustomGBufferMaterial(PS_INPUT input)
{
    GBufferMaterial material = GetDefaultGBufferMaterial(input);

float3 c_1729983980768 =  t_1729983980768.Sample(DefaultSampler, input.Tex).rgb;
float3 c_g1729983980768 =  t_1729983980768.Sample(DefaultSampler, input.Tex).g;
float3 c_r1729983980768 =  t_1729983980768.Sample(DefaultSampler, input.Tex).r;

material.albedo = c_1729983980768;
material.metallic = c_r1729983980768;
material.roughness = c_g1729983980768;

    return material;
}

#define GetGBufferMaterial GetCustomGBufferMaterial
#include "../EngineShader/BasePassPS.hlsl"
