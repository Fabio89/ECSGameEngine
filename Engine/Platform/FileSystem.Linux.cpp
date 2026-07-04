module;

#ifdef __linux__
#include <cstdio>
#include <unistd.h>
#endif

module FileSystem;

#ifdef __linux__

std::filesystem::path FileSystem::executablePath()
{
    std::array<char, 4096> buffer{};
    const ssize_t length = readlink("/proc/self/exe", buffer.data(), buffer.size() - 1);

    if (length < 0)
        throw std::runtime_error("Failed to determine executable path");

    return std::filesystem::path{std::string_view{buffer.data(), static_cast<std::size_t>(length)}};
}

std::optional<std::filesystem::path> openPathDialog(const char* command)
{
    FILE* pipe = popen(command, "r");

    if (!pipe)
        return std::nullopt;

    std::array<char, 512> buffer{};

    std::string result;

    while (fgets(buffer.data(), buffer.size(), pipe))
    {
        result += buffer.data();
    }

    const int exitCode = pclose(pipe);

    if (exitCode != 0)
        return std::nullopt;

    // zenity adds newline
    while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
    {
        result.pop_back();
    }

    if (result.empty())
        return std::nullopt;

    return std::filesystem::path(result);
}

std::optional<std::filesystem::path> FileSystem::openFileDialog()
{
    return openPathDialog("zenity --file-selection");
}

std::optional<std::filesystem::path> FileSystem::openFileDialog(std::string_view filter)
{
    std::string command = std::format("zenity --file-selection --file-filter='{}'", filter);
    return openPathDialog(command.c_str());
}

std::optional<std::filesystem::path> FileSystem::openFolderDialog()
{
    return openPathDialog("zenity --file-selection --directory");
}

#endif
