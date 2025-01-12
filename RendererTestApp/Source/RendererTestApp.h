#pragma once
#include <D3DCore/D3D11_GameApp.h>
#include <Manager/SceneManager.h>
#include <Renderer.h>
#include <DefferdRenderer.h>
#include <StructuredBuffer.h>



class RendererTestApp : public WinGameApp
{
public:
	RendererTestApp();
	~RendererTestApp();

public:
	virtual void Start() override;
	virtual void Update() override;
	virtual void Render() override;
	void Uninitialize();

private:
	std::unique_ptr<IRenderer> renderer;
	std::vector<ComPtr<IUnknown>> shaders;

	ComPtr<struct IDXGISwapChain1> swapChain;
	Texture backBuffer;

	Texture BRDF_LUT;
	Texture Diffuse_IBL;
	Texture Specular_IBL;



	struct DirectionLight
	{
		Vector4 LightColor;
		Vector3 LightDir;
		float LightIntensity;

		Matrix lightVP;
	};
	std::vector<DirectionLight> directLight;
	StructuredBuffer directLightBuffer;


private:
	void DXGIInit();


private:
	///////////////////////////////////////
	// Begin Test

	void TestInit();
	VertexShader vertexShader;
	class CubeObject* testObject;
	class GameObject* testObject2;

	// End Test
	///////////////////////////////////////
};