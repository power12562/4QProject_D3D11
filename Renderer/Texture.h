#pragma once
#include "RendererCore.h"
#include "ShaderResource.h"
#include <DirectXTex.h>

struct ETextureUsage
{
	using Type = uint32_t;
	static constexpr Type SRV = 0x8L;		// D3D11_BIND_SHADER_RESOURCE
	static constexpr Type RTV = 0x20L;		// D3D11_BIND_RENDER_TARGET
	static constexpr Type DSV = 0x40L;		// D3D11_BIND_DEPTH_STENCIL
	static constexpr Type UAV = 0x80L;		// D3D11_BIND_UNORDERED_ACCESS

	Type type;
	ETextureUsage operator|(ETextureUsage other) { return ETextureUsage{ this->type | other.type }; }
};

struct Texture : public ShaderResource
{
public:
	Texture();
	~Texture();

public:
	void CreateTexture(std::unique_ptr<DirectX::ScratchImage>&& image, ETextureUsage::Type textureUsage);
	void LoadTexture(ID3D11Resource* resource, ETextureUsage::Type textureUsage, _In_opt_ struct D3D11_SHADER_RESOURCE_VIEW_DESC *srvFormat = nullptr);
	void LoadTexture(ID3D11ShaderResourceView* srv);

	operator ID3D11Texture2D* ();
	operator ID3D11Resource* () { return texture.Get(); }
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

struct TextureArray : public ShaderResource
{
public:
	TextureArray();
	~TextureArray();

public:
	//void CreateTexture(std::unique_ptr<DirectX::ScratchImage>&& image, ETextureUsage::Type textureUsage);
	void LoadTexture(ID3D11Resource* resource, ETextureUsage::Type textureUsage);
	void LoadTexture(ID3D11ShaderResourceView* srv);

	operator ID3D11Texture2D* ();
	operator ID3D11Resource* () { return texture.Get(); }
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
