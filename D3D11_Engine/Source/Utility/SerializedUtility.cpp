#include "SerializedUtility.h"

void Binary::Write::data(std::ofstream& ofs, const void* data, size_t size)
{
	ofs.write(reinterpret_cast<const char*>(data), size);
}

void Binary::Read::data(std::ifstream& ifs, void* out, size_t size)
{
	ifs.read(reinterpret_cast<char*>(out), size);
}

void Binary::Write::string(std::ofstream& ofs, const std::string& data)
{
	size_t len = data.length();
	ofs.write(reinterpret_cast<const char*>(&len), sizeof(size_t));
	ofs.write(data.c_str(), len);
	return;
}

std::string Binary::Read::string(std::ifstream& ifs)
{
	std::string data;
	size_t len = 0;
	ifs.read(reinterpret_cast<char*>(&len), sizeof(size_t));
	data.resize(len);
	ifs.read(reinterpret_cast<char*>(data.data()), len);
	return data;
}

void Binary::Write::wstring(std::ofstream& ofs, const std::wstring& data)
{
	size_t len = data.length();
	ofs.write(reinterpret_cast<const char*>(&len), sizeof(size_t));
	ofs.write(reinterpret_cast<const char*>(data.c_str()), len * sizeof(wchar_t));
	return;
}

std::wstring Binary::Read::wstring(std::ifstream& ifs)
{
	std::wstring data;
	size_t len = 0;
	ifs.read(reinterpret_cast<char*>(&len), sizeof(size_t));
	data.resize(len);
	ifs.read(reinterpret_cast<char*>(data.data()), len * sizeof(wchar_t));
	return data;
}

void Binary::Write::floatArray(std::ofstream& ofs, const float* data, size_t size)
{
	ofs.write(reinterpret_cast<const char*>(data), sizeof(float) * size);
}

void Binary::Read::floatArray(std::ifstream& ifs, float* out, size_t size)
{
	ifs.read(reinterpret_cast<char*>(out), sizeof(float) * size);
}

void Binary::Write::Matrix(std::ofstream& ofs, const DirectX::SimpleMath::Matrix& data)
{
	ofs.write(reinterpret_cast<const char*>(&data), sizeof(DirectX::SimpleMath::Matrix));
}

DirectX::SimpleMath::Matrix Binary::Read::Matrix(std::ifstream& ifs)
{
	DirectX::SimpleMath::Matrix matrix;
	ifs.read(reinterpret_cast<char*>(&matrix), sizeof(DirectX::SimpleMath::Matrix));
	return DirectX::SimpleMath::Matrix();
}


void Binary::Write::Vector2(std::ofstream& ofs, const DirectX::SimpleMath::Vector2& data)
{
	Write::floatArray(ofs, reinterpret_cast<const float*>(&data), 2);
}

DirectX::SimpleMath::Vector2 Binary::Read::Vector2(std::ifstream& ifs)
{
	float data[2]{};
	Read::floatArray(ifs, data, 2);
	return DirectX::SimpleMath::Vector2(data);
}

void Binary::Write::Vector3(std::ofstream& ofs, const DirectX::SimpleMath::Vector3& data)
{
	Write::floatArray(ofs, reinterpret_cast<const float*>(&data), 3);
}

void Binary::Write::Vector4(std::ofstream& ofs, const DirectX::SimpleMath::Vector4& data)
{
	Write::floatArray(ofs, reinterpret_cast<const float*>(&data), 4);
}

DirectX::SimpleMath::Vector4 Binary::Read::Vector4(std::ifstream& ifs)
{
	float data[4]{};
	Read::floatArray(ifs, data, 4);
	return DirectX::SimpleMath::Vector4(data);
}

DirectX::SimpleMath::Vector3 Binary::Read::Vector3(std::ifstream& ifs)
{
	float data[3]{};
	Read::floatArray(ifs, data, 3);
	return DirectX::SimpleMath::Vector3(data);
}

void Binary::Write::Color(std::ofstream& ofs, const DirectX::SimpleMath::Color& data)
{
	Write::floatArray(ofs, reinterpret_cast<const float*>(&data), 4);
}

DirectX::SimpleMath::Color Binary::Read::Color(std::ifstream& ifs)
{
	float data[4]{};
	Read::floatArray(ifs, data, 4);
	return DirectX::SimpleMath::Color(data);
}

void Binary::Write::Quaternion(std::ofstream& ofs, const DirectX::SimpleMath::Quaternion& data)
{
	Write::floatArray(ofs, reinterpret_cast<const float*>(&data), 4);
}

DirectX::SimpleMath::Quaternion Binary::Read::Quaternion(std::ifstream& ifs)
{
	float data[4]{};
	Read::floatArray(ifs, data, 4);
	return DirectX::SimpleMath::Quaternion(data);
}

void Binary::Write::BoundingBox(std::ofstream& ofs, const DirectX::BoundingBox& data)
{
	Write::floatArray(ofs, reinterpret_cast<const float*>(&data.Center), 3);
	Write::floatArray(ofs, reinterpret_cast<const float*>(&data.Extents), 3);
}

DirectX::BoundingBox Binary::Read::BoundingBox(std::ifstream& ifs)
{
	float Center[3]{};
	Read::floatArray(ifs, Center, 3);
	float Extents[3]{};
	Read::floatArray(ifs, Extents, 3);
	DirectX::BoundingBox data{ DirectX::XMFLOAT3(Center), DirectX::XMFLOAT3(Extents) };
	return data;
}

void Binary::Write::PBRMaterial(std::ofstream& ofs, const cb_PBRMaterial& data)
{
	Write::Color(ofs, data.Albedo);
	Write::data(ofs, data.Metalness);
	Write::data(ofs, data.Roughness);
	Write::data(ofs, static_cast<bool>(data.UseMetalnessMap));
	Write::data(ofs, static_cast<bool>(data.UseRoughnessMap));
	Write::data(ofs, static_cast<bool>(data.UseRMACMap));
}

cb_PBRMaterial Binary::Read::PBRMaterial(std::ifstream& ifs)
{
	cb_PBRMaterial data;
	data.Albedo = Read::Color(ifs);
	data.Metalness = Read::data<float>(ifs);
	data.Roughness = Read::data<float>(ifs);
	data.UseMetalnessMap = Read::data<bool>(ifs);
	data.UseRoughnessMap = Read::data<bool>(ifs);
	data.UseRMACMap = Read::data<bool>(ifs);
	return data;
}
