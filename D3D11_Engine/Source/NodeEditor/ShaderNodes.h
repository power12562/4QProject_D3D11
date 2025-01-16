#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImNodeFlow.h>

#include <string>
#include <format>
#include <directxtk\SimpleMath.h>
#include "json.hpp"
#include "NodeFactory.h"


using ShaderPin = int;

using Vector2 = DirectX::SimpleMath::Vector2;
using Vector3 = DirectX::SimpleMath::Vector3;
using Vector4 = DirectX::SimpleMath::Vector4;

namespace ERegisterSlot
{
	enum Type
	{
		Texture,
		Constant,
		Sampler,
		UnorderedAccess,
		MAX
	};

	static std::string ToString(Type type)
	{
		switch (type)
		{
		case Texture: return "t";
		case Constant: return "b";
		case Sampler: return "s";
		case UnorderedAccess: return "u";
		}
		return {};
	}
}


struct ShaderDataProcess
{
	virtual ~ShaderDataProcess() = default;
};

struct Variable : public ShaderDataProcess
{
	std::string type;
	std::string identifier;
};

struct LocalVariable : public Variable
{
	std::string initializationExpression;
};

struct RegistorVariable : public Variable
{
	ERegisterSlot::Type registorSlot;
	int slotNum;
};

struct Execution : public ShaderDataProcess
{
	std::string leftIdentifier;
	std::string rightIdentifier;
};

struct Define : public ShaderDataProcess
{
	std::string name;
	std::string initializationExpression;
};

static std::ostream& operator<<(std::ostream& os, const LocalVariable& data)
{
	return os << std::format("{} {} = {};", data.type, data.identifier, data.initializationExpression);
}

static std::ostream& operator<<(std::ostream& os, const RegistorVariable& data)
{
	return os << std::format("{} {} : register({}{});", data.type, data.identifier, ERegisterSlot::ToString(data.registorSlot), data.slotNum);
}

static std::ostream& operator<<(std::ostream& os, const Execution& data)
{
	return os << std::format("{} = {};", data.leftIdentifier, data.rightIdentifier);
}

static std::ostream& operator<<(std::ostream& os, const Define& data)
{
	return os << std::format("#define {} {}", data.name, data.initializationExpression);
}

struct ShaderNodeReturn
{
	std::vector<std::shared_ptr<ShaderDataProcess>> data;

	/** �ֱ� ����� ����� */
	std::shared_ptr<Variable> result;
};

class ISerializable
{
public:
	virtual void Serialize(nlohmann::json& j) = 0;
	virtual void Deserialize(const nlohmann::json& j) = 0;
};

class ShaderNode : public ImFlow::BaseNode, public ISerializable
{
public:
	ShaderNode();
	virtual ~ShaderNode() = default;
	class NodeFlow* GetHandler();
public:
	virtual void Serialize(nlohmann::json& j){}
	virtual void Deserialize(const nlohmann::json& j){}
};

class ConstantValueNode : public ShaderNode
{
public:
	ConstantValueNode();
	virtual ~ConstantValueNode() = default;

public:
	void Set(float value);
	void draw() override;
	virtual void Serialize(nlohmann::json& j) override;
	virtual void Deserialize(const nlohmann::json& j) override;

private:
	float value;
};

class ConstantVector2Node : public ShaderNode
{
public:
	ConstantVector2Node();
	virtual ~ConstantVector2Node() = default;

public:
	void Set(const Vector2& value);
	void draw() override;
	virtual void Serialize(nlohmann::json& j) override;
	virtual void Deserialize(const nlohmann::json& j) override;

private:
	Vector2 value;
};


class ConstantVector3Node : public ShaderNode
{
public:
	ConstantVector3Node();
	virtual ~ConstantVector3Node() = default;

public:
	void Set(const Vector3& value);
	void draw() override;
	virtual void Serialize(nlohmann::json& j) override;
	virtual void Deserialize(const nlohmann::json& j) override;

private:
	Vector3 value;
};


class ConstantVector4Node : public ShaderNode
{
public:
	ConstantVector4Node();
	virtual ~ConstantVector4Node() = default;

public:
	void Set(const Vector4& value);
	void draw() override;
	virtual void Serialize(nlohmann::json& j) override;
	virtual void Deserialize(const nlohmann::json& j) override;

private:
	Vector4 value;
};


class TextureNode : public ShaderNode
{
public:
	TextureNode();
	virtual ~TextureNode() = default;

public:
	void Set(std::string& value);
	void draw() override;
	virtual void Serialize(nlohmann::json& j) override;
	virtual void Deserialize(const nlohmann::json& j) override;

private:
	std::string texturePath;
};


namespace EShaderResult
{
	enum Type
	{
		Albedo,
		Normal,
		Metallic,
		Roughness,
		Alpha,
		Specular,
		EnvironmentOcclusion,
		Emissive,
		MAX
	};
	static const char* pinNames[EShaderResult::MAX] =
	{
		(const char*)u8"�˺���",
		(const char*)u8"�븻",
		(const char*)u8"��Ż��",
		(const char*)u8"��ĥ��",
		(const char*)u8"����",
		(const char*)u8"����ŧ��",
		(const char*)u8"ȯ�� ����",
		(const char*)u8"�̹̽ú�"
	};
	static const char* hlslName[EShaderResult::MAX] =
	{
		"material.albedo",
		"material.normal",
		"material.metallic",
		"material.roughness",
		"material.alpha",
		"material.specular",
		"material.ambiantOcclusion",
		"material.emissive"
	};
}

class ShaderResultNode : public ShaderNode
{
public:
	ShaderResultNode();


	virtual void destroy() override { }
};


class NodeFlow : public ImFlow::ImNodeFlow
{
public:
	NodeFlow();
	virtual ~NodeFlow();
public:

	std::shared_ptr<ShaderNode> Create(std::string_view typeName);
	class ShaderResultNode* GetResultNode() { return resultNode; }
	ShaderNodeReturn& GetShaderNodeReturn() { return shaderNodeReturn; }

private:
	ShaderNodeFactory nodeFactory;
	class ShaderResultNode* resultNode;
	ShaderNodeReturn shaderNodeReturn;
};