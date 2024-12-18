#pragma once
#include <string>
#include <WinNls.h>

class misc
{
public:

    std::wstring string_to_LPCWSTR( const std::string& str ) 
    {
        int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);

        std::wstring wstrTo(size_needed, 0);

        MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);

        return wstrTo;
    }
};
misc g_misc;