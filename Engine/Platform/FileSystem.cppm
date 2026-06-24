export module FileSystem;
import std;

export namespace FileSystem
{
    std::filesystem::path executablePath();

    std::filesystem::path executableDirectory();

    std::filesystem::path openFileDialog();
}
