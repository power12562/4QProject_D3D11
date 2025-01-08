#pragma once

#include "RendererCore.h"
#include <memory>
#include <string_view>
#include <functional>
#include "DrawCommand.h"

class IRenderer abstract
{
public:
	virtual void AddDrawCommand(_In_ DrawCommand& command) = 0;
	virtual void SetRenderTarget(_In_ Texture& target) = 0;
	virtual void Render() = 0;
};

std::unique_ptr<IRenderer>& operator<<(std::unique_ptr<IRenderer>& renderer, const DrawCommand& drawCommand);
std::unique_ptr<IRenderer>& operator<<(std::unique_ptr<IRenderer>& renderer, Texture& drawCommand);






