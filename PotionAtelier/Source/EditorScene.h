#pragma once
#include <Scene/Base/Scene.h>

class EditorScene : public Scene
{
public:
	EditorScene() = default;
	virtual ~EditorScene() override = default;
	virtual void Start() override;

	virtual void ImGUIRender();
};