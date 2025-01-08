#include "utfConvert.h"
#pragma warning(disable : 4333)

std::string utfConvert::wstring_to_utf8(const std::wstring& wstr)
{
    std::string result;
    for (wchar_t wc : wstr)
    {
        if (wc <= 0x7F)
        {
            result.push_back(static_cast<char>(wc));
        }
        else if (wc <= 0x7FF)
        {
            result.push_back(static_cast<char>(0xC0 | ((wc >> 6) & 0x1F)));
            result.push_back(static_cast<char>(0x80 | (wc & 0x3F)));
        }
        else if (wc <= 0xFFFF)
        {
            result.push_back(static_cast<char>(0xE0 | ((wc >> 12) & 0x0F)));
            result.push_back(static_cast<char>(0x80 | ((wc >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (wc & 0x3F)));
        }
        else if (wc <= 0x10FFFF)
        {
            result.push_back(static_cast<char>(0xF0 | ((wc >> 18) & 0x07)));
            result.push_back(static_cast<char>(0x80 | ((wc >> 12) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | ((wc >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (wc & 0x3F)));
        }
        else {
            // Invalid Unicode code point, handle error appropriately
        }
    }
    return result;
}

std::wstring utfConvert::utf8_to_wstring(const std::string& str)
{
    std::wstring result;
    size_t i = 0;

    while (i < str.size())
    {
        wchar_t wc = 0;
        unsigned char c = str[i];

        if (c <= 0x7F)
        {
            wc = c;
            i += 1;
        }
        else if (c <= 0xBF)
        {
            throw std::invalid_argument("Invalid UTF-8 sequence");
        }
        else if (c <= 0xDF)
        {
            if (i + 1 >= str.size()) throw std::invalid_argument("Invalid UTF-8 sequence");
            wc = ((c & 0x1F) << 6) | (str[i + 1] & 0x3F);
            i += 2;
        }
        else if (c <= 0xEF)
        {
            if (i + 2 >= str.size()) throw std::invalid_argument("Invalid UTF-8 sequence");
            wc = ((c & 0x0F) << 12) | ((str[i + 1] & 0x3F) << 6) | (str[i + 2] & 0x3F);
            i += 3;
        }
        else if (c <= 0xF7)
        {
            if (i + 3 >= str.size()) throw std::invalid_argument("Invalid UTF-8 sequence");
            wc = ((c & 0x07) << 18) | ((str[i + 1] & 0x3F) << 12) | ((str[i + 2] & 0x3F) << 6) | (str[i + 3] & 0x3F);
            i += 4;
        }
        else
        {
            throw std::invalid_argument("Invalid UTF-8 sequence");
        }

        result.push_back(wc);
    }

    return result;
}

