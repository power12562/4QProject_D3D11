#ifndef __BASEPASSPS_HLSL__
#define __BASEPASSPS_HLSL__

#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/BRDF.hlsli"

/** Shader Flags
 * USE_ALPHA
 * ALPHA_BLEND
 * ALPHA_TEST
 * FORWARD
 */


struct DirectionLight
{
	float4 LightColor;
	float3 LightDir;
	float LightIntensity;
};

float3 GetRadiance(DirectionLight light)
{
	return light.LightColor.rgb * light.LightIntensity;
}

float3 GetDirection(DirectionLight light)
{
	return normalize(light.LightDir.xyz);
}

StructuredBuffer<DirectionLight> DirecLights : register(t16);
Texture2DArray ShadowMap : register(t20);


float3 DefaultLit(float3 albedo, float metallic, float roughness, float3 F0, float3 N, float3 V, float ambiantOcclusion)
{
	float3 finalColor = 0;
	//uint count, width, height, lod;
	//ShadowMap.GetDimensions(0, width, height, count, lod);
	//for (uint i = 0; i < count; i++)
	//{
		
	//}
	uint LightsCount, stride;
	DirecLights.GetDimensions(LightsCount, stride);
	for (uint i = 0; i < LightsCount; ++i)
	{
		finalColor += BRDF(albedo, metallic, roughness, F0, N, V, GetDirection(DirecLights[i])) * GetRadiance(DirecLights[i]);
	}
	finalColor += BRDF_IBL(albedo, metallic, roughness, F0, N, V) * (1 - ambiantOcclusion);
	return finalColor;
}

#ifndef GetAlpha
#define GetAlpha 1.0
#endif

#ifndef GetClipAlpha
#define GetClipAlpha 0.3333
#endif

#ifndef GetAlbedo
#define GetAlbedo 1.0
#endif

#ifndef GetMetallic
#define GetMetallic 1.0
#endif

#ifndef GetSpecular
#define GetSpecular 1.0
#endif

#ifndef GetRoughness
#define GetRoughness 0.5
#endif

#ifndef GetAmbiatOcclusion
#define GetAmbiatOcclusion 0.8
#endif

#ifndef GetNormal
#define GetNormal float3(0.0, 0.0, 1.0)
#endif

#ifndef GetEmissive
#define GetEmissive float3(0.0, 0.0, 0.0)
#endif

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
	float opacity = GetAlpha;

#ifdef ALPHA_TEST
    clip( opacity - GetClipAlpha );
#endif
	
	PSResult result = (PSResult)1;
	
	float3 albedo = GetAlbedo;
	float metallic = GetMetallic;
	float specular = GetSpecular;
	float roughness = GetRoughness;
	float3 normal = GetNormal;
	float3 emissiveColor = GetEmissive;
	float ambiantOcclusion = GetAmbiatOcclusion;
	
	float3x3 TBN = float3x3(input.Tangent, input.BiTangent, input.Normal);
	float3 N = normalize(mul(normal, TBN));
	
#ifdef FORWARD
	float3 finalColor = 0;
	
	float3 F0 = lerp(0.04, albedo, metallic) * specular;
    float3 V = normalize(MainCamPos - input.World);
	
	finalColor = DefaultLit(albedo, metallic, roughness, F0, N, V, ambiantOcclusion);
	result.color.rgb = LinearToGammaSpace(finalColor + emissiveColor);
	result.color.a = opacity;
	
	
#else
	static int AlbedoSlot = 0;
	static int SpecularSlot = 1;
	static int NormalSlot = 2;
	static int EmissiveSlot = 3;
	
	result.GBuffer[AlbedoSlot].rgb = albedo;
	result.GBuffer[SpecularSlot] = float4(metallic, specular, roughness, ambiantOcclusion);
	result.GBuffer[NormalSlot].rgb = N;
	result.GBuffer[EmissiveSlot].rgb = emissiveColor;
#endif
	
	return result;
}

#endif // __BASEPASSPS_HLSL__