#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>
#include <type_traits>
#include <DirectXCollision.h>
#include <directxtk/SimpleMath.h>
#include <Material/PBRMaterial.h>

namespace Binary
{
	namespace Write
	{
		template<typename T> void data(std::ofstream& ofs, const T data)
		{
			static_assert(std::is_integral_v<T>
				|| std::is_floating_point_v<T>
				|| std::is_same_v<T, bool>,
				"Type must be a basic type (int, char, bool, size_t, unsigned long, etc.)");
			ofs.write(reinterpret_cast<const char*>(&data), sizeof(T));
		}
		void data(std::ofstream& ofs, const void* data, size_t size);
		void string(std::ofstream& ofs, const std::string& data);
		void wstring(std::ofstream& ofs, const std::wstring& data);
		void floatArray(std::ofstream& ofs, const float* data, size_t size);

		void Matrix(std::ofstream& ofs, const DirectX::SimpleMath::Matrix& data);
		void Vector2(std::ofstream& ofs, const DirectX::SimpleMath::Vector2& data);
		void Vector3(std::ofstream& ofs, const DirectX::SimpleMath::Vector3& data);
		void Vector4(std::ofstream& ofs, const DirectX::SimpleMath::Vector4& data);
		void Color(std::ofstream& ofs, const DirectX::SimpleMath::Color& data);
		void Quaternion(std::ofstream& ofs, const DirectX::SimpleMath::Quaternion& data);
		void BoundingBox(std::ofstream& ofs, const DirectX::BoundingBox& data);
		void PBRMaterial(std::ofstream& ofs, const cb_PBRMaterial& data);
	}
	namespace Read 
	{
		template<typename T> T data(std::ifstream& ifs)
		{
			static_assert(std::is_integral_v<T>
				|| std::is_floating_point_v<T>
				|| std::is_same_v<T, bool>,
				"Type must be a basic type (int, char, bool, size_t, unsigned long, etc.)");
			T data{};
			ifs.read(reinterpret_cast<char*>(&data), sizeof(T));
			return data;
		}
		void data(std::ifstream& ifs, void* out, size_t size);

		std::string string(std::ifstream& ifs);
		std::wstring wstring(std::ifstream& ifs);
		void floatArray(std::ifstream& ifs, float* out, size_t size);

		DirectX::SimpleMath::Matrix Matrix(std::ifstream& ifs);
		DirectX::SimpleMath::Vector2 Vector2(std::ifstream& ifs);
		DirectX::SimpleMath::Vector3 Vector3(std::ifstream& ifs);
		DirectX::SimpleMath::Vector4 Vector4(std::ifstream& ifs);
		DirectX::SimpleMath::Color Color(std::ifstream& ifs);
		DirectX::SimpleMath::Quaternion Quaternion(std::ifstream& ifs);
		DirectX::BoundingBox BoundingBox(std::ifstream& ifs);
		cb_PBRMaterial PBRMaterial(std::ifstream& ifs);

	}
}