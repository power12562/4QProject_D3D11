#include "ShaderNodes.h"

ShaderNode::ShaderNode(size_t guid) :
	guid(guid)
{
	setStyle(ImFlow::NodeStyle::green());
	
}

ConstantValueNode::ConstantValueNode(size_t newGuid) :
	ShaderNode(newGuid)
{
	Set(0.0f);
	setStyle(ImFlow::NodeStyle::green());
	addOUT<ShaderNodeReturn>((char*)u8"값")->behaviour(
		[this]()
		{
			Variable* var = new Variable();
			var->type = "float";
			var->identifier = std::format({ "c_{}" }, guid);
			var->initializationExpression = std::format({ "{}" }, value);

			ShaderNodeReturn result = CreateShaderNodeReturn();
			result->push_back(var);
			return result;
		});
}

void ConstantValueNode::Set(float value)
{
	setTitle(std::format("{}", value));
	this->value = value;
}

void ConstantValueNode::draw()
{
	ImGui::SetNextItemWidth(100);
	ImGui::PushID("x");
	if (ImGui::DragFloat("", &value, 0.01f))
	{
		Set(value);
	}
	ImGui::PopID();
}

ConstantVector2Node::ConstantVector2Node(size_t newGuid) :
	ShaderNode(newGuid)
{
	Set(value);
	setStyle(ImFlow::NodeStyle::green());
	addOUT<ShaderNodeReturn>((char*)u8"값")->behaviour(
		[this]()
		{
			Variable* var = new Variable();
			var->type = "float2";
			var->identifier = std::format({ "c_{}" }, guid);
			var->initializationExpression = std::format({ "float2({}, {})" }, value.x, value.y);

			ShaderNodeReturn result = CreateShaderNodeReturn();
			result->push_back(var);

			return result;
		});
	addOUT<ShaderNodeReturn>("x")->behaviour(
		[this]()
		{
			Variable* var = new Variable();
			var->type = "float";
			var->identifier = std::format({ "c_x{}" }, guid);
			var->initializationExpression = std::format({ "{}" }, value.x);

			ShaderNodeReturn result = CreateShaderNodeReturn();
			result->push_back(var);

			return result;
		});
	addOUT<ShaderNodeReturn>("y")->behaviour(
		[this]()
		{
			Variable* var = new Variable();
			var->type = "float";
			var->identifier = std::format({ "c_y{}" }, guid);
			var->initializationExpression = std::format({ "{}" }, value.y);

			ShaderNodeReturn result = CreateShaderNodeReturn();
			result->push_back(var);
			return result;
		});
}
void ConstantVector2Node::Set(const Vector2& value)
{
	setTitle(std::format({ "{}, {}" }, value.x, value.y));
	this->value = value;
}

void ConstantVector2Node::draw()
{
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("x");
		if (ImGui::InputFloat("", &value.x))
		{
			Set(value);
		}
		ImGui::PopID();
	}
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("y");
		if (ImGui::InputFloat("", &value.y))
		{
			Set(value);
		}
		ImGui::PopID();
	}
}

ConstantVector3Node::ConstantVector3Node(size_t newGuid) :
	ShaderNode(newGuid)
{
	Set(value);
	setStyle(ImFlow::NodeStyle::green());
	addOUT<ShaderNodeReturn>((char*)u8"값")->behaviour(
		[this]()
		{
			Variable* var = new Variable();
			var->type = "float3";
			var->identifier = std::format({ "c_{}" }, guid);
			var->initializationExpression = std::format({ "float3({}, {}, {})" }, value.x, value.y, value.z);

			ShaderNodeReturn result = CreateShaderNodeReturn();
			result->push_back(var);
			return result;
		});
	addOUT<ShaderNodeReturn>("x")->behaviour(
		[this]()
		{
			Variable* var = new Variable();
			var->type = "float";
			var->identifier = std::format({ "c_x{}" }, guid);
			var->initializationExpression = std::format({ "{}" }, value.x);

			ShaderNodeReturn result = CreateShaderNodeReturn();
			result->push_back(var);
			return result;
		});
	addOUT<ShaderNodeReturn>("y")->behaviour(
		[this]()
		{
			Variable* var = new Variable();
			var->type = "float";
			var->identifier = std::format({ "c_y{}" }, guid);
			var->initializationExpression = std::format({ "{}" }, value.y);

			ShaderNodeReturn result = CreateShaderNodeReturn();
			result->push_back(var);
			return result;
		});
	addOUT<ShaderNodeReturn>("z")->behaviour(
		[this]()
		{
			Variable* var = new Variable();
			var->type = "float";
			var->identifier = std::format({ "c_z{}" }, guid);
			var->initializationExpression = std::format({ "{}" }, value.z);

			ShaderNodeReturn result = CreateShaderNodeReturn();
			result->push_back(var);
			return result;
		});
}

