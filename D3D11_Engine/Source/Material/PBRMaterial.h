#pragma once
#include <directxtk/simplemath.h>
#include <D3DCore/D3DConstBuffer.h>
#include <memory>

struct alignas(16) cb_PBRMaterial
{
    DirectX::SimpleMath::Color Albedo{1,1,1,1};
    float Metalness{};
    float Roughness{};

    CBbool UseMetalnessMap = false;
    CBbool UseRoughnessMap = false;
    CBbool UseRMACMap = false;
};