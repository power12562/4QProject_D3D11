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

private:
	std::unique_ptr<IRenderer> renderer;

	ComPtr<struct IDXGISwapChain4> swapChain;
	Texture backBuffer;
};