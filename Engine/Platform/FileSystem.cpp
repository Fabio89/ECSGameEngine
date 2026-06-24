module FileSystem;

std::filesystem::path FileSystem::executableDirectory()
{
    return executablePath().parent_path();
}