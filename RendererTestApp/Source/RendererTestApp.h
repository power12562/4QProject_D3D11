#pragma once
#include <D3DCore/D3D11_GameApp.h>
#include <Manager/SceneManager.h>
#include <Renderer.h>
#include <DefferdRenderer.h>

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

	ComPtr<struct IDXGISwapChain1> swapChain;
	Texture backBuffer;

private:
	void DXGIInit();


private:
	///////////////////////////////////////
	// Begin Test

	void TestInit();
	VertexShader vertexShader;
	class CubeObject* testObject;

	// End Test
	///////////////////////////////////////
};