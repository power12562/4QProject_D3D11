#pragma once

#include "RendererCore.h"
#include "ShaderResource.h"
#include "RendererBuffer.h"
#include <Directxtk/SimpleMath.h>

struct MeshBuffer : public ShaderResource
{
public:
	MeshBuffer();
	~MeshBuffer();

public:
	RendererBuffer vertexBuffer;
	RendererBuffer indexBuffer;
	uint32_t vertexStride;
	uint32_t indexCount;
	DirectX::BoundingBox boundingBox;
};