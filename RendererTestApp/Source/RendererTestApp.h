#pragma once
#include <D3DCore/D3D11_GameApp.h>

#include <Manager/SceneManager.h>
#include <Renderer.h>
#include <DefferdRenderer.h>
#include <StructuredBuffer.h>
#include <NodeEditor\NodeEditor.h>


class RendererTestApp : public WinGameApp
{
	NodeEditor nodeEditor;
public:
	RendererTestApp();
	~RendererTestApp();

public:
	virtual void Start() override;
	virtual void Update() override;
	virtual void Render() override;
	void Uninitialize();

private:
	std::unique_ptr<DefferdRenderer> renderer;

	ComPtr<struct IDXGISwapChain1> swapChain;
	Texture backBuffer;

	std::vector<class PBRMeshRender*> charList;

private:
	void DXGIInit();


private:
	///////////////////////////////////////
	// Begin Test

	void TestInit();
	VertexShader vertexShader;
	class CubeObject* testObject;
	class GameObject* testObject2;
	class GameObject* testObject3;



	// End Test
	///////////////////////////////////////
};