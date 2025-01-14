#include "RendererTestApp.h"
#include <dxgi1_6.h>
#include <filesystem>
#pragma comment(lib, "dxgi")
#include <framework.h>
#include <ranges>

extern LRESULT CALLBACK ImGUIWndProcDefault(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
RendererTestApp::RendererTestApp()
{
    this->windowName = L"Renderer Test App";
    //this->SetBorderlessWindowed();
    //this->SetOptimalScreenSize();
    this->customWndProc = ImGUIWndProcDefault;
}

RendererTestApp::~RendererTestApp() = default;

void RendererTestApp::Start()
{
    renderer = std::make_unique<DefferdRenderer>();        
    {
        std::filesystem::path shaderPath = __FILEW__;
        shaderPath = shaderPath.parent_path().parent_path() / L"Resource/EngineShader/";
        ShaderUtility::CopyShader(shaderPath);
    }
    {
        ComPtr<ID3D11ComputeShader> computeShader = nullptr;
        hlslManager.CreateSharingShader(L"Resource/EngineShader/DeferredRender.hlsl", &computeShader);
        static_cast<DefferdRenderer*>(renderer.get())->deferredCS.LoadShader(computeShader.Get());
    }
    {
		ComPtr<ID3D11PixelShader> pixelShader = nullptr;
		hlslManager.CreateSharingShader(L"Resource/EngineShader/DeferredRenderPS.hlsl", &pixelShader);
		static_cast<DefferdRenderer*>(renderer.get())->deferredPS.LoadShader(pixelShader.Get());
	}
    {
        ComPtr<ID3D11VertexShader> vertexShader = nullptr;
        ComPtr<ID3D11InputLayout> inputLayout = nullptr;
        hlslManager.CreateSharingShader(L"Resource/EngineShader/FullScreenTriangle.hlsl", &vertexShader, &inputLayout);
        static_cast<DefferdRenderer*>(renderer.get())->fullScreenVS.LoadShader(vertexShader.Get(), inputLayout.Get());
    }

    DXGIInit();

    if (sceneManager.nextScene == nullptr)
        sceneManager.LoadScene<Scene>(); //빈 씬 로드

    sceneManager.ChangeScene();
    sceneManager.AddObjects();
    
    gameObjectFactory.InitializeMemoryPool();
	TestInit();

    renderer->SetRenderTarget(backBuffer);
}

void RendererTestApp::Update()
{
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Renderer Test App");
    if (ImGui::Button("Recompile Shader") || ImGui::IsKeyDown(ImGuiKey_F2))
    {
        {
            std::filesystem::path shaderPath = __FILEW__;
            shaderPath = shaderPath.parent_path().parent_path() / L"Resource/EngineShader/";
            ShaderUtility::CopyShader(shaderPath);
            MeshRender::ReloadShaderAll();
        }

        {
            ComPtr<ID3D11ComputeShader> computeShader = nullptr;
            hlslManager.CreateSharingShader(L"Resource/EngineShader/DeferredRender.hlsl", &computeShader);
            static_cast<DefferdRenderer*>(renderer.get())->deferredCS.LoadShader(computeShader.Get());
        }

        {
            ComPtr<ID3D11PixelShader> pixelShader = nullptr;
            hlslManager.CreateSharingShader(L"Resource/EngineShader/DeferredRenderPS.hlsl", &pixelShader);
            static_cast<DefferdRenderer*>(renderer.get())->deferredPS.LoadShader(pixelShader.Get());
        }
        {
            ComPtr<ID3D11VertexShader> vertexShader = nullptr;
            ComPtr<ID3D11InputLayout> inputLayout = nullptr;
            hlslManager.CreateSharingShader(L"Resource/EngineShader/FullScreenTriangle.hlsl", &vertexShader, &inputLayout);
            static_cast<DefferdRenderer*>(renderer.get())->fullScreenVS.LoadShader(vertexShader.Get(), inputLayout.Get());
        }
    }
    

    ImGui::BeginGroup();
    ImGui::EditTransform(testObject);
    ImGui::Checkbox("isForward", &testObject->GetComponent<CubeMeshRender>().isForward);
    ImGui::EndGroup();
    ImGui::BeginGroup();
    ImGui::EditTransform(testObject2);
    ImGui::EndGroup();
    ImGui::BeginGroup();
    ImGui::EditTransform(testObject3);
    ImGui::EndGroup();


	ImGui::BeginGroup();

    //빛 정보
	ImGui::Text("Direct Light");
    for (size_t i = 0; i < std::size(renderer->directLight); i++)
    {
        auto& lightData = renderer->directLight.GetLight(i);
        ImGui::ColorEdit4("light Color", &lightData.Color);
        ImGui::DragFloat3("light Direction", (float*)&lightData.Directoin, 0.01f, -1.f, 1.f);
		lightData.Directoin.Normalize();
    }

	ImGui::EndGroup();

    using clock = std::chrono::high_resolution_clock;
    static clock::time_point previousTime = clock::now();
    static clock::time_point currentTime = clock::now();
    static int count = 0;
    static int beforeCount = 0;
    static std::chrono::nanoseconds elpasTime{};
    double deltaTime;
    previousTime = currentTime;
    currentTime = clock::now();

    auto duration = currentTime - previousTime;
	elpasTime += duration;

    ++count;
    //1초
    if (elpasTime >= std::chrono::seconds(1))
    {
        elpasTime -= std::chrono::seconds(1);
        beforeCount = count;
        count = 0;
	}
	ImGui::Text("fps : %d", beforeCount);
    ImGui::End();

    Transform::UpdateMatrix();
    Transform::ClearUpdateList();
	
}

void RendererTestApp::Render()
{
	renderer->SetProjection(Mathf::PI / 4, 0.1f, 10000.0f);
	renderer->SetCameraMatrix(DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixLookAtLH(Vector3(-0, 0, -5), Vector3(0, 0, 0), Vector3::Up)));


	auto& testComponent = testObject->GetComponent<CubeMeshRender>();
    testComponent.UpdateMeshDrawCommand();
    renderer->AddDrawCommand(testComponent.GetMeshDrawCommand());

	for (auto& chara : renderList)
	{
		chara->UpdateMeshDrawCommand();
		renderer->AddDrawCommand(chara->GetMeshDrawCommand());
	}
   

	renderer->Render();


    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    ImGui::ResetGlobalID();

	swapChain->Present(0, 0);
}

