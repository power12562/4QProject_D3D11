#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImNodeFlow.h>
#include "ShaderNodes.h"
#include <ranges>
#include <chrono>
#include <fstream>

#include <any>

class NodeEditor
{
public:
	NodeEditor();
	virtual ~NodeEditor();

	void Update();
	void Save();
	virtual void UpdateImp() {};


protected:
	std::shared_ptr<ImFlow::ImNodeFlow> myGrid;
	ShaderResultNode* resultNode;
	size_t count = 0;
};

class ShaderNodeEditor : public NodeEditor
{
public:
	virtual void UpdateImp() override
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu((char*)u8"파일", true))
			{
				if (ImGui::MenuItem((char*)u8"내보내기", nullptr, nullptr, true))
				{
					GenerateShaderCode();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	};
	void GenerateShaderCode()
	{
		std::stringstream ss;
		std::vector<ShaderDataProcess*> originalNodeReturn;

		for (size_t i = 0; i < EShaderResult::MAX; i++)
		{
			auto temp = resultNode->getInVal<ShaderNodeReturn>(EShaderResult::pinNames[i]);
			if (temp)
			{
				for (auto& item : *temp)
				{
					originalNodeReturn.emplace_back(item);
				}

				// temp의 마지막 Variable 찾기
				auto lastVariable = std::ranges::find_if(temp->rbegin(), temp->rend(), [](auto& item) { return dynamic_cast<Variable*>(item); });



				if (lastVariable != temp->rend())
				{
					Execution* execution = new Execution;
					execution->leftIdentifier = EShaderResult::hlslName[i];
					execution->rightIdentifier = static_cast<Variable*>(*lastVariable)->identifier;
					originalNodeReturn.emplace_back(execution);
				}

				delete temp;
			}
			myGrid->get_recursion_blacklist().clear();
		}

		std::vector<Variable*> definitions;
		std::vector<Execution*> executions;
		std::vector<Define*> defines;

		for (auto& item : originalNodeReturn)
		{
			if (auto data = dynamic_cast<Variable*>(item))
			{
				definitions.emplace_back(data);
			}
			else if (auto data = dynamic_cast<Execution*>(item))
			{
				executions.emplace_back(data);
			}
			else if (auto data = dynamic_cast<Define*>(item))
			{
				defines.emplace_back(data);
			}
		}
		std::ranges::sort(defines, [](auto& a, auto& b) { return a->name < b->name; });
		defines.erase(std::ranges::unique(defines, [](auto& a, auto& b) { return a->name == b->name; }).begin(), defines.end());

		std::ranges::sort(definitions, [](auto& a, auto& b) { return a->identifier < b->identifier; });
		definitions.erase(std::ranges::unique(definitions, [](auto& a, auto& b) { return a->identifier == b->identifier; }).begin(), definitions.end());

		for (auto& item : defines)
		{
			ss << *item << std::endl;
		}

		for (auto& item : definitions)
		{
			ss << *item << std::endl;
		}

		for (auto& item : executions)
		{
			ss << *item << std::endl;
		}

		for (auto& item : originalNodeReturn)
		{
			delete item;
		}


		std::ofstream file("Resource/Shader/Effect.hlsl");

		if (file.is_open())
		{
			file << R"aa(
#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/GBufferMaterial.hlsli"

GBufferMaterial GetCustomGBufferMaterial(PS_INPUT input)
{
	GBufferMaterial material = GetDefaultGBufferMaterial(input);

)aa";
			file << ss.str() << std::endl;
			file << R"aa(

	return material;
}

#define GetGBufferMaterial GetCustomGBufferMaterial
#include "../EngineShader/BasePassPS.hlsl"
)aa";

			file.close();
		}
	}
};
