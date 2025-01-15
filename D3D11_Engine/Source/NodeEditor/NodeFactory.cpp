#include "NodeFactory.h"

#define ADD_FACTORY_NODE(type)										\
nodeCreateFuncs[#type] = 											\
[](ImFlow::ImNodeFlow* nodeFlow) -> std::shared_ptr<ShaderNode>		\
	{ 																\
		return nodeFlow->placeNode<type>(); 						\
	}																\

ShaderNodeFactory::ShaderNodeFactory()
{
	ADD_FACTORY_NODE(ConstantValueNode);
	ADD_FACTORY_NODE(ConstantVector2Node);
	ADD_FACTORY_NODE(ConstantVector3Node);
	ADD_FACTORY_NODE(ConstantVector4Node);
}

ShaderNodeFactory::~ShaderNodeFactory() = default;

void ShaderNodeFactory::Set(std::shared_ptr<ImFlow::ImNodeFlow> grid)
{
	myGrid = grid;
}

std::shared_ptr<ShaderNode> ShaderNodeFactory::Create(std::string_view type)
{
	return nodeCreateFuncs[type.data()](myGrid.get());
}
