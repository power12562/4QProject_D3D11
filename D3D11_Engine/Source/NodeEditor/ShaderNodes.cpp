#include "ShaderNodes.h"
#include "Utility/WinUtility.h"

ShaderNode::ShaderNode()
{
	setStyle(ImFlow::NodeStyle::green());
	
}

NodeFlow* ShaderNode::GetHandler()
{
	return dynamic_cast<NodeFlow*>(getHandler());
}

ConstantValueNode::ConstantValueNode()
{
	Set(0.0f);
	setStyle(ImFlow::NodeStyle::green());
	addOUT<ShaderPin>((char*)u8"값")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value);


			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().result = var;

			return 0;
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
	addOUT<ShaderPin>((char*)u8"값")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float2";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "float2({}, {})" }, value.x, value.y);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().result = var;

			return 0;
		});
	addOUT<ShaderPin>("x")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_x{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.x);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().result = var;

			return 0;
		});
	addOUT<ShaderPin>("y")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_y{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.y);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().result = var;

			return 0;
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
	addOUT<ShaderPin>((char*)u8"값")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float3";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "float3({}, {}, {})" }, value.x, value.y, value.z);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().result = var;

			return 0;
		});
	addOUT<ShaderPin>("x")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_x{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.x);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().result = var;

			return 0;
		});
	addOUT<ShaderPin>("y")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_y{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.y);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().result = var;

			return 0;
		});
	addOUT<ShaderPin>("z")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_z{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.z);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().result = var;

			return 0;
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
	addOUT<ShaderPin>((char*)u8"값")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float4";
			var->identifier = std::format({ "c_{}" }, getUID());
			var->initializationExpression = std::format({ "float4({}, {}, {}, {})" }, value.x, value.y, value.z, value.w);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().result = var;

			return 0;
		});
	addOUT<ShaderPin>("x")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_x{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.x);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().result = var;

			return 0;
		});
	addOUT<ShaderPin>("y")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_y{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.y);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().result = var;

			return 0;
		});
	addOUT<ShaderPin>("z")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_z{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.z);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().result = var;

			return 0;
		});
	addOUT<ShaderPin>("w")->behaviour(
		[this]()
		{
			auto var = std::make_shared<LocalVariable>();
			var->type = "float";
			var->identifier = std::format({ "c_w{}" }, getUID());
			var->initializationExpression = std::format({ "{}" }, value.w);

			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().result = var;

			return 0;
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

TextureNode::TextureNode()
{
	setTitle("Texture Sample");
	Set(texturePath);
	setStyle(ImFlow::NodeStyle::cyan());

	addIN<ShaderPin>("uv", 0, ImFlow::ConnectionFilter::None());

	addOUT<ShaderPin>((char*)u8"RGB")->behaviour(
		[this]()
		{
			auto var = std::make_shared<RegistorVariable>();
			var->type = "Texture2D";
			var->identifier = std::format({ "t_{}" }, getUID());
			var->registorSlot = ERegisterSlot::Texture;

			auto var2 = std::make_shared<LocalVariable>();
			var2->type = "float3";
			var2->identifier = std::format({ "c_{}" }, getUID());
			var2->initializationExpression = std::format({ " {}.Sample(DefaultSampler, input.Tex).rgb" }, var->identifier);


			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var2);
			GetHandler()->GetShaderNodeReturn().result = var2;

			return 0;
		});
	addOUT<ShaderPin>((char*)u8"r")->behaviour(
		[this]()
		{
			auto var = std::make_shared<RegistorVariable>();
			var->type = "Texture2D";
			var->identifier = std::format({ "t_{}" }, getUID());
			var->registorSlot = ERegisterSlot::Texture;

			auto var2 = std::make_shared<LocalVariable>();
			var2->type = "float3";
			var2->identifier = std::format({ "c_r{}" }, getUID());
			var2->initializationExpression = std::format({ " {}.Sample(DefaultSampler, input.Tex).r" }, var->identifier);


			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var2);
			GetHandler()->GetShaderNodeReturn().result = var2;

			return 0;
		});
	addOUT<ShaderPin>((char*)u8"g")->behaviour(
		[this]()
		{
			auto var = std::make_shared<RegistorVariable>();
			var->type = "Texture2D";
			var->identifier = std::format({ "t_{}" }, getUID());
			var->registorSlot = ERegisterSlot::Texture;

			auto var2 = std::make_shared<LocalVariable>();
			var2->type = "float3";
			var2->identifier = std::format({ "c_g{}" }, getUID());
			var2->initializationExpression = std::format({ " {}.Sample(DefaultSampler, input.Tex).g" }, var->identifier);


			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var2);
			GetHandler()->GetShaderNodeReturn().result = var2;

			return 0;
		});
	addOUT<ShaderPin>((char*)u8"B")->behaviour(
		[this]()
		{
			auto var = std::make_shared<RegistorVariable>();
			var->type = "Texture2D";
			var->identifier = std::format({ "t_{}" }, getUID());
			var->registorSlot = ERegisterSlot::Texture;

			auto var2 = std::make_shared<LocalVariable>();
			var2->type = "float";
			var2->identifier = std::format({ "c_B{}" }, getUID());
			var2->initializationExpression = std::format({ " {}.Sample(DefaultSampler, input.Tex).b" }, var->identifier);


			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var2);
			GetHandler()->GetShaderNodeReturn().result = var2;

			return 0;
		});
	addOUT<ShaderPin>((char*)u8"RGBA")->behaviour(
		[this]()
		{
			auto var = std::make_shared<RegistorVariable>();
			var->type = "Texture2D";
			var->identifier = std::format({ "t_{}" }, getUID());
			var->registorSlot = ERegisterSlot::Texture;

			auto var2 = std::make_shared<LocalVariable>();
			var2->type = "float4";
			var2->identifier = std::format({ "c_RGBA{}" }, getUID());
			var2->initializationExpression = std::format({ " {}.Sample(DefaultSampler, input.Tex).rgba" }, var->identifier);


			GetHandler()->GetShaderNodeReturn().data.emplace_back(var);
			GetHandler()->GetShaderNodeReturn().data.emplace_back(var2);
			GetHandler()->GetShaderNodeReturn().result = var2;

			return 0;
		});


}

