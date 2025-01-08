#include "RendererCore.h"
#include <d3d11.h>
#include <format>

consteval bool IsDebug()
{
#ifdef _DEBUG
	return true;
#endif // _DEBUG
	return false;
}

ComPtr<ID3D11Device> RendererUtility::device{ nullptr };

void RendererUtility::SetDevice(ComPtr<ID3D11Device> newDevice)
{
	device = newDevice;
}

ID3D11Device* RendererUtility::GetDevice()
{
	return device.Get();
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
