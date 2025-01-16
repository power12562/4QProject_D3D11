#include "D3D11_GameApp.h"
#include <Manager/SceneManager.h>
#include <Core/TimeSystem.h>
#include <GameObject\Base\CameraObject.h>
#include <Core/DXTKInputSystem.h>
#include <Utility/ImguiHelper.h>
#include <Manager/ResourceManager.h>
#include <Light/PBRDirectionalLight.h>
#include <Light/SimpleDirectionalLight.h>
#include <Core/GameInputSystem.h>
#include <Manager/InputManager.h>
#include <Manager/HLSLManager.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void ClearGame()
{
	WinGameApp::GameEnd();
}

LRESULT CALLBACK ImGUIWndProcDefault(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;

	switch (message)
	{
#pragma region DXTKInputSystem 사용시 포함
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			
		}
		Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_ACTIVATEAPP:
		Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_XBUTTONDOWN:
	case WM_XBUTTONUP:
		D3D11_GameApp::ProcessMouse(message, wParam, lParam);
		break;
	case WM_INPUT:
	case WM_MOUSEMOVE:
	case WM_MOUSEHOVER:
	case WM_MOUSEWHEEL:
		Mouse::ProcessMessage(message, wParam, lParam);
		break;
	case WM_SYSKEYDOWN:
		if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000) // Alt + Enter 입력시
		{
			
		}
		D3D11_GameApp::ProcessKeyboard(message, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
		D3D11_GameApp::ProcessKeyboard(message, wParam, lParam);
		break;
	case WM_MENUCHAR:
		// A menu is active and the user presses a key that does not correspond
		// to any mnemonic or accelerator key. Ignore so we don't produce an error beep.
		return MAKELRESULT(0, MNC_CLOSE);
	case WM_MOUSEACTIVATE:
		// When you click to activate the window, we want Mouse to ignore that event.
		return MA_ACTIVATEANDEAT;
#pragma endregion 

	case WM_STYLECHANGED:
	{
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		sceneManager.currScene.reset();
		ClearGame();
		break;
	case WM_EXITSIZEMOVE: 
	{
		break;
	}
	case WM_SYSCOMMAND:
		if (wParam == SC_CLOSE)
		{
			sceneManager.EndGame = true;
		}
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void D3D11_GameApp::Present()
{
	RunApp->IDXGI.pSwapChain->Present(0, 0);
}

void D3D11_GameApp::GameEnd()
{
	sceneManager.EndGame = true;
}

void D3D11_GameApp::ChangeResolution(SIZE resize)
{
	if (D3D11_GameApp* app = static_cast<D3D11_GameApp*>(RunApp))
	{
		
	}
}

D3D11_GameApp::D3D11_GameApp()
{
	this->customWndProc = ImGUIWndProcDefault;
}

D3D11_GameApp::~D3D11_GameApp()
{

}

void D3D11_GameApp::Initialize(HINSTANCE hinstance)
{
	WinGameApp::Initialize(hinstance);
	inputManager.Initialize();
	InitMainRenderer();
	IDXGI.Init();
	RendererUtility::SetSwapChain(IDXGI.pSwapChain);

	ID3D11Device* pDevice = RendererUtility::GetDevice();
	ComPtr<ID3D11DeviceContext> pDeviceContext;
	pDevice->GetImmediateContext(&pDeviceContext);
	WinGameApp::InitImGUI(pDevice, pDeviceContext.Get());
}

void D3D11_GameApp::Uninitialize()
{
	WinGameApp::UninitImGUI();
	RendererUtility::SetSwapChain(nullptr);
	IDXGI.Uninit();
	UninitMainRenderer();
	inputManager.Uninitialize();
	WinGameApp::Uninitialize();
}

void D3D11_GameApp::Start()
{
	D3D11_GameApp::RunApp = this;

	gameObjectFactory.InitializeMemoryPool();

	if (sceneManager.nextScene == nullptr)
		sceneManager.LoadScene<Scene>(); //빈 씬 로드

	sceneManager.ChangeScene();
	sceneManager.AddObjects();
}

void D3D11_GameApp::Update()
{
	inputManager.UpdateProcesser();

	fixedElapsedTime += TimeSystem::Time.GetDeltaTime(false);
	while (fixedElapsedTime >= TimeSystem::FixedTimeStep)
	{		
		sceneManager.FixedUpdateScene();
		fixedElapsedTime -= TimeSystem::FixedTimeStep;
	}

	sceneManager.UpdateScene();
	sceneManager.LateUpdateScene();
}

void D3D11_GameApp::Render()
{
	sceneManager.RenderScene(); //씬 렌더링 함수.
	sceneManager.AddObjects();  //오브젝트 생성
	sceneManager.EraseObjects(); //오브젝트 삭제.
	sceneManager.ChangeScene(); //다음 씬 있으면 전환 
}

void D3D11_GameApp::ProcessMouse(UINT message, WPARAM wParam, LPARAM lParam)
{
	if(RunApp && wParam != 0)
		RunApp->PreProcessMouseMessage(message, static_cast<MouseVK>(wParam));

	Mouse::ProcessMessage(message, wParam, lParam);
}

void D3D11_GameApp::ProcessKeyboard(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (RunApp && wParam != 0)
		RunApp->PreProcessKeyboradMessage(message, static_cast<KeyboardVK>(wParam));
}

void D3D11_GameApp::SetBorderlessWindowed()
{
	windowStyleEX = WS_POPUP;
}

void D3D11_GameApp::SetOptimalScreenSize()
{
	clientSize = { 0, 0 };
}

void D3D11_GameApp::InitMainRenderer()
{
	MainRenderer = std::make_unique<DefferdRenderer>();
	MainRenderer->directLight.PushLight("Main Light", DirectionLightData
		{
			.Color = {1, 1, 1, 1},
			.Directoin = { 1, 1, 1, 1 }
		});

	{
		ComPtr<ID3D11ComputeShader> computeShader = nullptr;
		hlslManager.CreateSharingShader(L"Resource/EngineShader/DeferredRender.hlsl", &computeShader);
		MainRenderer->deferredCS.LoadShader(computeShader.Get());
	}
	{
		ComPtr<ID3D11PixelShader> pixelShader = nullptr;
		hlslManager.CreateSharingShader(L"Resource/EngineShader/DeferredRenderPS.hlsl", &pixelShader);
		MainRenderer->deferredPS.LoadShader(pixelShader.Get());
	}
	{
		ComPtr<ID3D11VertexShader> vertexShader = nullptr;
		ComPtr<ID3D11InputLayout> inputLayout = nullptr;
		hlslManager.CreateSharingShader(L"Resource/EngineShader/FullScreenTriangle.hlsl", &vertexShader, &inputLayout);
		MainRenderer->fullScreenVS.LoadShader(vertexShader.Get(), inputLayout.Get());
	}
}

void D3D11_GameApp::UninitMainRenderer()
{
	MainRenderer.reset();
}

void D3D11_GameApp::DXGI::Init()
{
	using namespace Utility;

	HRESULT result;

	//dxgi 디바이스
	result = CreateDXGIFactory1(__uuidof(IDXGIFactory4), (void**)&pDXGIFactory);
	Check(result);

	//모든 그래픽 카드 및 모니터 조사. 
	size_t i = 0;
	IDXGIAdapter* pAdapter = nullptr;
	while (pDXGIFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND) //그래픽 카드 정보)
	{
		result = pAdapter->QueryInterface(__uuidof(IDXGIAdapter3), reinterpret_cast<void**>(&pAdapter));
		Check(result);
		DXGIAdapters.push_back(reinterpret_cast<IDXGIAdapter3*>(pAdapter));
		DXGIOutputs.resize(i + 1);
		size_t j = 0;
		IDXGIOutput* pOutput = nullptr;
		while (pAdapter->EnumOutputs(j, &pOutput) != DXGI_ERROR_NOT_FOUND)
		{
			result = (pOutput->QueryInterface(__uuidof(IDXGIOutput1), reinterpret_cast<void**>(&pOutput)));
			Check(result);
			DXGIOutputs[i].push_back(reinterpret_cast<IDXGIOutput1*>(pOutput));
			++j;
		}
		++i;
	}

	DXGI_ADAPTER_DESC adapterDesc;
	for (auto& Adapters : DXGIAdapters)
	{
		Adapters->GetDesc(&adapterDesc);
		Debug_wprintf(L"%lu : %s\n", adapterDesc.DeviceId, adapterDesc.Description); //글카 이름
	}

	DXGI_OUTPUT_DESC outputDesc;
	DXGIOutputs[0][0]->GetDesc(&outputDesc); //메인 모니터
	if (outputDesc.AttachedToDesktop)
	{
		Debug_wprintf(L"Moniter 1 : %s\n", outputDesc.DeviceName);
	}

	CreateSwapChain();

	ComPtr<ID3D11Texture2D> backBufferTexture;
	result = pSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBufferTexture));
	Check(result);

	backBuffer = std::make_unique<Texture>();
	backBuffer->LoadTexture(backBufferTexture.Get(), ETextureUsage::RTV);

	MainRenderer->SetRenderTarget(*backBuffer);
}

