#pragma once

#include "RendererCore.h"
#include "Renderer.h"
#include <vector>

#include "StructuredBuffer.h"
#include <array>


struct DirectionLight
{
	Vector4 LightColor;
	Vector3 LightDir;
	float LightIntensity;

	Matrix lightVP;
};

class DefferdRenderer : public IRenderer
{
public:
	DefferdRenderer();
	virtual ~DefferdRenderer();

public:
	virtual void AddDrawCommand(_In_ const MeshDrawCommand& command) override;
	virtual void AddBinadble(std::string_view key, const Binadble& bindable) override;
	virtual void RemoveBinadble(std::string_view key) override;
	virtual void SetRenderTarget(_In_ Texture& target) override;
	virtual void Render() override;

	virtual void SetCameraMatrix(const Matrix& world);
	virtual void SetProjection(float fov, float nearZ, float farZ);
	
	//�������� �ȵ����� ����̾���....
	ComputeShader deferredCS;
	PixelShader deferredPS;
	VertexShader fullScreenVS;

	Texture BRDF_LUT;
	Texture Diffuse_IBL;
	Texture Specular_IBL;

	StructuredBuffer directLightBuffer;
private:
	std::vector<MeshDrawCommand> allDrawCommandsOrigin{};
	std::vector<MeshDrawCommand*> allDrawCommands{};
	std::vector<MeshDrawCommand*> deferredDrawCommands{};
	std::vector<MeshDrawCommand*> forwardDrawCommands{};
	std::vector<MeshDrawCommand*> alphaDrawCommands{};

	Texture renderTarget{};

	std::vector<std::string> bindablesKey;
	std::vector<Binadble> bindables;

#pragma region RHIDevice

	ComPtr<ID3D11Device> device{};
	ComPtr<ID3D11DeviceContext> immediateContext{};

#pragma endregion RHIDevice


#pragma region RenderState

	ComPtr<struct ID3D11DepthStencilState> defaultDSS{};
	ComPtr<struct ID3D11DepthStencilState> noWriteDSS{};
	ComPtr<struct ID3D11BlendState> noRenderState{};
	ComPtr<struct ID3D11BlendState> alphaRenderState{};

#pragma endregion RenderState

#pragma region RenderTexture

	Texture depthStencilTexture{};
	Texture depthStencilTexture2{};
	std::array<Texture, 4> renderBuffers{};
	Texture deferredBuffer{};
	std::array<Texture, 2> PostProcessTexture;
	int renderBufferIndex{ 0 };


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
	Binadble cameraBinadbleVS;
	Binadble cameraBinadblePS;
	Binadble cameraBinadbleCS;

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

