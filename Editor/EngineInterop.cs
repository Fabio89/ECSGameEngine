using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;
using Editor.GameProject;

namespace Editor;

public static partial class EngineInterop
{
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
    
    [DllImport("Engine.dll", EntryPoint = "saveCurrentProject", CallingConvention = CallingConvention.Cdecl)]
    public static extern void SaveCurrentProject();
    
    [DllImport("Engine.dll", EntryPoint = "serializeScene", CallingConvention = CallingConvention.Cdecl)]
    public static extern void SerializeScene(StringBuilder buffer, int bufferSize);
    
    [DllImport("Engine.dll", EntryPoint = "patchEntity", CallingConvention = CallingConvention.Cdecl)]
    public static extern void PatchEntity(ulong entity, string json);
}