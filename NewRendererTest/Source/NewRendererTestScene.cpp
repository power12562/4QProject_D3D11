#include "NewRendererTestScene.h"
#include <framework.h>

void NewRendererTestScene::Start()
{
	Scene::GuizmoSetting.UseImGuizmo = true;
	UseImGUI = true;

	auto mainCamera = NewGameObject<CameraObject>(L"MainCamera");
	mainCamera->SetMainCamera();
	mainCamera->transform.position = Vector3(0, 0, -5);
	mainCamera->AddComponent<CameraMoveHelper>();

	auto cube = NewGameObject<CubeObject>(L"Cube");
}

void NewRendererTestScene::ImGUIRender()
{
	ImGui::Begin("Hierarchy");
	ImGui::EditHierarchyView();
	ImGui::End();

	ImGui::Begin("Light");
	{
		ImGui::EditLight(DirectionalLight::DirectionalLights.get());
	}
	ImGui::End();
}
