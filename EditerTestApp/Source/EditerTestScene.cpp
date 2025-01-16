#include <framework.h>
#include "EditerTestScene.h"
#include "../EditerTestApp/PhysicsTestController.h"

void EditerTestScene::Start()
{
	Scene::GuizmoSetting.UseImGuizmo = true;
	UseImGUI = true;

	auto mainCamera = NewGameObject<CameraObject>(L"MainCamera");
	mainCamera->SetMainCamera();
	mainCamera->transform.position = Vector3(0, 0, -5);
	mainCamera->AddComponent<CameraMoveHelper>();

	auto cube = NewGameObject<CubeObject>(L"Cube");
	cube->AddComponent<PhysicsTestController>();

	auto ground = NewGameObject<CubeObject>(L"Ground");
	ground->AddComponent<BoxCollider>();
	ground->transform.SetPosition({ 0, -10, 0 });
	ground->transform.SetScale({ 10, 1, 10 });
}

void EditerTestScene::ImGUIRender()
{
	ImGui::Begin("Hierarchy");
	ImGui::EditHierarchyView();
	ImGui::End();

	ImGui::Begin("Light");
	{
		//ImGui::EditLight(DirectionalLight::DirectionalLights.get());
	}
	ImGui::End();
}
