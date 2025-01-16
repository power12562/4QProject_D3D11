#pragma once
#include <directxtk/simplemath.h>
#include <memory>

using CBbool = int;
struct alignas(16) cb_PBRMaterial
{
    DirectX::SimpleMath::Color Albedo{1,1,1,1};
    float Metalness{};
    float Roughness{};

    CBbool UseMetalnessMap = false;
    CBbool UseRoughnessMap = false;
    CBbool UseRMACMap = false;
};