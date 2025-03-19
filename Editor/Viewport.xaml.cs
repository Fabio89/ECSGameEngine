using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;

namespace Editor;

public partial class Viewport : UserControl
{
    public event Action EngineInitialised = delegate { };

    private IntPtr _glfwHwnd = IntPtr.Zero;

    private bool IsEngineInitialised()
    {
        return _glfwHwnd != IntPtr.Zero;
    }

    private void UpdateViewportExtents()
    {
        Window mainWindow = Window.GetWindow(this) ?? throw new InvalidOperationException("Could not find main window.");
        Point windowPosition = TransformToAncestor(mainWindow).Transform(new Point(0, 0));
        EngineInterop.SetViewport(_glfwHwnd, (int)windowPosition.X, (int)windowPosition.Y, (int)ActualWidth, (int)ActualHeight);
    }

    public Viewport()
    {
        InitializeComponent();
        Loaded += OnLoaded;
        SizeChanged += OnSizeChanged;
    }

    private void OnSizeChanged(object sender, SizeChangedEventArgs e)
    {
        if (!IsEngineInitialised())
            return;

        UpdateViewportExtents();
    }

    public static bool OpenProject(string path)
    {
        EngineInterop.OpenProject(path);
        return true;
    }

    private void OnUpdate(object? sender, EventArgs e)
    {
        if (_glfwHwnd == IntPtr.Zero)
            return;

        var keepRunning = EngineInterop.EngineUpdate(_glfwHwnd, 1f);
        if (!keepRunning)
        {
            EngineInterop.EngineShutdown(_glfwHwnd);
            Shutdown();
        }
    }

    private void OnLoaded(object sender, RoutedEventArgs e)
    {
        Loaded -= OnLoaded;
        InitializeViewport();
    }

    private void InitializeViewport()
    {
        if (_glfwHwnd != IntPtr.Zero)
            throw new InvalidOperationException("Tried to initialise viewport more than once!");

        UpdateLayout();
        if (ActualWidth == 0 || ActualHeight == 0)
            return;

        var hwndSource = (HwndSource?)PresentationSource.FromVisual(ViewportHost) ?? throw new InvalidOperationException();

        IntPtr parentHwnd = hwndSource.Handle;

        _glfwHwnd = EngineInterop.CreateWindow(parentHwnd, (int)ActualWidth, (int)ActualHeight);
        if (_glfwHwnd == IntPtr.Zero)
        {
            MessageBox.Show("Failed to create GLFW window.");
        }

        UpdateViewportExtents();
        EngineInterop.EngineInit(_glfwHwnd);
        CompositionTarget.Rendering += OnUpdate;
        EngineInitialised.Invoke();
    }

    public void Shutdown()
    {
        CompositionTarget.Rendering -= OnUpdate;
        if (_glfwHwnd != IntPtr.Zero)
            EngineInterop.EngineShutdown(_glfwHwnd);
    }
}