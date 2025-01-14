#include "Manager\HLSLManager.h"
#include <iostream>


int main(int argc, char* argv[])
{
    // ���� �� Ȯ��
    if (argc < 3)
    {
        std::cerr << "����: ���α׷� <dest> <source>" << std::endl;
        std::cerr << "���� �Ķ����" << std::endl;
		for (int i = 0; i < argc; ++i)
		{
			std::cerr << " " << argv[i] << std::endl;
		}
        if (argc == 1)
        {
            std::cerr << "����" << std::endl;
        }
        return 1;
    }

    // ���� �� ���ڸ� ���� ��η� ����
    std::filesystem::path dest = argv[1];
    if (!std::filesystem::exists(dest))
    {
        std::filesystem::create_directories(dest);
    }

    std::filesystem::path source = argv[2];

    // ShaderUtility�� �̿��� ���̴� ����
    ShaderUtility::CopyShader(dest, source);

    std::cout << "���̴� ���簡 �Ϸ�Ǿ����ϴ�: " << dest << " <- " << source << "\n";

    return 0;
}