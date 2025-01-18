#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImNodeFlow.h>

#include <string>
#include <format>
#include <directxtk\SimpleMath.h>
#include "json.hpp"
#include "NodeFactory.h"
#include "Texture.h"

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
	/** 텍스처용 임시 */
	std::filesystem::path path;
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

template<typename T>
struct ShaderPin
{
	std::shared_ptr<Variable> value;
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

	static void UnLabelPinRenderer(ImFlow::Pin* p);
	static std::function<bool(ImFlow::Pin*, ImFlow::Pin*)> SameType();
	static std::function<bool(ImFlow::Pin*, ImFlow::Pin*)> SameTypeBrotherPin();

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
	virtual ~TextureNode();

public:
	void Set(const std::filesystem::path& value);
	void draw() override;
	virtual void Serialize(nlohmann::json& j) override;
	virtual void Deserialize(const nlohmann::json& j) override;

private:
	std::filesystem::path texturePath;
	Texture texture;
};


class AddNode : public ShaderNode
{
public:
	AddNode();
	virtual ~AddNode() = default;
};

class SubNode : public ShaderNode
{
public:
	SubNode();
	virtual ~SubNode() = default;
};

class MullNode : public ShaderNode
{
public:
	MullNode();
	virtual ~MullNode() = default;
};

class DivNode : public ShaderNode
{
public:
	DivNode();
	virtual ~DivNode() = default;
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
		(const char*)u8"알베도",
		(const char*)u8"노말",
		(const char*)u8"메탈릭",
		(const char*)u8"거칠기",
		(const char*)u8"알파",
		(const char*)u8"스펙큘러",
		(const char*)u8"환경 차단",
		(const char*)u8"이미시브"
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
	template<typename T>
	std::shared_ptr<T> Create(std::string_view typeName)
	{
		return std::dynamic_pointer_cast<T>(nodeFactory.Create(typeName));
	}
	class ShaderResultNode* GetResultNode() { return resultNode; }
	ShaderNodeReturn& GetShaderNodeReturn() { return shaderNodeReturn; }

private:
	ShaderNodeFactory nodeFactory;
	class ShaderResultNode* resultNode;
	ShaderNodeReturn shaderNodeReturn;
};