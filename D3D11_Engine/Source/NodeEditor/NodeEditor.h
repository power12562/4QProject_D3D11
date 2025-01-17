#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImNodeFlow.h>
#include <fstream>
#include <filesystem>
#include "shaderNodes.h"
class NodeFlow;


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
	std::shared_ptr<NodeFlow> myGrid;
	std::filesystem::path path;
	std::string fileName;


	ImVec2 dragStartPos = ImVec2(0, 0);  // 드래그 시작 위치
	ImVec2 dragEndPos = ImVec2(0, 0);    // 드래그 끝 위치
	bool isDragging = false;              // 드래그 상태 확인
	bool isPopUp;
};

//unique_ptr 사용 권장
class ShaderNodeEditor : public NodeEditor
{
public:
	ShaderNodeEditor(std::filesystem::path path = "NodeEditor") : NodeEditor(path) {}

public:
	virtual void UpdateImp() override;
	void Export();

	template<typename NodeType, typename ValueType>
	void SetResultNode(ValueType value, EShaderResult::Type resultType)
	{
		auto newNode = myGrid->addNode<NodeType>({ 0,0 });
		newNode->Set(value);
		auto resultPin = myGrid->GetResultNode()->inPin(EShaderResult::pinNames[resultType]);
		auto outpin = newNode->outPin((const char*)u8"RGB");
		outpin->createLink(resultPin);
	}

};


