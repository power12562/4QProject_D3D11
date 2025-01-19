#include "NodeFactory.h"
#include "ShaderNodes.h"

#define ADD_FACTORY_NODE(type)										\
nodeCreateFuncs[#type] = 											\
[](ImFlow::ImNodeFlow* nodeFlow) -> std::shared_ptr<ShaderNode>		\
	{ 																\
		return std::dynamic_pointer_cast<ShaderNode>(nodeFlow->placeNode<type>()); 						\
	}																\

ShaderNodeFactory::ShaderNodeFactory()
{
	ADD_FACTORY_NODE(ConstantValueNode);
	ADD_FACTORY_NODE(ConstantVector2Node);
	ADD_FACTORY_NODE(ConstantVector3Node);
	ADD_FACTORY_NODE(ConstantVector4Node);
	ADD_FACTORY_NODE(TextureNode);
	ADD_FACTORY_NODE(AddNode);
	ADD_FACTORY_NODE(SubNode);
	ADD_FACTORY_NODE(MulNode);
	ADD_FACTORY_NODE(DivNode);
	ADD_FACTORY_NODE(TimeNode);
	ADD_FACTORY_NODE(TexCoordNode);
	ADD_FACTORY_NODE(MakeVector2Node);
	ADD_FACTORY_NODE(MakeVector3Node);
	ADD_FACTORY_NODE(MakeVector4Node);
	ADD_FACTORY_NODE(BreakVector2Node);

	nodeCreateFuncs["ShaderResultNode"] = 
		[](ImFlow::ImNodeFlow* nodeFlow) -> std::shared_ptr<ShaderNode> 
		{ 
			auto findNode = nodeFlow->getNodes()
				| std::views::transform([](const auto& item) { return std::dynamic_pointer_cast<ShaderNode>(item.second); })
				| std::views::filter([](const auto& item) { return !!std::dynamic_pointer_cast<ShaderResultNode>(item); });

			return *findNode.begin();

		};

}

ShaderNodeFactory::~ShaderNodeFactory() = default;

void ShaderNodeFactory::Set(NodeFlow* grid)
{
	myGrid = grid;
}

std::shared_ptr<ShaderNode> ShaderNodeFactory::Create(std::string_view type)
{
	assert(nodeCreateFuncs[type.data()]);
	return nodeCreateFuncs[type.data()](myGrid);
}
