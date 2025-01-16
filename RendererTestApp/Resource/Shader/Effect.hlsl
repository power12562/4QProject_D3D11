
#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/GBufferMaterial.hlsli"


Texture2D t_2626154924752 : register(t0);


GBufferMaterial GetCustomGBufferMaterial(PS_INPUT input)
{
    GBufferMaterial material = GetDefaultGBufferMaterial(input);

float3 c_2626154924752 =  t_2626154924752.Sample(DefaultSampler, input.Tex).rgb;
float c_x2626156479456 = 1;
float c_y2626156479456 = 0;

material.albedo = c_2626154924752;
material.metallic = c_x2626156479456;
material.roughness = c_y2626156479456;

    return material;
}

#define GetGBufferMaterial GetCustomGBufferMaterial
#include "../EngineShader/BasePassPS.hlsl"
