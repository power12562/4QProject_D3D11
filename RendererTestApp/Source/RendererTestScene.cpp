#include "RendererTestScene.h"
#include <framework.h>
#include <ranges>

void RendererTestScene::Start()
{
	Scene::GuizmoSetting.UseImGuizmo = true;

	UseImGUI = true;

	auto mainCamera = NewGameObject<CameraObject>(L"MainCamera");
	mainCamera->SetMainCamera();
	mainCamera->transform.position = Vector3(0, 0, -5);
	mainCamera->AddComponent<CameraMoveHelper>();

	auto cube = NewGameObject<CubeObject>(L"Cube");
	auto skyBox = NewGameObject<SkyBoxObject>(L"skyBox");

	nodeEditor = std::make_unique<ShaderNodeEditor>();
	nodeEditor2 = std::make_unique<ShaderNodeEditor>("NodeEditor2");
	//nodeEditor->SetResultNode<TextureNode>("Resource/Texture/1735656899.jpg", EShaderResult::Normal);
	auto pipe = std::views::transform([](auto& pair) { return pair.second; }) | std::ranges::views::join;

	std::ranges::copy(Utility::CollectMeshComponents(cube) | pipe, std::back_inserter(meshList));
	for (auto& i : Utility::CollectMeshComponents(skyBox) | pipe)
	{
		SkyBoxRender* sky = dynamic_cast<SkyBoxRender*>(i);
		if (sky)
		{
			sky->SetSkyBox(SkyBoxRender::ENV, L"Resource/Texture/IBL/EnvHDR.dds");
			sky->SetSkyBox(SkyBoxRender::BRDF_LUT, L"Resource/Texture/IBL/Brdf.dds");
			sky->SetSkyBox(SkyBoxRender::Diffuse_IBL, L"Resource/Texture/IBL/DiffuseIBL.dds");
			sky->SetSkyBox(SkyBoxRender::Specular_IBL, L"Resource/Texture/IBL/SpecularIBL.dds");

		}
	}
	

	for (auto& i : meshList)
	{
		i->materialAsset.OpenAsset(L"Resource/Texture/Test.MaterialAsset");
	}

}

void RendererTestScene::ImGUIRender()
{
    nodeEditor->Update();
	nodeEditor2->Update();

	ImGui::Begin("Hierarchy");
	ImGui::EditHierarchyView();
	ImGui::End();


    ImGui::Begin("Renderer Test App");
    if (ImGui::Button("Recompile Shader") || ImGui::IsKeyDown(ImGuiKey_F2))
    {
        {
            std::filesystem::path shaderPath = __FILEW__;
            shaderPath = shaderPath.parent_path().parent_path() / L"Resource/EngineShader/";
            ShaderUtility::CopyShader(shaderPath);
            MeshRender::ReloadShaderAll();
			for (auto& i : meshList)
			{
				i->materialAsset.OpenAsset(i->materialAsset.GetAssetPath().c_str());
			}
        }

    }	


	ImGui::End();
}
