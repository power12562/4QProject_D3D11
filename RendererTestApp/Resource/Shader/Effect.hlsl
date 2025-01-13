
#include "../EngineShader/Shared.hlsli"

Texture2D albedoTexture : register(t0);

float3 Effect()
{
	return float3(1.0, 0.0, 1.0);
}




//#define FORWARD
#define GetAlbedo 0
#define GetRoughness 0
#define GetMetallic 0


#define GetEmissive  Effect()
#define GetAmbiatOcclusion 1
#define GetSpecular 0


#include "../EngineShader/BasePassPS.hlsl"

