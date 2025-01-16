#pragma once
#include <Utility/D3D11Utility.h>

#pragma warning(disable : 4305)
struct alignas(16) cb_DirectionalLight
{
    DirectX::SimpleMath::Vector4 LightColor{ 1, 1, 1, 1 };
    DirectX::SimpleMath::Vector4 LightAmbient{ 0.02f, 0.02f, 0.02f, 0.f };
    DirectX::SimpleMath::Vector4 LightSpecular{ 1, 1, 1, 1 };
    DirectX::SimpleMath::Vector3 LightDir{ 0.f, -1.f, 1.f};
};
#pragma warning(default : 4305)

