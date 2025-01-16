#pragma once
#include <Utility/D3D11Utility.h>

struct alignas(16) cb_PBRDirectionalLight
{
    static constexpr unsigned int MAX_LIGHT_COUNT = 4;

    struct alignas(16)
    {
        DirectX::SimpleMath::Vector4 LightColor{ 1, 1, 1, 1 };
        DirectX::SimpleMath::Vector3 LightDir{ 0.f, -1.f, 1.f };
        float LightIntensity = 5.f;
    }
    Lights[MAX_LIGHT_COUNT]{};
    int LightsCount = 1;

    void PushLight();
    void PopLight();
};
