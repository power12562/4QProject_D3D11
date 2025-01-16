
#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/GBufferMaterial.hlsli"


Texture2D t_3023778357952 : register(t1);


GBufferMaterial GetCustomGBufferMaterial(PS_INPUT input)
{
    GBufferMaterial material = GetDefaultGBufferMaterial(input);

float3 c_3023778357952 =  t_3023778357952.Sample(DefaultSampler, input.Tex).rgb;
float3 c_g3023778357952 =  t_3023778357952.Sample(DefaultSampler, input.Tex).g;
float3 c_r3023778357952 =  t_3023778357952.Sample(DefaultSampler, input.Tex).r;

material.albedo = c_3023778357952;
material.metallic = c_r3023778357952;
material.roughness = c_g3023778357952;

    return material;
}

#define GetGBufferMaterial GetCustomGBufferMaterial
#include "../EngineShader/BasePassPS.hlsl"
