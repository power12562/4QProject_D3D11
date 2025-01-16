
#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/GBufferMaterial.hlsli"


Texture2D t_1492197825040 : register(t0);


GBufferMaterial GetCustomGBufferMaterial(PS_INPUT input)
{
    GBufferMaterial material = GetDefaultGBufferMaterial(input);

float3 c_1492197825040 =  t_1492197825040.Sample(DefaultSampler, input.Tex).rgb;
float c_x1492053298320 = 1;
float c_y1492053298320 = 0;

material.albedo = c_1492197825040;
material.metallic = c_x1492053298320;
material.roughness = c_y1492053298320;

    return material;
}

#define GetGBufferMaterial GetCustomGBufferMaterial
#include "../EngineShader/BasePassPS.hlsl"
