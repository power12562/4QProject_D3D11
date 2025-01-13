#include "Renderer.h"
#include <map>
#include <string>
#include <d3d11.h>

std::unique_ptr<IRenderer>& operator<<(std::unique_ptr<IRenderer>& renderer, const MeshDrawCommand& drawCommand)
{
	renderer->AddDrawCommand(drawCommand);
	return renderer;
}
