#include "WinGameApp.h"
#include <GameObject/Base/GameObject.h>
#include <D3DCore/D3DRenderer.h>
#include <Core/TimeSystem.h>
#include <Manager/SceneManager.h>
#include <cassert>
#include <cstdio>
#include <clocale>

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>
#include <Utility/ImguiHelper.h>
#include <Math/Mathf.h>
#include <Utility/Console.h>
#include <Utility/ExceptionUtility.h>

LRESULT CALLBACK DefaultWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void WinGameApp::Initialize(HINSTANCE hinstance)
{
	SetUnhandledExceptionFilter(CustomUnhandledExceptionFilter);

	if (RunApp != nullptr)
	{
		__debugbreak(); //실행중 초기화 불가능.
		return;
	}
		
	setlocale(LC_ALL, "");
#ifdef _DEBUG
	AllocConsole();
	FILE* _tempFile;
	freopen_s(&_tempFile, "CONOUT$", "w", stdout);
#endif // _DEBUG
	WinInit(hinstance);
	d3dRenderer.Init();
	InitImGUI();
}

void WinGameApp::Run()
{
	try
	{
		using namespace TimeSystem;
		Time.UpdateTime();
		Start();
		// PeekMessage 메세지가 있으면 true,없으면 false
		while (!isEnd)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					break;

				TranslateMessage(&msg); // 키입력관련 메시지 변환  WM_KEYDOWN -> WM_CHAR
				DispatchMessage(&msg);
			}
			else
			{
				Time.UpdateTime();
				Update();
				Render();
			}
		}
	}
	catch (const ObjectException& ex)
	{
		const char* what = ex.what();
		Debug_printf("%s\n", what);
		Debug_wprintf(L"Object : %s\n", ex.GetGameObject()->Name.c_str());
		Debug_printf("Flie : %s\n", ex.GetFileName());
		Debug_printf("Func : %s\n", ex.GetFuncName());
		Debug_printf("Line : %d\n", ex.GetLineNum());
		__debugbreak(); //예외 발생
		sceneManager.currScene.reset();
		sceneManager.nextScene.reset();
	}
	catch (const std::exception& ex)
	{
		const char* what = ex.what();
		Debug_printf("%s", what);
		__debugbreak(); //예외 발생
	
		sceneManager.currScene.reset();
		sceneManager.nextScene.reset();
	}
}

void WinGameApp::Uninitialize()
{
#ifdef _DEBUG
	FreeConsole();
#endif // _DEBUG
	UninitImGUI();
	d3dRenderer.Uninit();
}

void WinGameApp::ClampScreenMaxSize(SIZE& size)
{
	static const SIZE maxScreenSize = { GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN) };
	if (size.cx <= 0 || 0 >= size.cy ||
		size.cx > maxScreenSize.cx || maxScreenSize.cy < size.cy)
	{
		size = maxScreenSize;
	}
}

void WinGameApp::ClampClientMaxSize(SIZE& size)
{
	if (RunApp)
	{
		SIZE maxClientSize = RunApp->clientSize;
		if (size.cx <= 0 || 0 >= size.cy ||
			size.cx > maxClientSize.cx || maxClientSize.cy < size.cy)
		{
			size = maxClientSize;
		}
	}
}

void WinGameApp::WinToScreenCenter(HWND hwnd)
{
	int x, y, width, height;
	RECT rtDesk, rtWindow;
	GetWindowRect(GetDesktopWindow(), &rtDesk);
	GetWindowRect(hwnd, &rtWindow);
	width = rtWindow.right - rtWindow.left;
	height = rtWindow.bottom - rtWindow.top;
	x = (rtDesk.right - width) / 2;
	y = (rtDesk.bottom - height) / 2;
	MoveWindow(hwnd, x, y, width, height, FALSE);
}

