using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;

namespace Editor;

public partial class Viewport : UserControl
{
    [DllImport("Engine.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern int getCoolestNumber();

    [DllImport("Engine.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern IntPtr createWindow(IntPtr parentHwnd, int width, int height);
    
    [DllImport("Engine.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern void setViewportOffset(IntPtr window, int x, int y);

    [DllImport("Engine.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern void engineInit(IntPtr window);

    [DllImport("Engine.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern bool engineUpdate(IntPtr window, float deltaTime);

    [DllImport("Engine.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern void engineShutdown(IntPtr window);
    
    [DllImport("Engine.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern void loadScene(string path);
    

    private IntPtr _glfwHwnd;

    public Viewport()
    {
        InitializeComponent();
        Loaded += OnLoaded;
    }

    public static void LoadScene(string path)
    {
        loadScene(path);
    }

    private void OnUpdate(object? sender, EventArgs e)
    {
        if (_glfwHwnd == IntPtr.Zero)
            return;
            
        var keepRunning = engineUpdate(_glfwHwnd, 1f);
        if (!keepRunning)
        {
            engineShutdown(_glfwHwnd);
            Shutdown();
        }
    }

    private void OnLoaded(object sender, RoutedEventArgs e)
    {
        Loaded -= OnLoaded;
        CompositionTarget.Rendering += OnUpdate;

        InitialiseViewport();
    }

    private void InitialiseViewport()
    {
        var hwndSource = (HwndSource?)PresentationSource.FromVisual(ViewportHost) ?? throw new InvalidOperationException();

        IntPtr parentHwnd = hwndSource.Handle;

        _glfwHwnd = createWindow(parentHwnd, (int)ActualWidth, (int)ActualHeight);
        if (_glfwHwnd == IntPtr.Zero)
        {
            MessageBox.Show("Failed to create GLFW window.");
        }
        
        Window? mainWindow = Window.GetWindow(this);
        Point windowPosition = this.TransformToAncestor(mainWindow).Transform(new Point(0, 0));
        
        setViewportOffset(_glfwHwnd, (int)windowPosition.X, (int)windowPosition.Y);
        engineInit(_glfwHwnd);
    }

    public void Shutdown()
    {
        CompositionTarget.Rendering -= OnUpdate;
        if (_glfwHwnd != IntPtr.Zero)
            engineShutdown(_glfwHwnd);
    }
}