export module FileSystem;
import std;

export namespace FileSystem
{
    std::filesystem::path executablePath();

    std::filesystem::path executableDirectory();

    std::optional<std::filesystem::path> openFileDialog();

    std::optional<std::filesystem::path> openFileDialog(std::string_view filter);

    std::optional<std::filesystem::path> openFolderDialog();
}
