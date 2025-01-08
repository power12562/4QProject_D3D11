#pragma once
#include <Utility/D3D11Utility.h>

struct D3D11SamplerDescHash;
bool operator==(const D3D11_SAMPLER_DESC& lhs, const D3D11_SAMPLER_DESC& rhs);

class D3DSamplerState
{
public:
	static ULONG CreateSamplerResource(D3D11_SAMPLER_DESC& desc, ID3D11SamplerState*& outputSampler);
	static void ClearSamplerResources();
	static const size_t MAX_SIZE = 16;

private:
	static std::unordered_map<D3D11_SAMPLER_DESC, ID3D11SamplerState*, D3D11SamplerDescHash> SamplerResourceMap;
public:

public:
	D3DSamplerState() = default;
	//D3DSamplerState();
	~D3DSamplerState();

	void resize(size_t newsize);
	constexpr size_t size() { return samplerList.size(); }

	void SetSamplerState(int index, D3D11_SAMPLER_DESC& desc);
	void ResetSamplerState(int index);

	ID3D11SamplerState** data() { return samplerList.data(); }
	ID3D11SamplerState* operator[](int index) { return samplerList[index]; }

public:
	D3DSamplerState(const D3DSamplerState& rhs);
	D3DSamplerState& operator=(const D3DSamplerState& rhs);

private:
	std::vector<ID3D11SamplerState*> samplerList;
};

struct D3D11SamplerDescHash
{
	std::size_t operator()(const D3D11_SAMPLER_DESC & desc) const;
};