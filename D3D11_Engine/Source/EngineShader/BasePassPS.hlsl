#ifndef __BASEPASSPS_HLSL__
#define __BASEPASSPS_HLSL__

#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/Light.hlsli"
#include "../EngineShader/GBufferMaterial.hlsli"

/** Shader Flags
 * USE_ALPHA
 * ALPHA_BLEND
 * ALPHA_TEST
 * FORWARD
 */



#ifndef GetGBufferMaterial 
#define GetGBufferMaterial GetDefaultGBufferMaterial
#endif //GetGBufferMaterial


struct PSResult
{
#ifdef FORWARD
	float4 color : SV_TARGET0;
#else
	float4 GBuffer[4] : SV_TARGET;
#endif

};

#ifdef ALPHA_BLEND
[earlydepthstencil]
#endif
PSResult main(PS_INPUT input)
{
	GBufferMaterial material = GetGBufferMaterial(input);
	float opacity = material.alpha;

#ifdef ALPHA_TEST
    clip( opacity - material.clipAlpha );
#endif
	
#ifdef DITHERING
	//https://wincnt-shim.tistory.com/395
	static const float DitheringPattern[16] =
	{
		0  / 16.0, 8  / 16.0, 2  / 16.0, 10 / 16.0,
		12 / 16.0, 4  / 16.0, 14 / 16.0, 6  / 16.0,
		3  / 16.0, 11 / 16.0, 1  / 16.0, 9  / 16.0,
		15 / 16.0, 7  / 16.0, 13 / 16.0, 5  / 16.0
	};
	uint DitheringIndex = (uint(input.Pos.x) % 4) * 4 + uint(input.Pos.y) % 4;
	float DitheringOut = 1.0f - DitheringPattern[DitheringIndex];
	clip(opacity - DitheringOut);
#endif // DITHERING
	
	PSResult result = (PSResult)1;
	float3 albedo = material.albedo;
	float metallic = material.metallic;
	float specular = material.specular;
	float roughness = material.roughness;
	float3 normal = material.normal;
	float3 emissiveColor = material.emissive;
	float ambiantOcclusion = material.ambiantOcclusion;
	
	float3x3 TBN = float3x3(input.Tangent, input.BiTangent, input.Normal);
	float3 N = normalize(mul(normal, TBN));
	
#ifdef FORWARD
	float3 finalColor = 0;
	
	float3 F0 = lerp(0.04, albedo, metallic) * specular;
    float3 V = normalize(MainCamPos - input.World);
	
	finalColor = DefaultLit(albedo, metallic, roughness, F0, input.World, N, V, ambiantOcclusion);
	result.color.rgb = LinearToGammaSpace(finalColor + emissiveColor);
	result.color.a = opacity;
	
	
#else
	static int AlbedoSlot = 0;
	static int SpecularSlot = 1;
	static int NormalSlot = 2;
	static int EmissiveSlot = 3;
	
	result.GBuffer[AlbedoSlot].rgb = albedo;
	result.GBuffer[SpecularSlot] = float4(specular, metallic, roughness, ambiantOcclusion);
	result.GBuffer[NormalSlot].rgb = N;
	result.GBuffer[EmissiveSlot].rgb = emissiveColor;
#endif
	
	return result;
}

#endif // __BASEPASSPS_HLSL__