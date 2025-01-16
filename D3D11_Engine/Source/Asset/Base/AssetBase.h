#pragma once
#include <string>
#include <istream>
#include <ostream>

class AssetBase abstract
{
public:
	/*Extension*/
	AssetBase(const wchar_t* AssetExtension)
		: extension(AssetExtension) {}
	virtual ~AssetBase() = default;
private:
	const std::wstring extension; //»Æ¿Â¿⁄
	std::wstring assetPath;

public:
	void SaveAsset();
	void SaveAsAssetWithDialog();
	void OpenAssetWithDialog();
	void OpenAsset(const wchar_t* path);
	const std::wstring& GetAssetPath() { return assetPath; }

protected:
	virtual void Serialized(std::ofstream& ofs) = 0;
	virtual void Deserialized(std::ifstream& ifs) = 0;
};