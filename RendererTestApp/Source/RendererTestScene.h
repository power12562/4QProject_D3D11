#pragma once
#include <Scene/Base/Scene.h>
#include <NodeEditor\NodeEditor.h>

class RendererTestScene : public Scene
{
public:
	RendererTestScene() = default;
	virtual ~RendererTestScene() override = default;
	virtual void Start() override;

	virtual void ImGUIRender();
	std::unique_ptr<ShaderNodeEditor> nodeEditor;
	std::vector<MeshRender*> meshList;
};	