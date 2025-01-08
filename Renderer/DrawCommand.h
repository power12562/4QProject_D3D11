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
	Shader vertexShader;

	DirectX::BoundingBox boundingBox;
	ConstantBuffer transformBuffer;
};

struct MaterialData
{
	std::vector<ConstantBuffer> shaderResources;
	std::vector<Texture> texture;
	Shader pixelShader;
};


struct DrawCommand
{
public:
	DrawCommand();
	~DrawCommand();

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
void DrawCommand::SetShaderResource<MeshData>(_In_ const MeshData& resource);

template<>
void DrawCommand::SetShaderResource<MaterialData>(_In_ const MaterialData& resource);

template<typename T>
DrawCommand& operator<<(DrawCommand& drawCommand, const T& meshBuffer)
{
	drawCommand.SetShaderResource(meshBuffer);
	return drawCommand;
}