#include "NodeEditor.h"
#include "json.hpp"
#include "ShaderNodes.h"
#include <ranges>
#include <chrono>



NodeEditor::NodeEditor(std::filesystem::path path) : path{}
{
	//Load(path);
	this->path = path;

	myGrid = std::make_shared<ImFlow::ImNodeFlow>();
	nodeFactory.Set(myGrid);
	resultNode = myGrid->addNode<ShaderResultNode>({ 100, 100 }).get();
	auto temp = myGrid->addNode<ConstantVector3Node>({ 0, 0 }).get();

	temp->outPin(std::string((char*)u8"값"))->createLink(resultNode->inPin((const char*)u8"알베도"));




	myGrid->rightClickPopUpContent(
		[](ImFlow::BaseNode* node)
		{
			if (ImGui::MenuItem((char*)u8"텍스처"))
			{
				__debugbreak();
			}
		});


	myGrid->droppedLinkPopUpContent([](ImFlow::Pin* dragged)
									{

									});

}

NodeEditor::~NodeEditor()
{
	if (myGrid)
	{
		//Save();
		// 모든 노드 저장
	}

	myGrid.reset();
}

void NodeEditor::Update()
{
	ImGui::Begin("Example App",nullptr, ImGuiWindowFlags_MenuBar);
	UpdateImp();
	ImVec2 mousePos = ImGui::GetMousePos();  // 마우스 위치
	bool isMouseClicked = ImGui::IsMouseClicked(0);  // 좌클릭 확인
	if (isMouseClicked)
	{
		if (ImGui::IsKeyDown(ImGuiKey_1))
		{
			nodeFactory.Create("ConstantValueNode");
		}
		if (ImGui::IsKeyDown(ImGuiKey_2))
		{
			nodeFactory.Create("ConstantVector2Node");
		}
		if (ImGui::IsKeyDown(ImGuiKey_3))
		{
			nodeFactory.Create("ConstantVector3Node");
		}
		if (ImGui::IsKeyDown(ImGuiKey_4))
		{
			nodeFactory.Create("ConstantVector4Node");
		}
	}

	myGrid->update();

	static ImVec2 dragStartPos = ImVec2(0, 0);  // 드래그 시작 위치
	static ImVec2 dragEndPos = ImVec2(0, 0);    // 드래그 끝 위치
	static bool isDragging = false;              // 드래그 상태 확인

	// 마우스 클릭 및 드래그 시작 확인
	if (ImGui::IsMouseClicked(0))
	{
		auto& grid = myGrid->getGrid();
		//마우스가 그리드안인지 확인
		if (mousePos.x > grid.origin().x && mousePos.y > grid.origin().y &&
			mousePos.x < grid.origin().x + grid.size().x && mousePos.y < grid.origin().y + grid.size().y)
		{
			dragStartPos = (ImGui::GetMousePos());
			isDragging = true;
		}
	}
	if (myGrid->on_selected_node())
	{
		isDragging = false;
	}

	// 드래그 중일 때
	if (isDragging)
	{
		dragEndPos = (ImGui::GetMousePos());

		// 드래그 영역을 그리기
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->AddRect(dragStartPos, dragEndPos, IM_COL32(255, 255, 255, 255));


		// 마우스가 놓였을 때 선택 처리
		if (ImGui::IsMouseReleased(0))
		{
			isDragging = false;

			dragStartPos = myGrid->screen2grid(dragStartPos);
			dragEndPos = myGrid->screen2grid(dragEndPos);

			for (ImFlow::BaseNode* node : myGrid->getNodes() | std::views::transform([](auto& item) { return item.second.get(); }))
			{
				ImVec2 nodePos = node->getPos();
				auto nodeSize = node->getSize();
				float minX = (std::min)(dragStartPos.x, dragEndPos.x);
				float maxX = (std::max)(dragStartPos.x, dragEndPos.x);
				float minY = (std::min)(dragStartPos.y, dragEndPos.y);
				float maxY = (std::max)(dragStartPos.y, dragEndPos.y);

				if (nodePos.x + nodeSize.x >= minX && nodePos.x - nodeSize.x <= maxX &&
					nodePos.y + nodeSize.y >= minY && nodePos.y - nodeSize.x <= maxY)
				{
					node->selected(true);
				}

			}
		}
	}

	ImGui::End();

}



