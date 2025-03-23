module;
#include <windows.h>
export module Wrapper.Windows;
import std;

export using ::HWND;
export namespace Wrapper_Windows
{
    std::string getModuleFileName()
    {
        std::string buffer;
        buffer.resize(MAX_PATH);
        GetModuleFileNameA(nullptr, buffer.data(), MAX_PATH);
        return buffer;
    }

    using ::SetParent;
    using ::GetWindowLongA;
    using ::SetWindowLongA;
}




