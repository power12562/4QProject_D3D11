#include "NodeEditor.h"
#include "json.hpp"
#include "ShaderNodes.h"
#include <ranges>
#include <chrono>
#include <set>
#include "Utility/WinUtility.h"
#include "Asset\MaterialAsset.h"



std::shared_ptr<ShaderNode> NodeFlow::Create(std::string_view typeName)
{
	return nodeFactory.Create(typeName);
}

NodeEditor::NodeEditor() : path{}
{
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
	ImGui::BeginGroup();
	ImGui::GetWindowDrawList()->AddCircle( ImGui::GetCursorPos() + ImGui::GetWindowPos() +  ImVec2 { 16.0f, 16.0f }, 16.0f, IM_COL32(255, 255, 255, 255));
	ImGui::Dummy(ImVec2(32.0f, 32.0f));
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("Drag to move");
	}
	
	ImGui::PushItemWidth(200);
	ImGui::Text("Shading Model");
	ImGui::PushID("shadingModel");
	ImGui::Combo("", (int*)&myGrid->shadingModel, EShadingModel::name, IM_ARRAYSIZE(EShadingModel::name));
	ImGui::PopID();
	ImGui::Text("BlendMode");
	ImGui::PushID("blendMode");
	ImGui::Combo("", (int*)&myGrid->blendMode, EBlendMode::name, IM_ARRAYSIZE(EBlendMode::name));
	ImGui::PopID();
	ImGui::PopItemWidth();
	ImGui::EndGroup();
	ImGui::SameLine();

	myGrid->update();


	ImVec2 mousePos = ImGui::GetMousePos();  // 마우스 위치
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

			for (const auto& link : myGrid->getLinks() | std::views::transform([](const auto& item) { return item.lock().get(); }))
			{
				link->DragSelect({ minX, minY }, { maxX, maxY });
			}
		}

	}
	if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyDown(ImGuiKey_S))
	{
		Save();
	}

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
				Save();
				auto newpath = WinUtility::GetOpenFilePath(path.c_str());
				Load(newpath);
			}

			if (ImGui::MenuItem((char*)u8"내보내기", nullptr, nullptr, true))
			{
				Export();
			}
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
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

	j["shadingModel"] = myGrid->shadingModel;
	j["blendMode"] = myGrid->blendMode;

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
	myGrid->path = path;
	std::ifstream file(path);
	if (!file.is_open())
	{
		return;
	}
	nlohmann::json j;
	file >> j;
	file.close();
	if (j.find("shadingModel") != j.end())
	{
		myGrid->shadingModel = j["shadingModel"];
	}
	if (j.find("blendMode") != j.end())
	{
		myGrid->blendMode = j["blendMode"];
	}

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
		if (ImGui::IsKeyDown(ImGuiKey_A))
		{
			myGrid->Create("AddNode");
		}
		if (ImGui::IsKeyDown(ImGuiKey_S))
		{
			myGrid->Create("SubNode");
		}
		if (ImGui::IsKeyDown(ImGuiKey_M))
		{
			myGrid->Create("MullNode");
		}
		if (ImGui::IsKeyDown(ImGuiKey_D))
		{
			myGrid->Create("DivNode");
		}
		if (ImGui::IsKeyDown(ImGuiKey_U))
		{
			myGrid->Create("TexCoordNode");
		}
	}

};


void ShaderNodeEditor::Load(std::filesystem::path path)
{
	NodeEditor::Load(path);



	myGrid->rightClickPopUpContent(
		[this](ImFlow::BaseNode* node)
		{
			if (!node)
			{
				if (ImGui::MenuItem((char*)u8"시간"))
				{
					myGrid->Create("TimeNode");
				}
				if (ImGui::MenuItem((char*)u8"float", "(1)"))
				{
					myGrid->Create("ConstantValueNode");
				}
				if (ImGui::MenuItem((char*)u8"float2", "(2)"))
				{
					myGrid->Create("ConstantVector2Node");
				}
				if (ImGui::MenuItem((char*)u8"float3", "(3)"))
				{
					myGrid->Create("ConstantVector3Node");
				}
				if (ImGui::MenuItem((char*)u8"float4", "(4)"))
				{
					myGrid->Create("ConstantVector4Node");
				}
				if (ImGui::MenuItem((char*)u8"텍스처", "(T)"))
				{
					myGrid->Create("TextureNode");
				}
				if (ImGui::MenuItem((char*)u8"더하기", "(A)"))
				{
					myGrid->Create("AddNode");
				}
				if (ImGui::MenuItem((char*)u8"빼기", "(S)"))
				{
					myGrid->Create("SubNode");
				}
				if (ImGui::MenuItem((char*)u8"곱하기", "(M)"))
				{
					myGrid->Create("MullNode");
				}
				if (ImGui::MenuItem((char*)u8"나누기", "(D)"))
				{
					myGrid->Create("DivNode");
				}
				if (ImGui::MenuItem((char*)u8"텍스처 좌표", "(U)"))
				{
					myGrid->Create("TexCoordNode");
				}
				if (ImGui::MenuItem((char*)u8"float2만들기"))
				{
					myGrid->Create("MakeVector2Node");
				}
				if (ImGui::MenuItem((char*)u8"float3만들기"))
				{
					myGrid->Create("MakeVector3Node");
				}
				if (ImGui::MenuItem((char*)u8"float4만들기"))
				{
					myGrid->Create("MakeVector4Node");
				}
				if (ImGui::MenuItem((char*)u8"float2분해"))
				{
					myGrid->Create("BreakVector2Node");
				}
			}
		});


	myGrid->droppedLinkPopUpContent(
		[this](ImFlow::Pin* dragged)
		{
			std::shared_ptr<ShaderNode> result;
			if (ImGui::MenuItem((char*)u8"더하기", "(A)"))
			{
				result = myGrid->Create("AddNode");
			}
			if (ImGui::MenuItem((char*)u8"빼기", "(S)"))
			{
				result = myGrid->Create("SubNode");
			}
			if (ImGui::MenuItem((char*)u8"곱하기", "(M)"))
			{
				result = myGrid->Create("MulNode");
			}
			if (ImGui::MenuItem((char*)u8"나누기", "(D)"))
			{
				result = myGrid->Create("DivNode");
			}
			if (ImGui::MenuItem((char*)u8"float2만들기"))
			{
				result = myGrid->Create("MakeVector2Node");
			}
			if (ImGui::MenuItem((char*)u8"float3만들기"))
			{
				result = myGrid->Create("MakeVector3Node");
			}
			if (ImGui::MenuItem((char*)u8"float4만들기"))
			{
				result = myGrid->Create("MakeVector4Node");
			}
			if (ImGui::MenuItem((char*)u8"float2분해"))
			{
				result = myGrid->Create("BreakVector2Node");
			}
			if (result && dragged->getType() == ImFlow::PinType::PinType_Output)
			{
				result->getIns().front()->createLink(dragged);
			}
		});


}

