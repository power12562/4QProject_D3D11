#include "CubeMeshRender.h"
#include <Material/PBRMaterial.h>

void CubeMeshRender::Start()
{
	PBRMeshRender::Start();

    MeshID = 0;
    SetMeshResource(L"Cube");

    // 큐브의 정점 데이터
    vertices = {
        { { -1.0f,  1.0f, -1.0f, 1.0f }, {  0.0f,  0.0f, -1.0f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } }, // 0
        { {  1.0f,  1.0f, -1.0f, 1.0f }, {  0.0f,  0.0f, -1.0f }, {  1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } }, // 1
        { {  1.0f, -1.0f, -1.0f, 1.0f }, {  0.0f,  0.0f, -1.0f }, {  1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f } }, // 2
        { { -1.0f, -1.0f, -1.0f, 1.0f }, {  0.0f,  0.0f, -1.0f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } }, // 3

        // Back face
        { { -1.0f,  1.0f,  1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } }, // 4
        { {  1.0f,  1.0f,  1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } }, // 5
        { {  1.0f, -1.0f,  1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f } }, // 6
        { { -1.0f, -1.0f,  1.0f, 1.0f }, {  0.0f,  0.0f,  1.0f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } }, // 7

        // Top face
        { { -1.0f,  1.0f,  1.0f, 1.0f }, {  0.0f,  1.0f,  0.0f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } }, // 8
        { {  1.0f,  1.0f,  1.0f, 1.0f }, {  0.0f,  1.0f,  0.0f }, {  1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } }, // 9
        { {  1.0f,  1.0f, -1.0f, 1.0f }, {  0.0f,  1.0f,  0.0f }, {  1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f } }, // 10
        { { -1.0f,  1.0f, -1.0f, 1.0f }, {  0.0f,  1.0f,  0.0f }, {  1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } }, // 11

        // Bottom face
        { { -1.0f, -1.0f,  1.0f, 1.0f }, {  0.0f, -1.0f,  0.0f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 0.0f } }, // 12
        { {  1.0f, -1.0f,  1.0f, 1.0f }, {  0.0f, -1.0f,  0.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 0.0f } }, // 13
        { {  1.0f, -1.0f, -1.0f, 1.0f }, {  0.0f, -1.0f,  0.0f }, { -1.0f,  0.0f,  0.0f }, { 1.0f, 1.0f } }, // 14
        { { -1.0f, -1.0f, -1.0f, 1.0f }, {  0.0f, -1.0f,  0.0f }, { -1.0f,  0.0f,  0.0f }, { 0.0f, 1.0f } }, // 15

        // Left face
        { { -1.0f,  1.0f,  1.0f, 1.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 0.0f } }, // 16
        { { -1.0f,  1.0f, -1.0f, 1.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  1.0f }, { 1.0f, 0.0f } }, // 17
        { { -1.0f, -1.0f, -1.0f, 1.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  1.0f }, { 1.0f, 1.0f } }, // 18
        { { -1.0f, -1.0f,  1.0f, 1.0f }, { -1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f,  1.0f }, { 0.0f, 1.0f } }, // 19

        // Right face
        { {  1.0f,  1.0f,  1.0f, 1.0f }, {  1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 0.0f } }, // 20
        { {  1.0f,  1.0f, -1.0f, 1.0f }, {  1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 0.0f } }, // 21
        { {  1.0f, -1.0f, -1.0f, 1.0f }, {  1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f, -1.0f }, { 1.0f, 1.0f } }, // 22
        { {  1.0f, -1.0f,  1.0f, 1.0f }, {  1.0f,  0.0f,  0.0f }, {  0.0f,  0.0f, -1.0f }, { 0.0f, 1.0f } }, // 23
    };

    // 큐브의 인덱스 데이터
    indices = {
        0,  1,  2,  0,  2,  3,  // Front face
        4,  6,  5,  4,  7,  6,  // Back face
        8,  9, 10,  8, 10, 11,  // Top face
        12, 14, 13, 12, 15, 14, // Bottom face
        16, 18, 17, 16, 19, 18, // Left face
        20, 21, 22, 20, 22, 23  // Right face
    };

    CreateMesh();

    if (PBRMeshObject* object = dynamic_cast<PBRMeshObject*>(&gameObject))
    {
        object->Material.Albedo = { 0.39f, 0.39f, 0.39f, 1.f };
    }
}
