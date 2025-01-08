#pragma once
#include "RendererCore.h"
#include "ShaderResource.h"
#include <DirectXTex.h>

namespace ETextureUsage
{
	enum Type
	{
		None = 0,
		SRV = 0x8L,		// D3D11_BIND_SHADER_RESOURCE
		RTV = 0x20L,	// D3D11_BIND_RENDER_TARGET
		DSV = 0x40L,	// D3D11_BIND_DEPTH_STENCIL
		UAV = 0x80L,	// D3D11_BIND_UNORDERED_ACCESS
	};
}

struct Texture : public ShaderResource
{
public:
	Texture();
	~Texture();

public:
	void CreateTexture(std::unique_ptr<DirectX::ScratchImage>&& image, ETextureUsage::Type textureUsage);
	void LoadTexture(ID3D11Resource* resource, ETextureUsage::Type textureUsage);

	operator ID3D11ShaderResourceView* () { return shaderResourceView.Get(); }
	operator ID3D11RenderTargetView* () { return renderTargetView.Get(); }
	operator ID3D11DepthStencilView* () { return depthStencilView.Get(); }
	operator ID3D11UnorderedAccessView* () { return unorderedAccessView.Get(); }

private:
	ComPtr<ID3D11Resource> texture{};
	ComPtr<ID3D11ShaderResourceView> shaderResourceView{};
	ComPtr<ID3D11RenderTargetView> renderTargetView{};
	ComPtr<ID3D11DepthStencilView> depthStencilView{};
	ComPtr<ID3D11UnorderedAccessView> unorderedAccessView{};
};

