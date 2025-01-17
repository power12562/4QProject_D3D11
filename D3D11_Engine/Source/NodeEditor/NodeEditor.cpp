#include "NodeEditor.h"
#include "json.hpp"
#include "ShaderNodes.h"
#include <ranges>
#include <chrono>
#include "Utility/WinUtility.h"
#include "Asset\MaterialAsset.h"



std::shared_ptr<ShaderNode> NodeFlow::Create(std::string_view typeName)
{
	return nodeFactory.Create(typeName);
}

NodeEditor::NodeEditor(std::filesystem::path path) : path{}
{
	Load(path);
	//myGrid = std::make_shared<ImFlow::ImNodeFlow>();
	//nodeFactory.Set(myGrid);
	//resultNode = myGrid->addNode<ShaderResultNode>({ 100, 100 }).get();
	//auto temp = myGrid->addNode<ConstantVector3Node>({ 0, 0 }).get();
	//temp->outPin(std::string((char*)u8"값"))->createLink(resultNode->inPin((const char*)u8"알베도"));
}

NodeEditor::~NodeEditor()
{
	if (myGrid)
	{
		Save();
		// 모든 노드 저장
	}

	myGrid.reset();
}

void NodeEditor::Update()
{
	if (!isPopUp) return;
	
	ImGui::Begin(fileName.c_str(), &isPopUp, ImGuiWindowFlags_MenuBar);
	
	UpdateImp();
	ImVec2 mousePos = ImGui::GetMousePos();  // 마우스 위치
	bool isMouseClicked = ImGui::IsMouseClicked(0);  // 좌클릭 확인
	if (isMouseClicked)
	{
		if (ImGui::IsKeyDown(ImGuiKey_1))
		{
			myGrid->Create("ConstantValueNode");
		}
		if (ImGui::IsKeyDown(ImGuiKey_2))
		{
			myGrid->Create("ConstantVector2Node");
		}
		if (ImGui::IsKeyDown(ImGuiKey_3))
		{
			myGrid->Create("ConstantVector3Node");
		}
		if (ImGui::IsKeyDown(ImGuiKey_4))
		{
			myGrid->Create("ConstantVector4Node");
		}
		if (ImGui::IsKeyDown(ImGuiKey_T))
		{
			myGrid->Create("TextureNode");
		}
	}



	myGrid->update();


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
		float minX = (std::min)(dragStartPos.x, dragEndPos.x);
		float maxX = (std::max)(dragStartPos.x, dragEndPos.x);
		float minY = (std::min)(dragStartPos.y, dragEndPos.y);
		float maxY = (std::max)(dragStartPos.y, dragEndPos.y);

		//화면 영역만
		minX = (std::max)(minX, myGrid->getPos().x);
		minY = (std::max)(minY, myGrid->getPos().y);
		maxX = (std::min)(maxX, myGrid->getPos().x + myGrid->getGrid().size().x);
		maxY = (std::min)(maxY, myGrid->getPos().y + myGrid->getGrid().size().y);
		draw_list->AddRect(ImVec2(minX, minY), ImVec2(maxX, maxY), IM_COL32(255, 255, 255, 255));


		// 마우스가 놓였을 때 선택 처리
		if (ImGui::IsMouseReleased(0))
		{
			isDragging = false;

			dragStartPos = myGrid->screen2grid(dragStartPos);
			dragEndPos = myGrid->screen2grid(dragEndPos);
			minX = (std::min)(dragStartPos.x, dragEndPos.x);
			maxX = (std::max)(dragStartPos.x, dragEndPos.x);
			minY = (std::min)(dragStartPos.y, dragEndPos.y);
			maxY = (std::max)(dragStartPos.y, dragEndPos.y);

			for (ImFlow::BaseNode* node : myGrid->getNodes() | std::views::transform([](auto& item) { return item.second.get(); }))
			{
				ImVec2 nodePos = node->getPos();
				auto nodeSize = node->getSize();

				// 노드와 드래그 박스가 겹치는지 확인
				if (nodePos.x < maxX && nodePos.x + nodeSize.x > minX &&
					nodePos.y < maxY && nodePos.y + nodeSize.y > minY)
				{
					node->selected(true);
				}


			}
		}

	}
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyDown(ImGuiKey_S))
	{
		Save();
	}

	ImGui::End();

}



