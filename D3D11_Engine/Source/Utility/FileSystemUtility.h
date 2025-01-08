#pragma once
#include <iostream>
#include <fstream>
#include <filesystem>

namespace Utility
{
    //경로가 디렉토리인지 확인합니다
    bool IsPathDirectory(const std::filesystem::path& path);
}