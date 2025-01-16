#include "Manager\HLSLManager.h"
#include <iostream>
#include <deque>
#include <string>
#include <sstream>
#include <regex>

class PathFormatter
{
    std::string command;
public:
    PathFormatter()
    {
        command.reserve(300);
    }
    void Input(const char* arg)
    {
        command += arg;
    }
    std::string trim(const std::string& str)
    {
        size_t start = str.find_first_not_of(" \t\n\r");
        size_t end = str.find_last_not_of(" \t\n\r");
        return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
    }
    std::pair<std::string, std::string> GetConvertedPath()
    {
        std::vector<std::string> result;
        std::stringstream ss(command);

        std::string temp;
        while (std::getline(ss, temp, '|'))
        {
            result.push_back(trim(temp));
        }

        for (auto e : result)
        {
            std::cerr << e << std::endl;
        }

        if (result.size() == 2)
        {
            return std::make_pair(result[0], result[1]);
        }
        else
        {
            std::cerr << "WTF Something went wrong!!!!!!!";
            return std::make_pair(result[0], result[1]);
        }
    }
};

int main(int argc, char* argv[])
{
    std::filesystem::path dest;
    std::filesystem::path src;
    if (argc == 2)
    {
        PathFormatter converter;
        converter.Input(argv[1]);
        auto p = converter.GetConvertedPath();

        std::filesystem::path dest = p.first;
        std::filesystem::path source = p.second;
    }
    else if (argc == 3)
    {
		dest = argv[1];
		src = argv[2];
    }
    else
    {
		std::cerr << "����: ���α׷� <dest> <source>" << std::endl;
        return 0;
    }
    if (!std::filesystem::exists(dest))
    {
        std::filesystem::create_directories(dest);
    }


    // ShaderUtility�� �̿��� ���̴� ����
    ShaderUtility::CopyShader(dest, src);

    std::cout << "���̴� ���簡 �Ϸ�Ǿ����ϴ�: " << dest << " <- " << src << "\n";


    return 0;

    // ���� �� Ȯ��
    //if (argc < 3)
    //{
    //    std::cerr << "����: ���α׷� <dest> <source>" << std::endl;
    //    std::cerr << "���� �Ķ����" << std::endl;
    //    for (int i = 0; i < argc; ++i)
    //    {
    //        std::cerr << " " << argv[i] << std::endl;
    //    }
    //    if (argc == 1)
    //    {
    //        std::cerr << "����" << std::endl;
    //    }
    //    return 1;
    //}

    //// ���� �� ���ڸ� ���� ��η� ����
    //std::filesystem::path dest = argv[1];
    //if (!std::filesystem::exists(dest))
    //{
    //    std::filesystem::create_directories(dest);
    //}

    //std::filesystem::path source = argv[2];

    //// ShaderUtility�� �̿��� ���̴� ����
    //ShaderUtility::CopyShader(dest, source);

    //std::cout << "���̴� ���簡 �Ϸ�Ǿ����ϴ�: " << dest << " <- " << source << "\n";

    //return 0;
}