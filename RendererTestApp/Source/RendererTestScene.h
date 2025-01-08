#pragma once
#include <Scene/Base/Scene.h>

class RendererTestScene : public Scene
{
public:
	RendererTestScene() = default;
	virtual ~RendererTestScene() override = default;
	virtual void Start() override;

	virtual void ImGUIRender();
};