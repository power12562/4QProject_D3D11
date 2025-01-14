#include "Manager\HLSLManager.h"
#include <iostream>


int main(int argc, char* argv[])
{
    // 인자 수 확인
    if (argc < 3)
    {
        std::cerr << "사용법: 프로그램 <dest> <source>" << std::endl;
        std::cerr << "현재 파라미터" << std::endl;
		for (int i = 0; i < argc; ++i)
		{
			std::cerr << " " << argv[i] << std::endl;
		}
        if (argc == 1)
        {
            std::cerr << "없음" << std::endl;
        }
        return 1;
    }

    // 실행 시 인자를 파일 경로로 설정
    std::filesystem::path dest = argv[1];
    if (!std::filesystem::exists(dest))
    {
        std::filesystem::create_directories(dest);
    }

    std::filesystem::path source = argv[2];

    // ShaderUtility를 이용해 셰이더 복사
    ShaderUtility::CopyShader(dest, source);

    std::cout << "셰이더 복사가 완료되었습니다: " << dest << " <- " << source << "\n";

    return 0;
}