#include "Renderer.h"
#include <map>
#include <string>
#include <d3d11.h>

std::unique_ptr<IRenderer>& operator<<(std::unique_ptr<IRenderer>& renderer, MeshDrawCommand& drawCommand)
{
	renderer->AddDrawCommand(drawCommand);
	return renderer;
}

std::unique_ptr<IRenderer>& operator<<(std::unique_ptr<IRenderer>& renderer, Texture& drawCommand)
{
	renderer->SetRenderTarget(drawCommand);
	return renderer;
}