void ShaderNodeEditor::Export()
{
	std::vector<std::shared_ptr<ShaderDataProcess>> originalNodeReturn;
	std::vector<std::shared_ptr<ShaderDataProcess>>& processVector = myGrid->GetShaderNodeReturn().data;
	processVector.clear();



	for (size_t i = 0; i < EShaderResult::MAX; i++)
	{
		auto value = myGrid->GetResultNode()->getInVal<ShaderPin<void>>(EShaderResult::pinNames[i]);

		for (auto& item : processVector)
		{
			originalNodeReturn.emplace_back(item);
		}

		if (value.value)
		{
			auto execution = std::make_shared<Execution>();
			execution->leftIdentifier = EShaderResult::hlslName[i];
			execution->rightIdentifier = value.value->identifier;
			originalNodeReturn.emplace_back(execution);
		}

		processVector.clear();
	}

	std::vector<Define*> defines;
	std::vector<RegistorVariable*> registerValues;
	std::vector<LocalVariable*> localValues;
	std::vector<Execution*> executions;

	std::set<std::shared_ptr<RegistorVariable>> uniqueSet;
	std::set<std::shared_ptr<LocalVariable>> uniqueSet2;
	std::set<std::shared_ptr<Execution>> uniqueSet3;

	for (auto& item : originalNodeReturn)
	{
		if (auto data = std::dynamic_pointer_cast<Define>(item))
		{
			defines.emplace_back(data.get());
		}
		else if (auto data = std::dynamic_pointer_cast<RegistorVariable>(item))
		{
			if (uniqueSet.insert(data).second)
			{
				registerValues.emplace_back(data.get());
			}
		}
		else if (auto data = std::dynamic_pointer_cast<LocalVariable>(item))
		{
			if (uniqueSet2.insert(data).second)
			{
				localValues.emplace_back(data.get());
			}
		}
		else if (auto data = std::dynamic_pointer_cast<Execution>(item))
		{
			if (uniqueSet3.insert(data).second)
			{
				executions.emplace_back(data.get());
			}
		}
		 
	}

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

	bool isFoward = false;

	switch (myGrid->blendMode)
	{
	case EBlendMode::AlphaBlend:
	{
		defineLine << "#define BLEND_ALPHA" << std::endl;
		defineLine << "#define ALPHA_TEST" << std::endl;
		defineLine << "#define FORWARD" << std::endl;

		isFoward = true;
		break;
	}
	case EBlendMode::AlphaToCoverage:
	{
		defineLine << "#define ALPHA_TEST" << std::endl;
		break;
	}
	case EBlendMode::Dithering:
	{
		defineLine << "#define DITHERING" << std::endl;
		break;
	}
	case EBlendMode::Opaque:
	{
		break;
	}
	default:
		break;
	}



	std::string content = std::format(
		R"aa(
#include "../EngineShader/Shared.hlsli"
#include "../EngineShader/GBufferMaterial.hlsli"

struct PerFrameConstants
{{
	float Time;
	float Time0_1;
	float2 pad; //추천좀....
}};

struct CustomBuffer
{{
// 인풋 따로 받게끔 ex) 색, 가중치 같은거
}};
PerFrameConstants frameData : register(b3);
//CustomBuffer customData : register(b4);

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
	materialAsset.SetPixelShader(content, isFoward);

	for (auto& item : registerValues)
	{
		std::filesystem::path projPath = path;
		std::filesystem::path currPath = std::filesystem::current_path();

		if (projPath.is_relative())
		{
			projPath = currPath / projPath.parent_path();
		}

		std::filesystem::path relativePath = std::filesystem::relative(item->path, projPath);

		materialAsset.SetTexture2D(relativePath.c_str(), item->registorSlot);
	}
	materialAsset.SaveAsset();



	originalNodeReturn.clear();
}
