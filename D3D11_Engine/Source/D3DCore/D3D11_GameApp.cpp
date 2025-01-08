#include "D3D11_GameApp.h"
#include <Manager/SceneManager.h>
#include <Core/TimeSystem.h>
#include <D3DCore/D3DRenderer.h>
#include <GameObject\Base\CameraObject.h>
#include <Core/DXTKInputSystem.h>
#include <Utility/ImguiHelper.h>
#include <Manager/ResourceManager.h>
#include <Light/PBRDirectionalLight.h>
#include <Light/SimpleDirectionalLight.h>
#include <D3DCore/D3DConstBuffer.h>

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
			if (!d3dRenderer.IsSwapChainWindowed() && !sceneManager.EndGame)
			{
				d3dRenderer.ToggleFullscreenMode();
			}
		}
		Mouse::ProcessMessage(message, wParam, lParam);
		Keyboard::ProcessMessage(message, wParam, lParam);
		break;
	case WM_ACTIVATEAPP:
		Mouse::ProcessMessage(message, wParam, lParam);
		Keyboard::ProcessMessage(message, wParam, lParam);
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
			d3dRenderer.ToggleFullscreenMode(); //풀스크린 처리
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

void D3D11_GameApp::GameEnd()
{
	sceneManager.EndGame = true;
}

void D3D11_GameApp::ChangeResolution(SIZE resize)
{
	if (D3D11_GameApp* app = static_cast<D3D11_GameApp*>(RunApp))
	{
		D3D11_GameApp::ClampScreenMaxSize(resize);
		WinClientResize(GetHWND(), (int)resize.cx, (int)resize.cy);
		DXGI_SWAP_CHAIN_DESC1 desc{};
		Utility::CheckHRESULT(d3dRenderer.GetSwapChain()->GetDesc1(&desc));
		desc.Width = app->clientSize.cx;
		desc.Height = app->clientSize.cy;
		d3dRenderer.ReCreateSwapChain(&desc);
	}
}

D3D11_GameApp::D3D11_GameApp()
{
	this->customWndProc = ImGUIWndProcDefault;
}

D3D11_GameApp::~D3D11_GameApp()
{

}

void D3D11_GameApp::Start()
{
	D3D11_GameApp::RunApp = this;

	SimpleDirectionalLight::cb_light =    D3DConstBuffer::GetData<cb_DirectionalLight>(SimpleDirectionalLight::cb_light_key);
	DirectionalLight::DirectionalLights = D3DConstBuffer::GetData<cb_PBRDirectionalLight>(DirectionalLight::DirectionalLights_key);

	DXTKinputSystem.Initialize(GetHWND());
	gameObjectFactory.InitializeMemoryPool();

	if (sceneManager.nextScene == nullptr)
		sceneManager.LoadScene<Scene>(); //빈 씬 로드

	sceneManager.ChangeScene();
	sceneManager.AddObjects();
}

void D3D11_GameApp::Update()
{
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

	Keyboard::ProcessMessage(message, wParam, lParam);
}

void D3D11_GameApp::SetBorderlessWindowed()
{
	windowStyleEX = WS_POPUP;
}

void D3D11_GameApp::SetOptimalScreenSize()
{
	clientSize = { 0, 0 };
}