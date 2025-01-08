#pragma once

#include "RendererCore.h"
#include "Renderer.h"
#include <vector>

#include <array>

using Matrix = DirectX::SimpleMath::Matrix;

class DefferdRenderer : public IRenderer
{
public:
	DefferdRenderer();

public:
	virtual void AddDrawCommand(_In_ DrawCommand& command) override;
	virtual void SetRenderTarget(_In_ Texture& target) override;
	virtual void Render() override;

	void SetCameraMatricx(const Matrix& world);
	void SetProjection(float fov, float nearZ, float farZ);

private:
#pragma region IRenderer
	std::vector<DrawCommand> drawCommands{};
	Texture* renderTarget{ nullptr };

#pragma endregion IRenderer


#pragma region RHIDevice

	ComPtr<ID3D11Device> device{};
	ComPtr<ID3D11DeviceContext> immediateContext{};

#pragma endregion RHIDevice


#pragma region RenderState
	
	ComPtr<struct ID3D11DepthStencilState> noWriteDSS{};
	ComPtr<struct ID3D11BlendState> noRenderState{};
	ComPtr<struct ID3D11BlendState> alphaRenderState{};

#pragma endregion RenderState

#pragma region RenderTexture
	
	ComPtr<struct ID3D11DepthStencilView> depthStencilDSV{};
	ComPtr<struct ID3D11ShaderResourceView> depthStencilSRV{};
	std::array<ComPtr<struct ID3D11RenderTargetView>, 4> renderRTV{};
	std::array<ComPtr<struct ID3D11ShaderResourceView>, 4> renderSRV{};

#pragma endregion RenderTexture


#pragma region ViewPort
	
	uint32_t width;
	uint32_t height;

#pragma endregion ViewPort


#pragma region Camera

	float fov;
	float nearZ;
	float farZ;
	Matrix cameraWorld;
	Matrix cameraProjection;
	ConstantBuffer cameraBuffer;

#pragma endregion Camera

};

