#pragma once

#include "RendererCore.h"
#include "ShaderResource.h"
#include "ConstantBuffer.h"
#include "MeshBuffer.h"
#include "Shader.h"
#include "Texture.h"
#include "RendererBuffer.h"
#include <Directxtk/SimpleMath.h>
#include <vector>

struct MeshData
{
	RendererBuffer vertexBuffer;
	RendererBuffer indexBuffer;
	uint32_t indexCounts;
	uint32_t vertexStride;
	VertexShader vertexShader;

	DirectX::BoundingBox boundingBox;
	ConstantBuffer transformBuffer;
};

struct MaterialData
{
	std::vector<ConstantBuffer> shaderResources;
	std::vector<Texture> texture;
	PixelShader pixelShader;
};


struct MeshDrawCommand
{
public:
	MeshDrawCommand();
	~MeshDrawCommand();

public:
	template<typename T>
	void SetShaderResource(_In_ const T& resource)
	{
		static_assert(0);
	}

	const MeshData& GetMeshData() const { return meshData; }
	const MaterialData& GetMaterialData() const { return materialData; }


private:
	MeshData meshData;
	MaterialData materialData;
};

template<>
void MeshDrawCommand::SetShaderResource<MeshData>(_In_ const MeshData& resource);

template<>
void MeshDrawCommand::SetShaderResource<MaterialData>(_In_ const MaterialData& resource);

template<typename T>
MeshDrawCommand& operator<<(MeshDrawCommand& drawCommand, const T& meshBuffer)
{
	drawCommand.SetShaderResource(meshBuffer);
	return drawCommand;
}