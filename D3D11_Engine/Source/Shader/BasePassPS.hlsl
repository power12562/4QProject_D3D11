#include "Shared.hlsli"

/** Shader Flags
 * USE_ALPHA
 * ALPHA_BLEND
 * ALPHA_TEST
 * FORWARD
 */



#ifndef GetAlpha
#define GetAlpha 1.0
#endif

#ifndef GetClipAlpha
#define GetClipAlpha 0.3333
#endif


#ifdef FORWARD

#ifndef GetColor
#define GetColor 1.0
#endif

#else // FORWARD

#ifndef GetAlbedo
#define GetAlbedo 1.0
#endif

#ifndef GetSpecular
#define GetSpecular 1.0
#endif

#ifndef GetNormal
#define GetNormal 1.0
#endif

#ifndef GetEmissive
#define GetEmissive 1.0
#endif

#endif // FORWARD


cbuffer cb_Light : register(b3)
{
	struct
	{
		float4 LightColor;
		float3 LightDir;
		float LightIntensity;
	}
    Lights[MAX_LIGHT_COUNT];
	int LightsCount;
}

cbuffer cb_PBRMaterial : register(b4)
{
	float4 baseColor;
	float Metalness;
	float Roughness;
    
	bool UseMetalnessMap;
	bool UseRoughnessMap;
	bool UseRMACMap;
};

struct PSResult
{
#ifdef FORWARD
	float4 color : SV_Target0;
#else
	float4 GBuffer[4] : SV_Target;
#endif

};

#ifdef ALPHA_BLEND
[earlydepthstencil]
#endif
PSResult main(PS_INPUT input) : SV_TARGET
{
	float opacity = GetAlpha;

#ifdef ALPHA_TEST
    clip( opacity - GetClipAlpha );
#endif
	
	PSResult result = (PSResult)1;
	
#ifdef FORWARD
	result.color = GetColor;
#else
	static int AlbedoSlot = 0;
	static int SpecularSlot = 1;
	static int NormalSlot = 2;
	static int EmissiveSlot = 3;
	
	result.GBuffer[AlbedoSlot] = GetAlbedo;
	result.GBuffer[SpecularSlot] = GetSpecular;
	result.GBuffer[NormalSlot] = GetNormal;
	result.GBuffer[EmissiveSlot] = GetEmissive;
#endif

	
	return result;
}

