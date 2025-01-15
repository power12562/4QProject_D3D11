#include "ShaderNodes.h"

ShaderNode::ShaderNode()
{
	setStyle(ImFlow::NodeStyle::green());
	
}

ConstantValueNode::ConstantValueNode()
{
	Set(0.0f);
	setStyle(ImFlow::NodeStyle::green());
	addOUT<ShaderNodeReturn>((char*)u8"값")->behaviour(
		[this]()
		{
			Variable* var = new Variable();
			var->type = "float";
			var->identifier = std::format({ "c_{}" }, getUID());
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

void ConstantValueNode::Serialize(nlohmann::json& j)
{
	j["value"] = value;
}

void ConstantValueNode::Deserialize(const nlohmann::json& j)
{
	value = j["value"];
}

ConstantVector2Node::ConstantVector2Node()
{
	Set(value);
	setStyle(ImFlow::NodeStyle::green());
	addOUT<ShaderNodeReturn>((char*)u8"값")->behaviour(
		[this]()
		{
			Variable* var = new Variable();
			var->type = "float2";
			var->identifier = std::format({ "c_{}" }, getUID());
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
			var->identifier = std::format({ "c_x{}" }, getUID());
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
			var->identifier = std::format({ "c_y{}" }, getUID());
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

void ConstantVector2Node::Serialize(nlohmann::json& j)
{
	j["value.x"] = value.x;
	j["value.y"] = value.y;
}

void ConstantVector2Node::Deserialize(const nlohmann::json& j)
{
	value.x = j["value.x"];
	value.y = j["value.y"];
}

ConstantVector3Node::ConstantVector3Node()
{
	Set(value);
	setStyle(ImFlow::NodeStyle::green());
	addOUT<ShaderNodeReturn>((char*)u8"값")->behaviour(
		[this]()
		{
			Variable* var = new Variable();
			var->type = "float3";
			var->identifier = std::format({ "c_{}" }, getUID());
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
			var->identifier = std::format({ "c_x{}" }, getUID());
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
			var->identifier = std::format({ "c_y{}" }, getUID());
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
			var->identifier = std::format({ "c_z{}" }, getUID());
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

void ConstantVector3Node::Serialize(nlohmann::json& j)
{
	j["value.x"] = value.x;
	j["value.y"] = value.y;
	j["value.z"] = value.z;
}

void ConstantVector3Node::Deserialize(const nlohmann::json& j)
{
	value.x = j["value.x"];
	value.y = j["value.y"];
	value.z = j["value.z"];
}

ConstantVector4Node::ConstantVector4Node()
{
	Set(value);
	setStyle(ImFlow::NodeStyle::green());
	addOUT<ShaderNodeReturn>((char*)u8"값")->behaviour(
		[this]()
		{
			Variable* var = new Variable();
			var->type = "float4";
			var->identifier = std::format({ "c_{}" }, getUID());
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
			var->identifier = std::format({ "c_x{}" }, getUID());
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
			var->identifier = std::format({ "c_y{}" }, getUID());
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
			var->identifier = std::format({ "c_z{}" }, getUID());
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
			var->identifier = std::format({ "c_w{}" }, getUID());
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

void ConstantVector4Node::Serialize(nlohmann::json& j)
{
	j["value.x"] = value.x;
	j["value.y"] = value.y;
	j["value.z"] = value.z;
	j["value.w"] = value.w;
}

void ConstantVector4Node::Deserialize(const nlohmann::json& j)
{
	value.x = j["value.x"];
	value.y = j["value.y"];
	value.z = j["value.z"];
	value.w = j["value.w"];
}

ShaderResultNode::ShaderResultNode()
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
	return new std::vector<ShaderDataProcess*>;
}