void NodeEditor::Save()
{
	if (!myGrid)
	{
		return;
	}

	nlohmann::json j;
	for (ImFlow::BaseNode* item : myGrid->getNodes() | std::views::transform([](const auto& item) {return item.second.get();}))
	{
		nlohmann::json nodeJson;

		auto typeview =
			std::string_view(typeid(*item).name())
			| std::views::reverse
			| std::views::take_while([](char item) { return item != ' '; })
			| std::views::reverse
			| std::views::common;

		std::string type(typeview.begin(), typeview.end());

		nodeJson["Type"] = type;
		nodeJson["pos"] = { item->getPos().x, item->getPos().y };
		nodeJson["uid"] = item->getUID();

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
		
		linkJson["leftNode"] = item->left()->getParent()->getUID();
		linkJson["left"] = item->left()->getUid();

		linkJson["rightNode"] = item->right()->getParent()->getUID();
		linkJson["right"] = item->right()->getUid();
		
		j["links"].push_back(linkJson);
	}

	std::ofstream file(path);
	file << j.dump(4);
	file.close();
}

void NodeEditor::Load(std::filesystem::path path)
{
	isPopUp = true;
	path.replace_extension(".Proj");
	if (std::filesystem::exists(this->path))
	{
		Save();
	}
	fileName = path.filename().string();
	myGrid = std::make_shared<NodeFlow>();
	this->path = path;



	myGrid->rightClickPopUpContent(
		[this](ImFlow::BaseNode* node)
		{
			if (ImGui::MenuItem((char*)u8"텍스처"))
			{
				myGrid->Create("TextureNode");
			}
		});


	myGrid->droppedLinkPopUpContent([](ImFlow::Pin* dragged)
									{

									});


	std::ifstream file(path);
	if (!file.is_open())
	{
		return;
	}
	nlohmann::json j;
	file >> j;
	file.close();

	
	std::map<ImFlow::NodeUID, std::shared_ptr<ImFlow::BaseNode>> nodes;
	for (auto& item : j["nodes"])
	{
		auto newNode = myGrid->Create(item["Type"]);
		newNode->setPos({ item["pos"][0], item["pos"][1] });

		ISerializable* serializeable = dynamic_cast<ISerializable*>(newNode.get());
		if (serializeable)
		{
			serializeable->Deserialize(item);
		}

		nodes.emplace(item["uid"], newNode);
	}

	for (auto& item : j["links"])
	{
		std::shared_ptr< ImFlow::Pin> left = nullptr;
		std::shared_ptr< ImFlow::Pin> right = nullptr;
		auto leftUid = item["left"].get<ImFlow::PinUID>();
		auto rightUid = item["right"].get<ImFlow::PinUID>();
		auto leftNodeUid = item["leftNode"].get<ImFlow::PinUID>();
		auto rightNodeUid = item["rightNode"].get<ImFlow::PinUID>();
		

		left = (nodes[leftNodeUid]->getOuts() | std::views::filter([&leftUid](auto& item) { return item->getUid() == leftUid; }) | std::views::take(1)).front();
		right = (nodes[rightNodeUid]->getIns() | std::views::filter([&rightUid](auto& item) { return item->getUid() == rightUid; }) | std::views::take(1)).front();

		if (left && right)
		{
			left->createLink(right.get());
		}
	}


}

