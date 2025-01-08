#include "Texture.h"


Texture::Texture() = default;
Texture::~Texture() = default;

void Texture::CreateTexture(std::unique_ptr<DirectX::ScratchImage>&& image, ETextureUsage::Type textureUsage)
{
	HRESULT result;
	ID3D11Device* device = RendererUtility::GetDevice();
	ComPtr<ID3D11Resource> textureResource;

	result = DirectX::CreateTextureEx(device,
									  image->GetImages(),
									  image->GetImageCount(),
									  image->GetMetadata(),
									  D3D11_USAGE_DEFAULT,
									  textureUsage,
									  0,
									  0,
									  DirectX::CREATETEX_DEFAULT,
									  &textureResource);
	Check(result);

	LoadTexture(textureResource.Get(), textureUsage);
}

void Texture::LoadTexture(ID3D11Resource* resource, ETextureUsage::Type textureUsage)
{
	HRESULT result;
	ID3D11Device* device = RendererUtility::GetDevice();

	texture = resource;
	if (textureUsage & ETextureUsage::SRV)
	{
		result = device->CreateShaderResourceView(texture.Get(), nullptr, shaderResourceView.GetAddressOf());
		Check(result);
	}
	if (textureUsage & ETextureUsage::RTV)
	{
		result = (device->CreateRenderTargetView(texture.Get(), nullptr, renderTargetView.GetAddressOf()));
		Check(result);
	}
	if (textureUsage & ETextureUsage::DSV)
	{
		result = device->CreateDepthStencilView(texture.Get(), nullptr, depthStencilView.GetAddressOf());
		Check(result);
	}
	if (textureUsage & ETextureUsage::UAV)
	{
		result = device->CreateUnorderedAccessView(texture.Get(), nullptr, unorderedAccessView.GetAddressOf());
		Check(result);
	}
}