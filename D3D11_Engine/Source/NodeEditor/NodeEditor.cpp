#include "NodeEditor.h"



NodeEditor::NodeEditor()
{
	myGrid = std::make_shared<ImFlow::ImNodeFlow>();
	resultNode = myGrid->addNode<ShaderResultNode>({ 0,0 }, ++count).get();

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
		Save();
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
		ImVec2 greedPos = myGrid->screen2grid(mousePos);
		if (ImGui::IsKeyDown(ImGuiKey_1))
		{
			myGrid->addNode<ConstantValueNode>(greedPos, ++count);
		}
		if (ImGui::IsKeyDown(ImGuiKey_2))
		{
			myGrid->addNode<ConstantVector2Node>(greedPos, ++count);
		}
		if (ImGui::IsKeyDown(ImGuiKey_3))
		{
			myGrid->addNode<ConstantVector3Node>(greedPos, ++count);
		}
		if (ImGui::IsKeyDown(ImGuiKey_4))
		{
			myGrid->addNode<ConstantVector4Node>(greedPos, ++count);
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
		ShaderNode* node = static_cast<ShaderNode*>(item);
		nlohmann::json nodeJson;
		nlohmann::json pinsJson;
		nodeJson["Type"] = typeid(*item).name();
		nodeJson["pos"] = { item->getPos().x, item->getPos().y };
		nodeJson["size"] = { item->getSize().x, item->getSize().y };
		node->Serialize(nodeJson);
		for (auto& pin : node->getIns())
		{
			pinsJson.push_back(pin->getUid());
		}
		nodeJson["pins"] = pinsJson;
		j["nodes"].push_back(nodeJson);
	}

	for (const auto& item : myGrid->getLinks() | std::views::transform([](const auto& item) { return item.lock().get(); }))
	{
		nlohmann::json linkJson;
		linkJson["start"] = item->left()->getUid();
		linkJson["end"] = item->right()->getUid();
		j["links"].push_back(linkJson);
	}

	std::ofstream file(path);
	file << j.dump(4);
	file.close();



}

void NodeEditor::Load(std::filesystem::path path)
{
	if (std::filesystem::exists(path))
	{
		Save();
	}

	this->path = path;
	myGrid = std::make_shared<ImFlow::ImNodeFlow>();

	std::ifstream file(path);
	if (!file.is_open())
	{
		return;
	}
	nlohmann::json j;
	file >> j;
	file.close();

	struct Link
	{
		int startPinID;
		int endPinID;
	};
	ImFlow::BaseNode* lastNode = nullptr;
	lastNode->in
	for (auto& item : j["nodes"])
	{

	}

	for (auto& item : j["links"])
	{


	}
}
