#pragma once

#include "RendererCore.h"
#include "Renderer.h"
#include <vector>

#include <array>


class DefferdRenderer : public IRenderer
{
public:
	DefferdRenderer();
	virtual ~DefferdRenderer();

public:
	virtual void AddDrawCommand(_In_ const MeshDrawCommand& command) override;
	virtual void AddBinadble(_In_ const Binadble& bindable) override;
	virtual void SetRenderTarget(_In_ Texture& target) override;
	virtual void Render() override;

	virtual void SetCameraMatrix(const Matrix& world);
	virtual void SetProjection(float fov, float nearZ, float farZ);

private:
	std::vector<MeshDrawCommand> allDrawCommandsOrigin{};
	std::vector<MeshDrawCommand*> allDrawCommands{};
	std::vector<MeshDrawCommand*> deferredDrawCommands{};
	std::vector<MeshDrawCommand*> forwardDrawCommands{};

	Texture* renderTarget{ nullptr };

	std::vector<Binadble> bindables;

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
	
	Texture depthStencilTexture{};
	std::array<Texture, 4> renderBuffers{};

#pragma endregion RenderTexture


#pragma region ViewPort
	
	uint32_t width{ 0 };
	uint32_t height{ 0 };

#pragma endregion ViewPort


#pragma region Camera

	float fov{ 0.0f };
	float nearZ{ 0.0f };
	float farZ{ 0.0f };
	Matrix cameraWorld;
	Matrix cameraProjection;
	ConstantBuffer cameraBuffer;
	Binadble cameraBinadble;

	struct CameraBufferData
	{
		alignas(16) Vector3 MainCamPos;
		alignas(16) Matrix View;
		alignas(16) Matrix Projection;
		alignas(16) Matrix IVM;
		alignas(16) Matrix IPM;
	};

#pragma endregion Camera

private:
	void ProcessDrawCommands(std::vector<MeshDrawCommand*>& drawCommands, bool isWithMaterial = true);
	void BindBinadble(const Binadble& bindable);
};