void NodeEditor::Save()
{
	nlohmann::json j;
	for (ImFlow::BaseNode* item : myGrid->getNodes() | std::views::transform([](const auto& item) {return item.second.get();}))
	{
		nlohmann::json nodeJson;

		auto typeview =
			std::string_view(typeid(*item).name())
			| std::views::reverse
			| std::views::take_while([](char item) { return item == ' '; })
			| std::views::reverse;

		std::string type(typeview.begin(), typeview.end());

		nodeJson["Type"] = type;
		nodeJson["pos"] = { item->getPos().x, item->getPos().y };

		for (auto& pin : item->getIns())
		{
			nodeJson["pins"].push_back(pin->getUid());
		}
		for (auto& pin : item->getOuts())
		{
			nodeJson["pins"].push_back(pin->getUid());
		}

		ISerializable* serializeable = dynamic_cast<ISerializable*>(item);
		if (serializeable)
		{
			serializeable->Serialize(nodeJson);
		}


		j["nodes"].push_back(nodeJson);
	}

	for (const auto& item : myGrid->getLinks() | std::views::transform([](const auto& item) { return item.lock().get(); }))
	{
		nlohmann::json linkJson;
		linkJson["left"] = item->left()->getUid();
		linkJson["right"] = item->right()->getUid();
		j["links"].push_back(linkJson);
	}

	std::ofstream file(path);
	file << j.dump(4);
	file.close();
}

void NodeEditor::Load(std::filesystem::path path)
{
	if (std::filesystem::exists(this->path))
	{
		Save();
	}

	this->path = path;
	myGrid = std::make_shared<ImFlow::ImNodeFlow>();
	nodeFactory.Set(myGrid);

	std::ifstream file(path);
	if (!file.is_open())
	{
		return;
	}
	nlohmann::json j;
	file >> j;
	file.close();

	

	for (auto& item : j["nodes"])
	{
		auto newNode = nodeFactory.Create(item["Type"]);
		newNode->setPos({ item["pos"][0], item["pos"][1] });

		ISerializable* serializeable = dynamic_cast<ISerializable*>(newNode.get());
		if (serializeable)
		{
			serializeable->Deserialize(item);
		}
	}

	for (auto& item : j["links"])
	{
		ImFlow::Pin* left;
		ImFlow::Pin* right;

		for (const auto& node : myGrid->getNodes() | std::views::transform([](const auto& item) { return item.second.get(); }))
		{
			right = node->inPin(item["right"]);
			left = node->outPin(item["left"]);
			left->createLink(right);
		}


	}
}

void ShaderNodeEditor::UpdateImp()
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

void ShaderNodeEditor::GenerateShaderCode()
{
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

	std::stringstream defineLine;
	std::stringstream definitionsLine;
	std::stringstream executionsLine;

	for (auto& item : defines)
	{
		defineLine << *item << std::endl;
	}

	for (auto& item : definitions)
	{
		definitionsLine << *item << std::endl;
	}

	for (auto& item : executions)
	{
		executionsLine << *item << std::endl;
	}

	for (auto& item : originalNodeReturn)
	{
		delete item;
	}


	std::ofstream file("Resource/Shader/Effect.hlsl");

	if (file.is_open())
	{
		std::string content = std::format(
			R"aa(
#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/GBufferMaterial.hlsli"

{0}

GBufferMaterial GetCustomGBufferMaterial(PS_INPUT input)
{{
    GBufferMaterial material = GetDefaultGBufferMaterial(input);

{1}
{2}
    return material;
}}

#define GetGBufferMaterial GetCustomGBufferMaterial
#include "../EngineShader/BasePassPS.hlsl"
)aa",
defineLine.str(),
definitionsLine.str(),
executionsLine.str()
);

		file << content;
		file.close();
	}
}