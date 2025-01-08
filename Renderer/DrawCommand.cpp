#include "DrawCommand.h"
#include <d3d11.h>

DrawCommand::DrawCommand() = default;
DrawCommand::~DrawCommand() = default;

template<>
void DrawCommand::SetShaderResource<MeshData>(_In_ const MeshData& resource)
{
	meshData = resource;
}

template<>
void DrawCommand::SetShaderResource<MaterialData>(_In_ const MaterialData& resource)
{
	materialData = resource;
}