TextureNode::~TextureNode()
{
	if ((ID3D11ShaderResourceView*)texture)
	{
		texture.~Texture();
		textureManager.ReleaseSharingTexture(texturePath.c_str());
	}
}

void TextureNode::Set(const std::filesystem::path& value)
{
	if (!std::filesystem::exists(value))
	{
		return;
	}
	if ((ID3D11ShaderResourceView*)texture)
	{
		texture.~Texture();
		textureManager.ReleaseSharingTexture(texturePath.c_str());
	}

	texturePath = value;

	ComPtr<ID3D11ShaderResourceView> srv;
	textureManager.CreateSharingTexture(value.c_str(), &srv);
	srv->AddRef();

	texture.LoadTexture(srv.Get());
}

void TextureNode::draw()
{
	if (ImGui::Button("Load Texture"))
	{
		auto path = WinUtility::GetOpenFilePath();
		if (!path.empty())
		{
			Set(path);
		}
	}
	ImGui::Image((ID3D11ShaderResourceView*)texture, { 100, 100 });
}

void TextureNode::Serialize(nlohmann::json& j)
{
	j["path"] = texturePath.string();
}

void TextureNode::Deserialize(const nlohmann::json& j)
{
	if (j.find("path") != j.cend())
	{
		Set(j["path"].get<std::string>());
	}
}


ShaderResultNode::ShaderResultNode()
{
	setStyle(ImFlow::NodeStyle::red());
	setTitle("BRDF");
	

	for (auto& name : EShaderResult::pinNames)
	{
		addIN<ShaderPin>(name, 0, ImFlow::ConnectionFilter::None());
	}
}

NodeFlow::NodeFlow()
{
	nodeFactory.Set(this);
	resultNode = addNode<ShaderResultNode>({ 100, 100 }).get();
}

NodeFlow::~NodeFlow() = default;