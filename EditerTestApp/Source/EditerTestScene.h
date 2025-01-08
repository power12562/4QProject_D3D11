#pragma once
#include <Scene/Base/Scene.h>

class EditerTestScene : public Scene
{
public:
	EditerTestScene() = default;
	virtual ~EditerTestScene() override = default;
	virtual void Start() override;

	virtual void ImGUIRender();
};