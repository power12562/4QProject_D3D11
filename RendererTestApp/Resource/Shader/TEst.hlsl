
#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/GBufferMaterial.hlsli"

Texture2D albedoTexture : register(t0);
//#define FORWARD

GBufferMaterial GetCustomGBufferMaterial(PS_INPUT input)
{
	GBufferMaterial material = GetDefaultGBufferMaterial(input);
	
	material.albedo = albedoTexture.Sample(DefaultSampler, input.Tex).rgb;
	material.roughness = 1;
	material.metallic = 0.5;
	
	
	return material;

}
#define GetGBufferMaterial GetCustomGBufferMaterial

#include "../EngineShader/BasePassPS.hlsl"

