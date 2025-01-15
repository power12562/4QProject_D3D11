#include "NodeEditor.h"



NodeEditor::NodeEditor()
{
	myGrid = std::make_shared<ImFlow::ImNodeFlow>();
	resultNode = myGrid->addNode<ShaderResultNode>({ 0,0 }, ++count).get();

	myGrid->rightClickPopUpContent(
		[](ImFlow::BaseNode* node)
		{
			if (ImGui::MenuItem((char*)u8"�ؽ�ó"))
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
		// ��� ��� ����
	}
	myGrid.reset();
}

void NodeEditor::Update()
{
	ImGui::Begin("Example App",nullptr, ImGuiWindowFlags_MenuBar);
	UpdateImp();
	ImVec2 mousePos = ImGui::GetMousePos();  // ���콺 ��ġ
	bool isMouseClicked = ImGui::IsMouseClicked(0);  // ��Ŭ�� Ȯ��
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

	static ImVec2 dragStartPos = ImVec2(0, 0);  // �巡�� ���� ��ġ
	static ImVec2 dragEndPos = ImVec2(0, 0);    // �巡�� �� ��ġ
	static bool isDragging = false;              // �巡�� ���� Ȯ��

	// ���콺 Ŭ�� �� �巡�� ���� Ȯ��
	if (ImGui::IsMouseClicked(0))
	{
		auto& grid = myGrid->getGrid();
		//���콺�� �׸�������� Ȯ��
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

	// �巡�� ���� ��
	if (isDragging)
	{
		dragEndPos = (ImGui::GetMousePos());

		// �巡�� ������ �׸���
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		draw_list->AddRect(dragStartPos, dragEndPos, IM_COL32(255, 255, 255, 255));


		// ���콺�� ������ �� ���� ó��
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

}
