#include "Source\Manager\HLSLManager.h"
#include <iostream>


int main(int argc, char* argv[])
{
    // ���� �� Ȯ��
    if (argc < 3)
    {
        std::cerr << "����: ���α׷� <dest> <source>\n";
        return 1;
    }

    // ���� �� ���ڸ� ���� ��η� ����
    std::filesystem::path dest = argv[1];
    std::filesystem::path source = argv[2];

    // ShaderUtility�� �̿��� ���̴� ����
    ShaderUtility::CopyShader(dest, source);

    std::cout << "���̴� ���簡 �Ϸ�Ǿ����ϴ�: " << dest << " <- " << source << "\n";

    return 0;
}