void RendererTestApp::Uninitialize()
{
    sceneManager.AddObjects();
    sceneManager.currScene.reset();
    gameObjectFactory.UninitializeMemoryPool();

	backBuffer.~Texture();
	swapChain.ReleaseAndGetAddressOf();
	renderer.reset();
    RendererUtility::SetSwapChain(nullptr);

	WinGameApp::Uninitialize();
}

void RendererTestApp::DXGIInit()
{
    ID3D11Device* device = RendererUtility::GetDevice();
    swapChain = RendererUtility::GetSwapChain();

    
	HRESULT result;
    ComPtr<ID3D11Texture2D> backBufferTexture;

    result = swapChain->GetBuffer(0, IID_PPV_ARGS(&backBufferTexture));
    Check(result);

    backBuffer.LoadTexture(backBufferTexture.Get(), ETextureUsage::RTV);
}

#include <fstream>
#include <Utility\AssimpUtility.h>
#include "Manager\GameObjectFactory.h"

void RendererTestApp::TestInit()
{
    ComPtr<ID3D11ShaderResourceView> srv;


    testObject = NewGameObject<CubeObject>(L"Cube");
    testObject->GetComponent<CubeMeshRender>().SetPixelShader(L"Resource/Shader/TEst.hlsl");


	testObject2 = Utility::LoadFBX(L"Resource/char/char.fbx", false, SURFACE_TYPE::PBR);
    testObject2->transform.position = Vector3(-2.0f, 0.0f, 0.0f);
	testObject2->transform.scale = Vector3(0.01f, 0.01f, 0.01f);


    testObject3 = Utility::LoadFBX(L"Resource/char/char.fbx", false, SURFACE_TYPE::PBR);
    testObject3->transform.position = Vector3(2.0f, 0.0f, 0.0f);
    testObject3->transform.scale = Vector3(0.01f, 0.01f, 0.01f);

    auto pipe = std::views::transform([](auto& pair) { return pair.second; }) | std::ranges::views::join;

    std::ranges::copy(Utility::CollectMeshComponents(testObject2) | pipe, std::back_inserter(renderList));
    std::ranges::copy(Utility::CollectMeshComponents(testObject3) | pipe, std::back_inserter(renderList));

    Texture albedo;
    textureManager.CreateSharingTexture(L"Resource/Texture/1735656899.jpg", &srv);
    albedo.LoadTexture(srv.Get());

	testObject->GetComponent<CubeMeshRender>().texturesV2.emplace_back(albedo);
    testObject->GetComponent<CubeMeshRender>().texturesSlot.emplace_back(0);

    Texture BRDF_LUT;
    Texture Diffuse_IBL;
    Texture Specular_IBL;

	textureManager.CreateSharingTexture(L"Resource/Texture/IBL/Brdf.dds", &srv);
	BRDF_LUT.LoadTexture(srv.Get());

	textureManager.CreateSharingCubeMap(L"Resource/Texture/IBL/DiffuseIBL.dds", &srv);
	Diffuse_IBL.LoadTexture(srv.Get());

	textureManager.CreateSharingCubeMap(L"Resource/Texture/IBL/SpecularIBL.dds", &srv);
	Specular_IBL.LoadTexture(srv.Get());

    renderer->directLight.PushLight("1", 
                                    DirectionLightData
                                    {
                                        .Color = Vector4(1, 1, 1, 1), 
                                        .Directoin = Vector3(0, 0, -1),
										.Intensity = 1
                                    });

    
	
    renderer->AddBinadble("BRDF_LUT_PS", Binadble{ EShaderType::Pixel, EShaderBindable::ShaderResource, 30, (ID3D11ShaderResourceView*)BRDF_LUT });
    renderer->AddBinadble("Diffuse_IBL_PS", Binadble{ EShaderType::Pixel, EShaderBindable::ShaderResource, 31, (ID3D11ShaderResourceView*)Diffuse_IBL });
    renderer->AddBinadble("Specular_IBL_PS", Binadble{ EShaderType::Pixel, EShaderBindable::ShaderResource, 32, (ID3D11ShaderResourceView*)Specular_IBL });
   
    renderer->AddBinadble("BRDF_LUT_CS", Binadble{ EShaderType::Compute, EShaderBindable::ShaderResource, 30, (ID3D11ShaderResourceView*)BRDF_LUT });
    renderer->AddBinadble("Diffuse_IBL_CS", Binadble{ EShaderType::Compute, EShaderBindable::ShaderResource, 31, (ID3D11ShaderResourceView*)Diffuse_IBL });
    renderer->AddBinadble("Specular_IBL_CS", Binadble{ EShaderType::Compute, EShaderBindable::ShaderResource, 32, (ID3D11ShaderResourceView*)Specular_IBL });

    renderer->BRDF_LUT = BRDF_LUT;
    renderer->Diffuse_IBL = Diffuse_IBL;
    renderer->Specular_IBL = Specular_IBL;


    return;


    ShaderHeader header;
    ShaderHeader header2;

    std::ifstream headerFile("Resource/EngineShader/Shared.hlsli");
    if (headerFile.is_open())
    {
        std::string headerSource((std::istreambuf_iterator<char>(headerFile)), std::istreambuf_iterator<char>());
        header.Load("../EngineShader/Shared.hlsli", headerSource.c_str(), headerSource.size());
    }

    std::ifstream headerFile2("Resource/EngineShader/VSInput.hlsli");
	if (headerFile2.is_open())
	{
		std::string headerSource2((std::istreambuf_iterator<char>(headerFile2)), std::istreambuf_iterator<char>());
		header2.Load("VSInput.hlsli", headerSource2.c_str(), headerSource2.size());
	}

    std::ifstream file("Resource/EngineShader/VertexShader.hlsl");
    if (file.is_open())
    {
		std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
		vertexShader.Compile(source.c_str(), source.size());
    }

}