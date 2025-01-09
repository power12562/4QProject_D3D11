#pragma once

#include "RendererCore.h"
#include <memory>
#include <string_view>
#include <functional>
#include "DrawCommand.h"

class IRenderer abstract
{
public:
	virtual ~IRenderer() {}

public:
	virtual void AddDrawCommand(_In_ const MeshDrawCommand& command) = 0;
	virtual void SetRenderTarget(_In_ Texture& target) = 0;
	virtual void Render() = 0;

	virtual void SetCameraMatrix(const Matrix& world) = 0;
	virtual void SetProjection(float fov, float nearZ, float farZ) = 0;
};

std::unique_ptr<IRenderer>& operator<<(std::unique_ptr<IRenderer>& renderer, const MeshDrawCommand& drawCommand);
std::unique_ptr<IRenderer>& operator<<(std::unique_ptr<IRenderer>& renderer, Texture& drawCommand);






