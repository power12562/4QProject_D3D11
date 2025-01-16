#include "RendererCore.h"
#include <d3d11.h>
#include <format>
#include <dxgi1_2.h>

consteval bool IsDebug()
{
#ifdef _DEBUG
	return true;
#endif // _DEBUG
	return false;
}

std::atomic_int RendererUtility::deviceCount{ 0 };
std::atomic_int RendererUtility::swapChainCount{ 0 };
ComPtr<ID3D11Device> RendererUtility::device{ nullptr };
ComPtr<IDXGISwapChain1> RendererUtility::swapChain{ nullptr };

void RendererUtility::SetDevice(ComPtr<ID3D11Device> newDevice)
{
	if (newDevice)
	{
		if (device == newDevice)
		{
			++deviceCount;
		}
		else
		{
			device = newDevice;
		}
	}
	else
	{
		if (--deviceCount == 0)
		{
			device.Reset();
		}
	}
}

ID3D11Device* RendererUtility::GetDevice()
{
	return device.Get();
}

void RendererUtility::SetSwapChain(ComPtr<struct IDXGISwapChain1> swapChain)
{
	RendererUtility::swapChain = swapChain;
}

ComPtr<struct IDXGISwapChain1> RendererUtility::GetSwapChain()
{
	return swapChain;
}

void Check::operator()(HRESULT result)
{
	if (!SUCCEEDED(result))
	{
		MessageBoxA(NULL, std::format("errID : {}", result).c_str(), "Error", MB_OK);
		if constexpr (IsDebug())
		{
			__debugbreak();
		}
	}
}
