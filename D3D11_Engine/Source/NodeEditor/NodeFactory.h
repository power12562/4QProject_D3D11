#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImNodeFlow.h>
#include <string>
#include <map>


class ShaderNode;
class NodeFlow;

class ShaderNodeFactory
{
public:
	ShaderNodeFactory();
	~ShaderNodeFactory();

public:
	void Set(NodeFlow* grid);
	std::shared_ptr<ShaderNode> Create(std::string_view type);

private:
	NodeFlow* myGrid;
	std::map<std::string, std::function<std::shared_ptr<ShaderNode>(ImFlow::ImNodeFlow*)>> nodeCreateFuncs;
};


