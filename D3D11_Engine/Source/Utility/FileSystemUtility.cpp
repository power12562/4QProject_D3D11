#include "FileSystemUtility.h"

bool Utility::IsPathDirectory(const std::filesystem::path& path)
{
	// 경로를 문자열로 변환하여 마지막 문자를 확인
	std::wstring pathStr = path.native();
	return (!pathStr.empty() && (pathStr.back() == L'/' || pathStr.back() == L'\\'));
}
