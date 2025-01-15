#ifndef __GBUFFERMATERIAL_HLSL__
#define __GBUFFERMATERIAL_HLSL__

#include "../EngineShader/Shared.hlsli"


struct GBufferMaterial
{
	float3 albedo;
	float alpha;
	
	float specular;
	float metallic;
	float roughness;
	float ambiantOcclusion;
	
	float3 normal;
	float clipAlpha;
	
	float3 emissive;
};

GBufferMaterial GetDefaultGBufferMaterial(PS_INPUT input)
{
	GBufferMaterial material;
	
	material.albedo = 1.0;
	material.alpha = 1.0;
	material.specular = 1.0;
	material.metallic = 1.0;
	material.roughness = 0.0;
	material.ambiantOcclusion = 1;
	
	material.normal = float3(0.0, 0.0, 1.0);
	material.emissive = 0;
	
	material.clipAlpha = 0.3333;
	
	return material;
}

#endif // __GBUFFERMATERIAL_HLSL__