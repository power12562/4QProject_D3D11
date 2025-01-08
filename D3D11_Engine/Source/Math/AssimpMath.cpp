#include "AssimpMath.h"
#include <assert.h>

using namespace SimpleMath;

void AssimpMath::DecomposeTransform(const aiNode* node, Vector3& position, Quaternion& rotation, Vector3& scale)
{
    // Assimp의 aiMatrix4x4를 DirectX의 XMMATRIX로 변환
    aiMatrix4x4 aiTransform = node->mTransformation;

    Matrix dxTransform = Matrix(&aiTransform.a1).Transpose();
    bool decompose_result = dxTransform.Decompose(scale, rotation, position);
    assert(decompose_result && "Fail decomposeTransform");
}