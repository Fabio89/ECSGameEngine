using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using Editor.GameProject;

namespace Editor;

/// <summary>
/// Interaction logic for MainWindow.xaml
/// </summary>
public partial class MainWindow
{
    private const int AssetBrowserHeight = 400;
    public MainWindow()
    {
        InitializeComponent();
        Viewport.EngineInitialised += OnEngineInitialised;
    }

    private void OnEngineInitialised()
    {
        var settings = SettingsManager.LoadSettings();

        if (settings.RecentProjects.Any(ProjectOpener.OpenProject))
        {
        }
    }

    protected override void OnClosed(EventArgs e)
    {
        Viewport.Shutdown();
        base.OnClosed(e);
    }
}