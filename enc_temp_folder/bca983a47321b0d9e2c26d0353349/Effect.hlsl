
#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/GBufferMaterial.hlsli"

GBufferMaterial GetCustomGBufferMaterial(PS_INPUT input)
{
	GBufferMaterial material = GetDefaultGBufferMaterial(input);

float3 c_2 = float3(1, 0, 0);
float c_x2 = 1;
material.albedo = c_2;
material.metallic = c_x2;
material.roughness = c_x2;



	return material;
}

#define GetGBufferMaterial GetCustomGBufferMaterial
#include "../EngineShader/BasePassPS.hlsl"
