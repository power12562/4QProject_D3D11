#pragma once
#include <D3DCore/D3D11_GameApp.h>
#include <Manager/SceneManager.h>
#include <Renderer.h>
#include <DefferdRenderer.h>
#include <StructuredBuffer.h>



// Å×½ºÆ®!!!!!!!!!!!!!!!!!!!!!!!!!!!
#include <../ImguiNodeEditor/imgui_node_editor.h>
namespace ImGui
{
	namespace Node = ax::NodeEditor;
}


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



	ImGui::Node::EditorContext* m_Context = nullptr;

	// End Test
	///////////////////////////////////////
};