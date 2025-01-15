#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImNodeFlow.h>
#include <string>
#include <map>
#include "ShaderNodes.h"


class ShaderNodeFactory
{
public:
	ShaderNodeFactory();
	~ShaderNodeFactory();

public:
	void Set(std::shared_ptr<ImFlow::ImNodeFlow> grid);
	std::shared_ptr<ShaderNode> Create(std::string_view type);

private:
	std::shared_ptr<ImFlow::ImNodeFlow> myGrid;
	std::map<std::string, std::function<std::shared_ptr<ShaderNode>(ImFlow::ImNodeFlow*)>> nodeCreateFuncs;
};


