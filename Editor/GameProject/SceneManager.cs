using System.Text;
using System.Text.Json;

namespace Editor.GameProject;

public class SceneManager
{
    private static SceneManager? _instance;
    public static SceneManager Instance => _instance ??= new SceneManager();

 
    private readonly StringBuilder _jsonBuffer = new(4096);

    public event Action<Scene> Loaded = delegate { };
    
    public Scene LoadCurrentScene()
    {
        try
        {
            _jsonBuffer.Clear();
            Engine.SerializeScene(_jsonBuffer, _jsonBuffer.Capacity);
            var sceneStr = _jsonBuffer.ToString();
            var scene = Editor.Utils.Deserialize<Scene>(sceneStr) ?? new Scene();
            Loaded.Invoke(scene);
            return scene;
        }
        catch (Exception e)
        {
            Console.WriteLine(e);
            var scene = new Scene();
            Loaded.Invoke(scene);
            return scene;
        }
    }
    
    private SceneManager() { }
}