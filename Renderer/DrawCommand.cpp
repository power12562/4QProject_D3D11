#include "DrawCommand.h"
#include <d3d11.h>

MeshDrawCommand::MeshDrawCommand() = default;
MeshDrawCommand::~MeshDrawCommand() = default;

template<>
void MeshDrawCommand::SetShaderResource<MeshData>(_In_ const MeshData& resource)
{
	meshData = resource;
}

template<>
void MeshDrawCommand::SetShaderResource<MaterialData>(_In_ const MaterialData& resource)
{
	materialData = resource;
}



