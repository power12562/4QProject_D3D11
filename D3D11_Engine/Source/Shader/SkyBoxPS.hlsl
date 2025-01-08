#include "Shared.hlsli"
#include "SkyBoxShared.hlsli"
SamplerState samLinear : register(s0);
TextureCube envTexture : register(t0);

struct ps_out
{
    float4 final : SV_Target;
    float depth : SV_Depth;
};

ps_out main(PixelShaderInput input)
{
    ps_out Output;
    Output.final = envTexture.Sample(samLinear, input.TexCoords);
    Output.depth = 1.0 - Epsilon;
    return Output;
}
