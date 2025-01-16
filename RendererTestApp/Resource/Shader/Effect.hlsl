
#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/GBufferMaterial.hlsli"


Texture2D t_2007787453632 : register(t0);


GBufferMaterial GetCustomGBufferMaterial(PS_INPUT input)
{
    GBufferMaterial material = GetDefaultGBufferMaterial(input);

float3 c_2007787453632 =  t_2007787453632.Sample(DefaultSampler, input.Tex).rgb;
float c_x2007217487136 = 1;
float c_y2007217487136 = 0;

material.albedo = c_2007787453632;
material.metallic = c_x2007217487136;
material.roughness = c_y2007217487136;
material.specular = c_x2007217487136;
material.ambiantOcclusion = c_x2007217487136;

    return material;
}

#define GetGBufferMaterial GetCustomGBufferMaterial
#include "../EngineShader/BasePassPS.hlsl"
