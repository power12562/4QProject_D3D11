//https://www.slideshare.net/DevCentralAMD/vertex-shader-tricks-bill-bilodeau

#include "../EngineShader/Shared.hlsli"

Texture2D T_Depth : register(t0); //Depth.r

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
};

VS_OUTPUT main(uint id : SV_VertexID)
{
	VS_OUTPUT output;
	
	output.texcoord.x = float(id / 2) * 2.0;
	output.texcoord.y = 1.0 - float(id % 2) * 2.0;
	
	output.position.x = float(id / 2) * 4.0 - 1.0;
	output.position.y = float(id % 2) * 4.0 - 1.0;
	output.position.z = T_Depth.SampleLevel(DefaultSampler, output.texcoord, 0) + 0.00000005;
	output.position.w = 1.0;
	
	
	return output;
}