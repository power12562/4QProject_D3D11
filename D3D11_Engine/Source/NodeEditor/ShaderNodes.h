#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImNodeFlow.h>

#include <string>
#include <format>
#include <directxtk\SimpleMath.h>
#include "json.hpp"
using Vector2 = DirectX::SimpleMath::Vector2;
using Vector3 = DirectX::SimpleMath::Vector3;
using Vector4 = DirectX::SimpleMath::Vector4;


struct ShaderDataProcess
{
	virtual ~ShaderDataProcess() = default;
};

struct Variable : public ShaderDataProcess
{
	std::string type;
	std::string identifier;
	std::string initializationExpression;
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

static std::ostream& operator<<(std::ostream& os, const Variable& data)
{
	return os << std::format("{} {} = {};", data.type, data.identifier, data.initializationExpression);
}

static std::ostream& operator<<(std::ostream& os, const Execution& data)
{
	return os << std::format("{} = {};", data.leftIdentifier, data.rightIdentifier);
}

static std::ostream& operator<<(std::ostream& os, const Define& data)
{
	return os << std::format("#define {} {}", data.name, data.initializationExpression);
}

using ShaderNodeReturn = std::vector<ShaderDataProcess*>*;
static ShaderNodeReturn CreateShaderNodeReturn();

class ShaderNode : public ImFlow::BaseNode
{
public:
	ShaderNode(size_t guid);
	virtual void Serialize(nlohmann::json& j){}
	virtual void Deserialize(const nlohmann::json& j){}

protected:
	size_t guid;
};

class ConstantValueNode : public ShaderNode
{
public:
	ConstantValueNode(size_t newGuid);

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
	ConstantVector2Node(size_t newGuid);

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
	ConstantVector3Node(size_t newGuid);

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
	ConstantVector4Node(size_t newGuid);

public:
	void Set(const Vector4& value);
	void draw() override;
	virtual void Serialize(nlohmann::json& j) override;
	virtual void Deserialize(const nlohmann::json& j) override;

private:
	Vector4 value;
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
	ShaderResultNode(size_t newGuid);


	virtual void destroy(){ }

};
