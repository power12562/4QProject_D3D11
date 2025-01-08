#pragma once
#include <Utility/D3D11Utility.h>
#include <assimp/scene.h>
#include <assimp/matrix4x4.h>

using namespace DirectX;
namespace AssimpMath
{
    void DecomposeTransform(const aiNode* node, SimpleMath::Vector3& position, SimpleMath::Quaternion& rotation, SimpleMath::Vector3& scale);
}

