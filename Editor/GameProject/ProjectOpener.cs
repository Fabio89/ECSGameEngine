using System.IO;

namespace Editor.GameProject;

public static class ProjectOpener
{
    public static event Action Opened = delegate { };
    public static string CurrentProjectPath { get; private set; } = string.Empty;
    
    public static bool OpenProject(string path)
    {
        if (!Viewport.OpenProject(path)) return false;
        Console.WriteLine($"Opened project: '{path}'");
        CurrentProjectPath = path;
        Opened.Invoke();
        return true;
    }
}