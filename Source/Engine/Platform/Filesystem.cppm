export module Platform.Filesystem;
import std;

export namespace Platform
{
    std::filesystem::path executablePath();

    std::filesystem::path executableDirectory() { return executablePath().parent_path(); }
}
