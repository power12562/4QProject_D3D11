
#include "../EngineShader/Shared.hlsli"

//#define FORWARD
#define GetAlbedo albedoTexture.Sample(DefaultSampler, input.Tex).rgb
#define GetRoughness 0
#define GetMetallic 0.5

Texture2D albedoTexture  : register(t0);


#include "../EngineShader/BasePassPS.hlsl"

