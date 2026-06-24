module;

#ifdef _WIN32
#include <windows.h>
#endif

module FileSystem;

#ifdef _WIN32

std::filesystem::path FileSystem::executablePath()
{
    std::wstring buffer(MAX_PATH, L'\0');

    DWORD length = GetModuleFileNameW(
        nullptr,
        buffer.data(),
        static_cast<DWORD>(buffer.size()));

    buffer.resize(length);

    return std::filesystem::path(buffer);
}

#endif