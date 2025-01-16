
#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/GBufferMaterial.hlsli"




GBufferMaterial GetCustomGBufferMaterial(PS_INPUT input)
{
    GBufferMaterial material = GetDefaultGBufferMaterial(input);

float c_1855765704016 = 1;
float3 c_1856011792880 = float3(1, 0, 1);

material.albedo = c_1856011792880;
material.roughness = c_1855765704016;

    return material;
}

#define GetGBufferMaterial GetCustomGBufferMaterial
#include "../EngineShader/BasePassPS.hlsl"
