using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;
using Editor.GameProject;

namespace Editor;

public static partial class EngineInterop
{
    private static readonly JsonSerializerOptions SerializerOptions = new()
    {
        Converters = { new EntityConverter(), new Vector3Converter() },
        WriteIndented = true
    };
    private static readonly StringBuilder JsonBuffer = new(4096);
    
    [DllImport("Engine.dll", EntryPoint = "getCoolestNumber", CallingConvention = CallingConvention.Cdecl)]
    public static extern int GetCoolestNumber();

    [DllImport("Engine.dll", EntryPoint = "createWindow", CallingConvention = CallingConvention.Cdecl)]
    public static extern IntPtr CreateWindow(IntPtr parentHwnd, int width, int height);

    [DllImport("Engine.dll", EntryPoint = "setViewport", CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetViewport(IntPtr window, int x, int y, int width, int height);

    [DllImport("Engine.dll", EntryPoint = "engineInit")]
    public static extern void EngineInit(IntPtr window);

    [DllImport("Engine.dll", EntryPoint = "engineUpdate", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool EngineUpdate(IntPtr window, float deltaTime);

    [DllImport("Engine.dll", EntryPoint = "engineShutdown")]
    public static extern void EngineShutdown(IntPtr window);

    [DllImport("Engine.dll", EntryPoint = "openProject", CallingConvention = CallingConvention.Cdecl)]
    public static extern void OpenProject(string path);
    
    [DllImport("Engine.dll", EntryPoint = "serializeScene", CallingConvention = CallingConvention.Cdecl)]
    private static extern void SerializeScene(StringBuilder buffer, int bufferSize);

    public static Scene LoadScene()
    {
        var buffer = JsonBuffer;
        SerializeScene(buffer, buffer.Capacity);
        var sceneStr = buffer.ToString(); 
        var scene = JsonSerializer.Deserialize<Scene>(sceneStr, SerializerOptions);
        return scene ?? new Scene();
    }
}