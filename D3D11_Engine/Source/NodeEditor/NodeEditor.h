#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImNodeFlow.h>
#include "NodeFactory.h"
#include <fstream>
#include <filesystem>

class NodeEditor
{
public:
	NodeEditor(std::filesystem::path path);
	virtual ~NodeEditor();

	void Update();
	void Save();
	void Load(std::filesystem::path path);
	virtual void UpdateImp() {};

protected:
	std::shared_ptr<ImFlow::ImNodeFlow> myGrid;
	ShaderNodeFactory nodeFactory;
	class ShaderResultNode* resultNode;
	std::filesystem::path path;
};

//unique_ptr 사용 권장
class ShaderNodeEditor : public NodeEditor
{
public:
	ShaderNodeEditor(std::filesystem::path path = "NodeEditor.Proj") : NodeEditor(path) {}
	virtual void UpdateImp() override;
	void GenerateShaderCode();
	
};
