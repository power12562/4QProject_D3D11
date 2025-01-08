#include "D3DSamplerState.h"
#include <Utility/MemoryUtility.h>
#include <D3DCore/D3DRenderer.h>
#include <Utility/Console.h>

using namespace Utility;

std::unordered_map<D3D11_SAMPLER_DESC, ID3D11SamplerState*, D3D11SamplerDescHash> D3DSamplerState::SamplerResourceMap;

ULONG D3DSamplerState::CreateSamplerResource(D3D11_SAMPLER_DESC& desc, ID3D11SamplerState*& outputSampler)
{
	auto iter = SamplerResourceMap.find(desc);
	if (iter != SamplerResourceMap.end())
	{
		outputSampler = iter->second;
		ULONG refcounter = outputSampler->AddRef();
		return refcounter;
	}
	else
	{
		CheckHRESULT(d3dRenderer.GetDevice()->CreateSamplerState(&desc, &outputSampler));
		SamplerResourceMap[desc] = outputSampler;
		ULONG refcounter = outputSampler->AddRef();
		return refcounter;
	}
}

void D3DSamplerState::ClearSamplerResources()
{
	for (auto& sampler : SamplerResourceMap)
	{
		ULONG refcounter = sampler.second->Release();
	}
	SamplerResourceMap.clear();
}

D3DSamplerState::~D3DSamplerState()
{
	for (auto& item : samplerList)
	{
		ULONG refcounter = SafeRelease(item);
	}
}

void D3DSamplerState::resize(size_t newsize)
{
	if (newsize < D3DSamplerState::MAX_SIZE)
	{
		if (newsize < samplerList.size())
		{
			for (int i = (int)newsize; i < samplerList.size(); i++)
			{
				ResetSamplerState(i);
			}
		}
		samplerList.resize(newsize);
	}
	else
	{
		__debugbreak();
		//최대 크기 초과
	}
}

void D3DSamplerState::SetSamplerState(int index, D3D11_SAMPLER_DESC& desc)
{
	ResetSamplerState(index);
	CreateSamplerResource(desc, samplerList[index]);
}

void D3DSamplerState::ResetSamplerState(int index)
{
	if (samplerList[index])
	{
		samplerList[index]->Release();
		samplerList[index] = nullptr;
	}
}

D3DSamplerState::D3DSamplerState(const D3DSamplerState& rhs)
{
	if (this == &rhs)
		return;

	samplerList = rhs.samplerList;
	for (auto& sampler : samplerList)
	{
		sampler->AddRef();
	}
}

D3DSamplerState& D3DSamplerState::operator=(const D3DSamplerState& rhs)
{
	if (this == &rhs)
		return *this;

	for (auto& sampler : samplerList)
	{
		sampler->Release();
	}
	samplerList = rhs.samplerList;
	for (auto& sampler : samplerList)
	{
		ULONG refcounter = sampler->AddRef();
	}

	return *this;
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

std::size_t D3D11SamplerDescHash::operator()(const D3D11_SAMPLER_DESC& desc) const
{
	std::size_t hash = 0;

	// 각 멤버에 std::hash 적용
	hash ^= std::hash<int>()(desc.Filter);
	hash ^= std::hash<int>()(desc.AddressU) << 1;
	hash ^= std::hash<int>()(desc.AddressV) << 2;
	hash ^= std::hash<int>()(desc.AddressW) << 3;
	hash ^= std::hash<float>()(desc.MipLODBias) << 4;
	hash ^= std::hash<int>()(desc.MaxAnisotropy) << 5;
	hash ^= std::hash<int>()(desc.ComparisonFunc) << 6;

	// BorderColor는 배열이라 개별적으로 해싱
	for (int i = 0; i < 4; ++i) {
		hash ^= std::hash<float>()(desc.BorderColor[i]) << (7 + i);
	}

	hash ^= std::hash<float>()(desc.MinLOD) << 11;
	hash ^= std::hash<float>()(desc.MaxLOD) << 12;

	return hash;
}
