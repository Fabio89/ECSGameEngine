using System.IO;

namespace Editor.GameProject;

public class ProjectOpener : ViewModelBase
{
    private string _projectPath = string.Empty;

    public string ProjectFilePath
    {
        get => _projectPath;
        set => SetField(ref _projectPath, value);
    }
    
    public static void OpenProject(string path)
    {
        Console.WriteLine($"Opened project: {path}");
        Viewport.LoadScene(path);
    }
}