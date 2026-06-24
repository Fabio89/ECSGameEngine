module;

#ifdef __linux__
#include <unistd.h>
#endif

module FileSystem;

#ifdef __linux__

std::filesystem::path FileSystem::executablePath()
{
    std::array<char, 4096> buffer{};

    const ssize_t length =
        readlink(
            "/proc/self/exe",
            buffer.data(),
            buffer.size() - 1);

    if (length < 0)
    {
        throw std::runtime_error("Failed to determine executable path");
    }

    return std::filesystem::path(
        std::string_view(
            buffer.data(),
            static_cast<std::size_t>(length)));
}

std::filesystem::path FileSystem::openFileDialog()
{
    return {};
}

#endif
