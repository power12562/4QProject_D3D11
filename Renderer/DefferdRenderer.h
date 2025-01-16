#pragma once

#include "RendererCore.h"
#include "Renderer.h"
#include "Light.h"
#include <vector>

#include <array>



struct MeshDrawCommand2
{
public:
	RendererBuffer vertexBuffer;
	RendererBuffer indexBuffer;
	uint32_t indexCounts;
	uint32_t vertexStride;
	VertexShader vertexShader;
	int vsShaderResourcesStart;
	int vsShaderResourcesEnd;

	DirectX::BoundingOrientedBox boundingBox;


	PixelShader pixelShader;
	int psShaderResourcesStart;
	int psShaderResourcesEnd;
};

class DefferdRenderer : public IRenderer
{
public:
	DefferdRenderer();
	virtual ~DefferdRenderer();

public:
	void SetSkyBoxDrawCommand(_In_ const MeshDrawCommand& command);


	virtual void AddDrawCommand(_In_ const MeshDrawCommand& command) override;
	virtual void AddBinadble(std::string_view key, const Binadble& bindable) override;
	virtual void RemoveBinadble(std::string_view key) override;
	virtual void SetRenderTarget(_In_ Texture& target) override;
	virtual void Render() override;

	virtual void SetCameraMatrix(const Matrix& world);
	virtual void SetProjection(float fov, float nearZ, float farZ);
	
	//마음에는 안들지만 방법이없음....
	ComputeShader deferredCS;
	PixelShader deferredPS;
	VertexShader fullScreenVS;

	Texture BRDF_LUT;
	Texture Diffuse_IBL;
	Texture Specular_IBL;

	DirectionLightBuffer directLight;
private:
	std::vector<MeshDrawCommand2> allDrawCommandsOrigin{};
	std::vector<MeshDrawCommand2*> allDrawCommands{};
	std::vector<MeshDrawCommand2*> deferredDrawCommands{};
	std::vector<MeshDrawCommand2*> forwardDrawCommands{};
	std::vector<MeshDrawCommand2*> alphaDrawCommands{};
	std::vector<Binadble> drawCommandBindable{};
	MeshDrawCommand skyBoxDrawCommand{};

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

#pragma endregion Camera

	ComPtr<ID3D11SamplerState> samplerState;

private:
	void ProcessDrawCommand(MeshDrawCommand& drawCommands);
	void ProcessDrawCommands(std::vector<MeshDrawCommand2*>& drawCommands, bool isWithMaterial = true);
	void BindBinadble(const Binadble& bindable);
};

