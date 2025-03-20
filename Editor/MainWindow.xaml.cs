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
        ProjectOpener.Instance.Opened += OnProjectOpened;
    }

    private void OnProjectOpened()
    {
        SceneManager.Instance.LoadCurrentScene();
    }

    private void OnEngineInitialised()
    {
        var settings = SettingsManager.LoadSettings();

        if (settings.RecentProjects.Any(ProjectOpener.Instance.OpenProject))
        {
        }
    }

    protected override void OnClosed(EventArgs e)
    {
        Viewport.Shutdown();
        base.OnClosed(e);
    }
}