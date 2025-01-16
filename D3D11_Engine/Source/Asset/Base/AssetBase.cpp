#include "AssetBase.h"
#include <filesystem>
#include <framework.h>

void AssetBase::SaveAsset()
{
	if (assetPath == L"")
	{
		SaveAsAssetWithDialog();
	}
	else
	{
		std::ofstream ofs(assetPath.c_str(), std::ios::binary | std::ios::trunc);
		Serialized(ofs);
		ofs.close();
	}
}

void AssetBase::SaveAsAssetWithDialog()
{ 
	std::wstring savePath = WinUtility::GetSaveAsFilePath(extension.c_str());
	if (savePath != L"")
	{
		std::filesystem::path path(savePath);
		std::filesystem::path fileExtension = path.extension();
		std::wstring dotExtension = L"." + extension;
		if (fileExtension != dotExtension)
		{
			if (fileExtension != L"")
			{
				path.replace_extension(dotExtension);
			}
			else
			{
				path += dotExtension;
			}
		}

		if (!std::filesystem::exists(path))
		{
			if (!std::filesystem::exists(path.parent_path()))
			{
				std::filesystem::create_directories(path.parent_path());
			}
		}
		else
		{
			int result = MessageBox(
				NULL,
				L"파일이 존재합니다. 덮어쓰시겠습니까?",
				path.c_str(),
				MB_OKCANCEL | MB_ICONQUESTION
			);
			if (result == IDCANCEL) {
				return;
			}
		}

		assetPath = path.c_str();
		SaveAsset();
	}
}

void AssetBase::OpenAssetWithDialog()
{
	std::wstring openPath = WinUtility::GetOpenFilePath(extension.c_str());
	if (openPath != L"")
	{
		OpenAsset(openPath.c_str());
	}
}

void AssetBase::OpenAsset(const wchar_t* path)
{
	std::ifstream ifs(path, std::ios::binary);
	Deserialized(ifs);
	ifs.close();
	assetPath = path;
}
