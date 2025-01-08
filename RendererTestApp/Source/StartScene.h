#pragma once
#include <Scene/Base/Scene.h>

class StartScene : public Scene
{
public:
	StartScene() = default;
	virtual ~StartScene() override = default;
	virtual void Start() override;

	virtual void ImGUIRender();
};