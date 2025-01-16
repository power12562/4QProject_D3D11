#pragma once
#include "RendererCore.h"
#include "ShaderResource.h"
#include <vector>
#include <d3d11.h>

struct D3D11SamplerDescHash;
bool operator==(const D3D11_SAMPLER_DESC& lhs, const D3D11_SAMPLER_DESC& rhs); //D3D11_SAMPLER_DESC 비교 함수

class SamplerState : public ShaderResource
{
public:
	static void CleanupSamplers();

private:
	static ULONG CreateSamplerResource(const D3D11_SAMPLER_DESC& desc, ID3D11SamplerState** outputSampler);
	static std::unordered_map<D3D11_SAMPLER_DESC, ID3D11SamplerState*, D3D11SamplerDescHash> SamplerResourceMap;

public:
	SamplerState();
	~SamplerState();

	void SetSampler(const D3D11_SAMPLER_DESC& desc);
	inline operator ID3D11SamplerState* () const { return sampler.Get(); }
private:
	ComPtr<ID3D11SamplerState> sampler;
};

struct D3D11SamplerDescHash
{
	std::size_t operator()(const D3D11_SAMPLER_DESC& desc) const;
};