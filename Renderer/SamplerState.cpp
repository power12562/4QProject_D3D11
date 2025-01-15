#include "SamplerState.h"

std::unordered_map<D3D11_SAMPLER_DESC, ID3D11SamplerState*, D3D11SamplerDescHash> SamplerState::SamplerResourceMap;

void SamplerState::CleanupSamplers()
{
	for (auto& [key, sampler] : SamplerResourceMap)
	{
		ULONG refcounter = sampler->Release();
		if (refcounter == 0)
		{
			SamplerResourceMap.erase(key);
		}
	}
}

ULONG SamplerState::CreateSamplerResource(const D3D11_SAMPLER_DESC& desc, ID3D11SamplerState** outputSampler)
{
	auto iter = SamplerResourceMap.find(desc);
	if (iter != SamplerResourceMap.end())
	{
		*outputSampler = iter->second;
		ULONG refcounter = (*outputSampler)->AddRef();
		return refcounter;
	}
	else
	{
		Check(RendererUtility::GetDevice()->CreateSamplerState(&desc, outputSampler));
		SamplerResourceMap[desc] = *outputSampler;
		ULONG refcounter = (*outputSampler)->AddRef();
		return refcounter;
	}
}

SamplerState::SamplerState() = default;
SamplerState::~SamplerState() = default;

void SamplerState::SetSampler(const D3D11_SAMPLER_DESC& desc)
{
	ComPtr<ID3D11SamplerState> newSampler;
	CreateSamplerResource(desc, &newSampler);
	sampler = newSampler;
}

bool operator==(const D3D11_SAMPLER_DESC& lhs, const D3D11_SAMPLER_DESC& rhs)
{
	return lhs.Filter == rhs.Filter &&
		lhs.AddressU == rhs.AddressU &&
		lhs.AddressV == rhs.AddressV &&
		lhs.AddressW == rhs.AddressW &&
		lhs.MipLODBias == rhs.MipLODBias &&
		lhs.MaxAnisotropy == rhs.MaxAnisotropy &&
		lhs.ComparisonFunc == rhs.ComparisonFunc &&
		std::equal(std::begin(lhs.BorderColor), std::end(lhs.BorderColor), std::begin(rhs.BorderColor)) &&
		lhs.MinLOD == rhs.MinLOD &&
		lhs.MaxLOD == rhs.MaxLOD;
}
