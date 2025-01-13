
#include "../EngineShader/Shared.hlsli"

Texture2D albedoTexture : register(t0);

//#define FORWARD
#define GetAlbedo albedoTexture.Sample(DefaultSampler, input.Tex).rgb
#define GetRoughness 0
#define GetMetallic 0.5




#include "../EngineShader/BasePassPS.hlsl"