void WinGameApp::WinClientResize(HWND hwnd, int width, int height)
{
	SIZE size{ width , height };
	RECT clientRect = { 0, 0, width, height };
	AdjustWindowRect(&clientRect, d3dRenderer.swapChainWindowed ? RunApp->windowStyleEX : WS_POPUP, FALSE);
	SIZE windowSize{ clientRect.right - clientRect.left , clientRect.bottom - clientRect.top };
	SIZE windowClientOffset{ windowSize.cx - size.cx, windowSize.cy - size.cy };
	ClampScreenMaxSize(windowSize);
	size.cx = windowSize.cx - windowClientOffset.cx;
	size.cy = windowSize.cy - windowClientOffset.cy;
	RunApp->clientSize = size;

	int x, y;
	RECT rtDesk, rtWindow;
	GetWindowRect(GetDesktopWindow(), &rtDesk);
	GetWindowRect(hwnd, &rtWindow);
	x = (rtDesk.right - width) / 2;
	y = (rtDesk.bottom - height) / 2;
	MoveWindow(hwnd, x, y, (int)windowSize.cx, (int)windowSize.cy, FALSE);
}

void WinGameApp::GameEnd()
{
	if (RunApp)
	{
		RunApp->isEnd = true;
		RunApp = nullptr;
	}
}

int WinGameApp::GetFPS()
{
	if (RunApp)
	{
		TimeSystem::Time.GetFrameRate();
	}
	return -1;
}

bool WinGameApp::WinInit(HINSTANCE hInstance)
{
	if (RunApp)
	{
		__debugbreak(); //2개의 앱 루프 실행 불가.
		return false;
	}
	RunApp = this;

	//static hInstance
	if (WinGameApp::hInstance == NULL)
	{
		WinGameApp::hInstance = hInstance;
	}								
								  
	// 윈도우 클래스 구조체 초기화
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = winClassStyle;
	if (customWndProc != nullptr)
	{	
		wc.lpfnWndProc = customWndProc;
	}
	else
	{
		wc.lpfnWndProc = DefaultWndProc;	// 기본 윈도우 프로시저 함수
	}	
	wc.hInstance = hInstance;   // 인스턴스 핸들
	// NULL을 사용하면 현재 실행 중인 프로그램의 인스턴스 핸들을 사용하게 됩니다.두 번째 인자는 로드할 커서의 ID입니다.
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszClassName = winClassName;

	// 윈도우 클래스 등록
	if (!RegisterClassEx(&wc))
	{
		int error = GetLastError();
		wchar_t errorStr[25]{};
		swprintf_s(errorStr, L"윈도우 클래스 등록 실패. 에러 : %d", error);

		MessageBox(NULL, errorStr, L"에러", MB_OK | MB_ICONERROR);
		isEnd = false;
		return false;
	}

	ClampScreenMaxSize(clientSize);
	RECT clientRect = { 0, 0, clientSize.cx, clientSize.cy };
	AdjustWindowRect(&clientRect, windowStyleEX, FALSE); 	
	SIZE windowSize{ clientRect.right - clientRect.left , clientRect.bottom - clientRect.top };
	SIZE windowClientOffset{ windowSize.cx - clientSize.cx, windowSize.cy - clientSize.cy };
	ClampScreenMaxSize(windowSize);
	clientSize.cx = windowSize.cx - windowClientOffset.cx;
	clientSize.cy = windowSize.cy - windowClientOffset.cy;
	// 윈도우 생성
	WinGameApp::hwnd = CreateWindowEx(
		0,
		winClassName,
		windowName,
		windowStyleEX,
		0, 0,
		windowSize.cx, windowSize.cy,
		NULL, NULL, hInstance, NULL
	);

	if (!hwnd)
	{
		int error = GetLastError();
		wchar_t errorStr[25]{};

		swprintf_s(errorStr, L"윈도우 생성 실패. 에러 : %d", error);
		MessageBox(NULL, errorStr, L"에러", MB_OK | MB_ICONERROR);
		isEnd = false;
		return false;
	}

	// 윈도우 표시
	ShowWindow(hwnd, 10);
	UpdateWindow(hwnd);
	WinToScreenCenter(hwnd);

#ifdef _DEBUG
		RECT rc;
		GetClientRect(hwnd, &rc);
		int sx = rc.right - rc.left;
		int sy = rc.bottom - rc.top;
		printf("ClientSize : %d, %d\n", sx, sy);
#endif // _DEBUG

	return true;
}

void WinGameApp::InitImGUI()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(GetHWND());
	ImGui_ImplDX11_Init(d3dRenderer.GetDevice(), d3dRenderer.GetDeviceContext());
}

void WinGameApp::UninitImGUI()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

//
//  함수: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  용도: 주 창의 메시지를 처리합니다.
//  WM_DESTROY  - 종료 메시지를 게시하고 반환합니다.
//
//
LRESULT CALLBACK DefaultWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}