void ConstantVector3Node::Set(const Vector3& value)
{
	setTitle(std::format({ "{}, {}. {}" }, value.x, value.y, value.z));
	this->value = value;
}

void ConstantVector3Node::draw()
{
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("x");
		if (ImGui::InputFloat("", &value.x))
		{
			Set(value);
		}
		ImGui::PopID();
	}
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("y");
		if (ImGui::InputFloat("", &value.y))
		{
			Set(value);
		}
		ImGui::PopID();
	}
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("z");
		if (ImGui::InputFloat("", &value.z))
		{
			Set(value);
		}
		ImGui::PopID();
	}
}

ConstantVector4Node::ConstantVector4Node(size_t newGuid) :
	ShaderNode(newGuid)
{
	Set(value);
	setStyle(ImFlow::NodeStyle::green());
	addOUT<ShaderNodeReturn>((char*)u8"값")->behaviour(
		[this]()
		{
			Variable* var = new Variable();
			var->type = "float4";
			var->identifier = std::format({ "c_{}" }, guid);
			var->initializationExpression = std::format({ "float4({}, {}, {}, {})" }, value.x, value.y, value.z, value.w);

			ShaderNodeReturn result = CreateShaderNodeReturn();
			result->push_back(var);
			return result;
		});
	addOUT<ShaderNodeReturn>("x")->behaviour(
		[this]()
		{
			Variable* var = new Variable();
			var->type = "float";
			var->identifier = std::format({ "c_x{}" }, guid);
			var->initializationExpression = std::format({ "{}" }, value.x);

			ShaderNodeReturn result = CreateShaderNodeReturn();
			result->push_back(var);
			return result;
		});
	addOUT<ShaderNodeReturn>("y")->behaviour(
		[this]()
		{
			Variable* var = new Variable();
			var->type = "float";
			var->identifier = std::format({ "c_y{}" }, guid);
			var->initializationExpression = std::format({ "{}" }, value.y);

			ShaderNodeReturn result = CreateShaderNodeReturn();
			result->push_back(var);
			return result;
		});
	addOUT<ShaderNodeReturn>("z")->behaviour(
		[this]()
		{
			Variable* var = new Variable();
			var->type = "float";
			var->identifier = std::format({ "c_z{}" }, guid);
			var->initializationExpression = std::format({ "{}" }, value.z);

			ShaderNodeReturn result = CreateShaderNodeReturn();
			result->push_back(var);
			return result;
		});
	addOUT<ShaderNodeReturn>("w")->behaviour(
		[this]()
		{
			Variable* var = new Variable();
			var->type = "float";
			var->identifier = std::format({ "c_w{}" }, guid);
			var->initializationExpression = std::format({ "{}" }, value.w);

			ShaderNodeReturn result = CreateShaderNodeReturn();
			result->push_back(var);
			return result;
		});
}

void ConstantVector4Node::Set(const Vector4& value)
{
	setTitle(std::format({ "{}, {}. {}, {}" }, value.x, value.y, value.z, value.w));
	this->value = value;
}

void ConstantVector4Node::draw()
{
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("x");
		if (ImGui::InputFloat("", &value.x))
		{
			Set(value);
		}
		ImGui::PopID();
	}
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("y");
		if (ImGui::InputFloat("", &value.y))
		{
			Set(value);
		}
		ImGui::PopID();
	}
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("z");
		if (ImGui::InputFloat("", &value.z))
		{
			Set(value);
		}
		ImGui::PopID();
	}
	{
		ImGui::SetNextItemWidth(100);
		ImGui::PushID("w");
		if (ImGui::InputFloat("", &value.w))
		{
			Set(value);
		}
		ImGui::PopID();
	}
}

ShaderResultNode::ShaderResultNode(size_t newGuid) :
	ShaderNode(newGuid)
{
	setStyle(ImFlow::NodeStyle::red());
	setTitle("BRDF");
	

	for (auto& name : EShaderResult::pinNames)
	{
		addIN<ShaderNodeReturn>(name, nullptr, ImFlow::ConnectionFilter::None());
	}
}

ShaderNodeReturn CreateShaderNodeReturn()
{
	{
		return new std::vector<ShaderDataProcess*>;
	}
}
