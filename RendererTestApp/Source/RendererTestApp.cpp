﻿#include "RendererTestApp.h"

#include <dxgi1_6.h>
#pragma comment(lib, "dxgi")
#include <framework.h>

RendererTestApp::RendererTestApp()
{
    this->windowName = L"Renderer Test App";
    //this->SetBorderlessWindowed();
    //this->SetOptimalScreenSize();
}

RendererTestApp::~RendererTestApp() = default;

void RendererTestApp::Start()
{
    renderer = std::make_unique<DefferdRenderer>();
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

}

void RendererTestApp::Render()
{
    renderer << Binadble { EShaderType::Pixel, EShaderBindable::ShaderResource, 30, (ID3D11ShaderResourceView*)BRDF_LUT };
    renderer << Binadble { EShaderType::Pixel, EShaderBindable::ShaderResource, 31, (ID3D11ShaderResourceView*)Diffuse_IBL };
    renderer << Binadble { EShaderType::Pixel, EShaderBindable::ShaderResource, 32, (ID3D11ShaderResourceView*)Specular_IBL };

	renderer->SetProjection(Mathf::PI / 4, 0.1f, 1000.0f);
	renderer->SetCameraMatrix(DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixLookAtLH(Vector3(-2, 2, -5), Vector3(0, 0, 0), Vector3::Up)));
    //renderer->SetCameraMatricx(Matrix::CreateTranslation(Vector3(0, 0, 10)));
    renderer << testObject->GetComponent<CubeMeshRender>().GetMeshDrawCommand();

	renderer->Render();

	swapChain->Present(0, 0);
}

void RendererTestApp::Uninitialize()
{
	BRDF_LUT.~Texture();
	Diffuse_IBL.~Texture();
	Specular_IBL.~Texture();

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
    testObject = NewGameObject<CubeObject>(L"Cube");
    testObject->GetComponent<CubeMeshRender>().SetPixelShader(L"Resource/EngineShader/CubeShader.hlsl");
    testObject->GetComponent<CubeMeshRender>().SetPixelShader(L"Resource/Shader/TEst.hlsl");

	ComPtr<ID3D11ShaderResourceView> srv;
	textureManager.CreateSharingTexture(L"Resource/Texture/IBL/Brdf.dds", &srv);
	BRDF_LUT.LoadTexture(srv.Get());

	textureManager.CreateSharingCubeMap(L"Resource/Texture/IBL/DiffuseIBL.dds", &srv);
	Diffuse_IBL.LoadTexture(srv.Get());

	textureManager.CreateSharingCubeMap(L"Resource/Texture/IBL/SpecularIBL.dds", &srv);
	Specular_IBL.LoadTexture(srv.Get());

    //auto aa = Utility::CreateCompressTexture(d3dRenderer.GetDevice(), L"Resource/Texture/IBL/EnvHDR.dds", nullptr, &srv, Utility::E_COMPRESS::BC6);
	//Utility::SaveTextureForDDS(L"Resource/Texture/IBL/EnvHDR2.dds", aa);

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
