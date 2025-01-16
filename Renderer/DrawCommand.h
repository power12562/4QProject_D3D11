#pragma once

#include "RendererCore.h"
#include "ShaderResource.h"
#include "ConstantBuffer.h"
#include "MeshBuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "RendererBuffer.h"
#include "SamplerState.h"
#include <Directxtk/SimpleMath.h>
#include <vector>

struct MeshData
{
	RendererBuffer vertexBuffer;
	RendererBuffer indexBuffer;
	uint32_t indexCounts;
	uint32_t vertexStride;
	VertexShader vertexShader;
	std::vector<Binadble> shaderResources;

	DirectX::BoundingOrientedBox boundingBox;
};

struct MaterialData
{
	PixelShader pixelShader;
	std::vector<Binadble> shaderResources;
};


struct MeshDrawCommand
{
public:
	MeshDrawCommand();
	~MeshDrawCommand();

public:
	MeshData meshData;
	MaterialData materialData;
};

struct SkyBoxMeshDrawCommand : public MeshDrawCommand
{
};

struct PostProcessMeshDrawCommand
{
	ComputeShader computeShader;
};
