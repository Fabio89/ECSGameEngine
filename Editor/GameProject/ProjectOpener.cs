namespace Editor.GameProject;

public class ProjectOpener
{
    private static ProjectOpener? _instance;
    public static ProjectOpener Instance => _instance ??= new ProjectOpener();
    
    public event Action Opened = delegate { };
    public string CurrentProjectPath { get; private set; } = string.Empty;
    
    public bool OpenProject(string path)
    {
        if (!Viewport.OpenProject(path)) return false;
        Console.WriteLine($"Opened project: '{path}'");
        CurrentProjectPath = path;
        Opened.Invoke();
        return true;
    }
    
    private ProjectOpener() { }
}