#pragma once
#include <Scene/Base/Scene.h>

class NewRendererTestScene : public Scene
{
public:
	NewRendererTestScene() = default;
	virtual ~NewRendererTestScene() override = default;
	virtual void Start() override;

	virtual void ImGUIRender();
};