void ShaderNodeEditor::UpdateImp()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu((char*)u8"파일", true))
		{
			if (ImGui::MenuItem((char*)u8"저장", "Ctrl + S", nullptr, true))
			{
				Save();
			}

			if (ImGui::MenuItem((char*)u8"열기", nullptr, nullptr, true))
			{

				//Save();
			}
			
			if (ImGui::MenuItem((char*)u8"내보내기", nullptr, nullptr, true))
			{
				Export();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
};

void ShaderNodeEditor::Export()
{
	std::vector<std::shared_ptr<ShaderDataProcess>> originalNodeReturn;

	for (size_t i = 0; i < EShaderResult::MAX; i++)
	{
		std::vector<std::shared_ptr<ShaderDataProcess>>& processVector = myGrid->GetShaderNodeReturn().data;

		processVector.clear();

		myGrid->GetResultNode()->getInVal<ShaderPin>(EShaderResult::pinNames[i]);

		for (auto& item : processVector)
		{
			originalNodeReturn.emplace_back(item);
		}

		if (myGrid->GetShaderNodeReturn().result)
		{
			auto execution = std::make_shared<Execution>();
			execution->leftIdentifier = EShaderResult::hlslName[i];
			execution->rightIdentifier = myGrid->GetShaderNodeReturn().result->identifier;
			originalNodeReturn.emplace_back(execution);
		}

		processVector.clear();
		myGrid->GetShaderNodeReturn().result = {};
		myGrid->get_recursion_blacklist().clear();
	}

	std::vector<Define*> defines;
	std::vector<RegistorVariable*> registerValues;
	std::vector<LocalVariable*> localValues;
	std::vector<Execution*> executions;

	for (auto& item : originalNodeReturn)
	{
		if (auto data = std::dynamic_pointer_cast<Define>(item))
		{
			defines.emplace_back(data.get());
		}
		else if (auto data = std::dynamic_pointer_cast<RegistorVariable>(item))
		{
			registerValues.emplace_back(data.get());
		}
		else if (auto data = std::dynamic_pointer_cast<LocalVariable>(item))
		{
			localValues.emplace_back(data.get());
		}
		else if (auto data = std::dynamic_pointer_cast<Execution>(item))
		{
			executions.emplace_back(data.get());
		}
		 
	}
	std::ranges::sort(defines, [](auto& a, auto& b) { return a->name < b->name; });
	defines.erase(std::ranges::unique(defines, [](auto& a, auto& b) { return a->name == b->name; }).begin(), defines.end());
	
	std::ranges::sort(registerValues, [](auto& a, auto& b) { return a->identifier < b->identifier; });
	registerValues.erase(std::ranges::unique(registerValues, [](auto& a, auto& b) { return a->identifier == b->identifier; }).begin(), registerValues.end());

	std::ranges::sort(localValues, [](auto& a, auto& b) { return a->identifier < b->identifier; });
	localValues.erase(std::ranges::unique(localValues, [](auto& a, auto& b) { return a->identifier == b->identifier; }).begin(), localValues.end());

	std::stringstream defineLine;
	std::stringstream registerValueLine;
	std::stringstream localValueLine;
	std::stringstream executionsLine;

	for (auto& item : defines)
	{
		defineLine << *item << std::endl;
	}

	int registerCounts[ERegisterSlot::MAX] = { 0, };

	for (auto& item : registerValues)
	{
		item->slotNum = registerCounts[item->registorSlot]++;
		registerValueLine << *item << std::endl;
	}

	for (auto& item : localValues)
	{
		localValueLine << *item << std::endl;
	}

	for (auto& item : executions)
	{
		executionsLine << *item << std::endl;
	}




	std::string content = std::format(
		R"aa(
#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/GBufferMaterial.hlsli"

{0}
{1}

GBufferMaterial GetCustomGBufferMaterial(PS_INPUT input)
{{
    GBufferMaterial material = GetDefaultGBufferMaterial(input);

{2}
{3}
    return material;
}}

#define GetGBufferMaterial GetCustomGBufferMaterial
#include "../EngineShader/BasePassPS.hlsl"
)aa",
defineLine.str(),
registerValueLine.str(),
localValueLine.str(),
executionsLine.str()
);




	MaterialAsset materialAsset;
	materialAsset.SetPixelShader(content);

	for (auto& item : registerValues)
	{
		std::filesystem::path relativePath =  std::filesystem::relative(item->path, std::filesystem::current_path());

		materialAsset.SetTexture2D(relativePath.c_str(), item->registorSlot);
	}

	materialAsset.SaveAsset();



	originalNodeReturn.clear();
}
