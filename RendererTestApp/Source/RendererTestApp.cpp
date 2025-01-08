#include "RendererTestApp.h"

#include <dxgi1_6.h>
#pragma comment(lib, "dxgi")

RendererTestApp::RendererTestApp()
{
    this->windowName = L"Renderer Test App";
    //this->SetBorderlessWindowed();
    //this->SetOptimalScreenSize();
}

RendererTestApp::~RendererTestApp()
{
    backBuffer.~Texture();
	swapChain.ReleaseAndGetAddressOf();
    renderer.reset();
}

void RendererTestApp::Start()
{
    renderer = std::make_unique<DefferdRenderer>();
    DXGIInit();
	TestInit();
}

void RendererTestApp::Update()
{

}

void RendererTestApp::Render()
{
    renderer << backBuffer;
	renderer->Render();

	swapChain->Present(0, 0);
}

void RendererTestApp::DXGIInit()
{
    ID3D11Device* device = RendererUtility::GetDevice();
    HRESULT result;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = clientSize.cx;
    swapChainDesc.Height = clientSize.cy;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    // 비트블릿 방식
    if (0)
    {
        swapChainDesc.BufferCount = 1;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    }

    ComPtr<IDXGIFactory4> pFactory = nullptr;
    result = CreateDXGIFactory2(0, IID_PPV_ARGS(&pFactory));
    Check(result);

    ComPtr<IDXGISwapChain1> swapChain1 = nullptr;
    result = pFactory->CreateSwapChainForHwnd(device,
        GetHWND(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain1);
    Check(result);

    result = swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain));
    Check(result);

    ComPtr<ID3D11Texture2D> backBufferTexture;

    result = swapChain->GetBuffer(0, IID_PPV_ARGS(&backBufferTexture));
    Check(result);

    backBuffer.LoadTexture(backBufferTexture.Get(), ETextureUsage::RTV);
}

#include <fstream>

void RendererTestApp::TestInit()
{

	std::ifstream file("RendererTestApp/Shader/VertexShader.hlsl");
    if (file.is_open())
    {
		std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		file.close();
		vertexShader.Compile(source.c_str(), source.size());
    }
}
