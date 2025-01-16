
#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/GBufferMaterial.hlsli"


Texture2D t_2449223994048 : register(t0);


GBufferMaterial GetCustomGBufferMaterial(PS_INPUT input)
{
    GBufferMaterial material = GetDefaultGBufferMaterial(input);

float3 c_2449223994048 =  t_2449223994048.Sample(DefaultSampler, input.Tex).rgb;
float c_x2449226999152 = 1;
float c_y2449226999152 = 0;

material.albedo = c_2449223994048;
material.metallic = c_x2449226999152;
material.roughness = c_y2449226999152;

    return material;
}

#define GetGBufferMaterial GetCustomGBufferMaterial
#include "../EngineShader/BasePassPS.hlsl"
