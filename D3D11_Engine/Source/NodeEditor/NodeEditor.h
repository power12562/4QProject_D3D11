#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImNodeFlow.h>
#include <string>
#include <format>
#include <directxtk\SimpleMath.h>
#include <ranges>

using Vector2 = DirectX::SimpleMath::Vector2;
using Vector3 = DirectX::SimpleMath::Vector3;
using Vector4 = DirectX::SimpleMath::Vector4;
#include <any>


struct NodeDataStruct
{
	std::string type;
	std::string identifier;
	std::string initializationExpression;
	uint8_t isConstant : 1;
	uint8_t isRegistor : 1;
};

struct Definition
{
	NodeDataStruct& data;
};


std::ostream& operator<<(std::ostream& os, const Definition& data)
{
	if (data.data.isConstant)
	{
		return os;
	}
	return os << std::format("{} {} = {}", data.data.type, data.data.identifier, data.data.initializationExpression);
}



struct ShaderNodeData
{

};

template<typename T>
concept IsShaderNodeData = std::is_base_of_v<ShaderNodeData, T>;



class ConstantValueNode : public ImFlow::BaseNode
{
public:
	ConstantValueNode()
	{
		Set(0.0f);
		setStyle(ImFlow::NodeStyle::green());
		addOUT<NodeDataStruct>("")->behaviour(
			[this]() 
			{ 
				return NodeDataStruct
				{
					"",
					"",
					std::format({"{}"}, value),
					true
				};
			});
	};

	void Set(float value)
	{
		setTitle(std::format("{}", value));
		this->value = value;
	}

	void draw() override
	{
		ImGui::SetNextItemWidth(100);
		ImGui::Text((char*)u8"��");
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("x��");
		if (ImGui::InputFloat("", &value)) 
		{
			Set(value);
		}


		ImGui::PopID();
	}

private:
	float value;
};


class ConstantVector2Node : public ImFlow::BaseNode
{
public:
	ConstantVector2Node()
	{
		Set(value);
		setStyle(ImFlow::NodeStyle::green());
		addOUT<NodeDataStruct>("")->behaviour(
			[this]()
			{
				return NodeDataStruct
				{
					"",
					"",
					std::format({"{}, {}"}, value.x, value.y),
					true
				};
			});
	};

	void Set(const Vector2& value)
	{
		setTitle(std::format({ "{}, {}" }, value.x, value.y));
		this->value = value;
	}

	void draw() override
	{
		ImGui::SetNextItemWidth(100);
		ImGui::Text((char*)u8"��");
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("x��");
		if (ImGui::InputFloat2("", (float*)&value))
		{
			Set(value);
		}


		ImGui::PopID();
	}

private:
	Vector2 value;
};


class ConstantVector3Node : public ImFlow::BaseNode
{
public:
	ConstantVector3Node()
	{
		Set(value);
		setStyle(ImFlow::NodeStyle::green());
		addOUT<NodeDataStruct>("")->behaviour(
			[this]()
			{
				return NodeDataStruct
				{
					"",
					"",
					std::format({"{}, {}. {}"}, value.x, value.y, value.z),
					true
				};
			});
	};

	void Set(const Vector3& value)
	{
		setTitle(std::format({ "{}, {}. {}" }, value.x, value.y, value.z));
		this->value = value;
	}

	void draw() override
	{
		ImGui::SetNextItemWidth(100);
		ImGui::Text((char*)u8"��");
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("x��");
		if (ImGui::InputFloat3("", (float*)&value))
		{
			Set(value);
		}


		ImGui::PopID();
	}

private:
	Vector3 value;
};


class ConstantVector4Node : public ImFlow::BaseNode
{
public:
	ConstantVector4Node()
	{
		Set(value);
		setStyle(ImFlow::NodeStyle::green());
		addOUT<NodeDataStruct>("")->behaviour(
			[this]()
			{
				return NodeDataStruct
				{
					"",
					"",
					std::format({"{}, {}. {}, {}"}, value.x, value.y, value.z, value.w),
					true
				};
			});
	};

	void Set(const Vector4& value)
	{
		setTitle(std::format({ "{}, {}. {}, {}" }, value.x, value.y, value.z, value.w));
		this->value = value;
	}

	void draw() override
	{
		ImGui::SetNextItemWidth(100);
		ImGui::Text((char*)u8"��");
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("x��");
		if (ImGui::InputFloat4("", (float*)&value))
		{
			Set(value);
		}


		ImGui::PopID();
	}

private:
	Vector4 value;
};



struct ValueNodeData : public ShaderNodeData
{
	std::string valueName;
};

struct TextureNodeData : public ShaderNodeData
{
	std::string texturePath;
};



class ShaderResultNode : public ImFlow::BaseNode
{
	static std::function<bool(ImFlow::Pin*, ImFlow::Pin*)> AnyCast() 
	{ 
		return 
			[](ImFlow::Pin* out, ImFlow::Pin* in) 
			{ 
				return true; 
			}; 
	}
public:
	ShaderResultNode()
	{
		setStyle(ImFlow::NodeStyle::red());
		setTitle("BRDF");

		addIN<NodeDataStruct>((char*)u8"�˺���", NodeDataStruct{}, AnyCast());
		addIN<NodeDataStruct>((char*)u8"�븻", NodeDataStruct{}, AnyCast());
	}



};

class NodeEditor
{
public:
	NodeEditor()
	{
		myGrid = std::make_unique<ImFlow::ImNodeFlow>();
		resultNode = myGrid->addNode<ShaderResultNode>({ 0,0 });
	}
	~NodeEditor()
	{
		if (myGrid)
		{
			// ��� ��� ����
		}
		myGrid.reset();
	}

	void Update()
	{
		ImGui::Begin("Example App");

		ImVec2 mousePos = ImGui::GetMousePos();  // ���콺 ��ġ
		bool isMouseClicked = ImGui::IsMouseClicked(0);  // ��Ŭ�� Ȯ��
		if (isMouseClicked)
		{
			ImVec2 greedPos = myGrid->screen2grid(mousePos);
			if (ImGui::IsKeyDown(ImGuiKey_1))
			{
				myGrid->addNode<ConstantValueNode>(greedPos);
			}
			if (ImGui::IsKeyDown(ImGuiKey_2))
			{
				myGrid->addNode<ConstantVector2Node>(greedPos);
			}
			if (ImGui::IsKeyDown(ImGuiKey_3))
			{
				myGrid->addNode<ConstantVector3Node>(greedPos);
			}
			if (ImGui::IsKeyDown(ImGuiKey_4))
			{
				myGrid->addNode<ConstantVector4Node>(greedPos);
			}
		}
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
					float minX = (std::min)(dragStartPos.x, dragEndPos.x);
					float maxX = (std::max)(dragStartPos.x, dragEndPos.x);
					float minY = (std::min)(dragStartPos.y, dragEndPos.y);
					float maxY = (std::max)(dragStartPos.y, dragEndPos.y);

					if (nodePos.x >= minX && nodePos.x <= maxX &&
						nodePos.y >= minY && nodePos.y <= maxY)
					{
						node->selected(true);
					}

				}
			}
		}

		ImGui::End();

	}



private:
	std::unique_ptr<ImFlow::ImNodeFlow> myGrid;
	std::shared_ptr<ShaderResultNode> resultNode;

protected:

};


class ShaderCode
{
public:
	std::string code;
};


class ShaderNodeEditor : public NodeEditor
{
public:
	ShaderCode GenerateShaderCode()
	{
		ShaderCode code;
		return code;
	}
};