void D3D11_GameApp::DXGI::Uninit()
{
	using namespace Utility;

	backBuffer.reset();
	SafeRelease(pSwapChain);
	for (auto& i : DXGIOutputs)
	{
		SafeReleaseArray(i);
	}
	SafeRelease(pDXGIFactory);
}

void D3D11_GameApp::DXGI::CreateSwapChain()
{
	using namespace Utility;
	ID3D11Device* pDevice = RendererUtility::GetDevice();
	HRESULT result;

	if (pSwapChain)
		return;

	DXGI_SWAP_CHAIN_DESC1 swapDesc{}; //스왑체인 속성 구조체  
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc{};
	swapDesc.BufferCount = 2; //버퍼 개수
	swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //버퍼 사용 방식 지정
	swapDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //출력 포멧 지정.
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; //플립 모드 사용.   
	swapDesc.Flags |= DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; //전체 화면 변환시 해상도 및 모니터 설정 자동 변경 플래그
	swapDesc.BufferCount = 1; //버퍼 개수
	swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; //플립 모드 사용.   

	//버퍼 사이즈 지정
	SIZE clientSize = WinGameApp::GetClientSize();
	swapDesc.Width = clientSize.cx;
	swapDesc.Height = clientSize.cy;

	//샘플링 설정 *(MSAA)
	swapDesc.SampleDesc.Count = 1;
	swapDesc.SampleDesc.Quality = 0;

	//0/0은 자동 설정임. 그리고 창모드는 어차피 적용 안됨...
	fullScreenDesc.Windowed = true; //창모드 유무
	fullScreenDesc.RefreshRate.Numerator = 0;
	fullScreenDesc.RefreshRate.Denominator = 0;

	UINT creationFlags = 0;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	result = pDXGIFactory->CreateSwapChainForHwnd(
		pDevice, WinGameApp::GetHWND(), &swapDesc, &fullScreenDesc, nullptr, &pSwapChain);
	Check(result);

	result = (pSwapChain->QueryInterface(IID_PPV_ARGS(&pSwapChain)));
	Check(result);
}

