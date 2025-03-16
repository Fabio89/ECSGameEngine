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
        //OpenProjectBrowserDialog();
    }
    
    protected override void OnClosed(EventArgs e)
    {
        Viewport.Shutdown();
        base.OnClosed(e);
    }

    private void OpenProjectBrowserDialog()
    {
        var projectBrowser = new ProjectBrowserDialog();
        if (projectBrowser.ShowDialog() == false)
        {
            Application.Current.Shutdown();
        }
    }

    private void OnClickMenuItem_New(object sender, RoutedEventArgs e)
    {
        throw new NotImplementedException();
    }
    
    private async void OnClickMenuItem_Open(object sender, RoutedEventArgs e)
    {
        try
        {
            var result = await FilePicker.PickFileAsync([$"{Project.Extension}"]);
            if (result != null)
            {
                ProjectOpener.OpenProject(result);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex);
        }
    }

    private void OnClickMenuItem_Save(object sender, RoutedEventArgs e)
    {
        throw new NotImplementedException();
    }

    private void OnClickMenuItem_Exit(object sender, RoutedEventArgs e)
    {
        throw new NotImplementedException();
    }

    private void OnClickMenuItem_Undo(object sender, RoutedEventArgs e)
    {
        throw new NotImplementedException();
    }

    private void OnClickMenuItem_Redo(object sender, RoutedEventArgs e)
    {
        throw new NotImplementedException();
    }

    private void OnClickMenuItem_Cut(object sender, RoutedEventArgs e)
    {
        throw new NotImplementedException();
    }

    private void OnClickMenuItem_Copied(object sender, RoutedEventArgs e)
    {
        throw new NotImplementedException();
    }

    private void OnClickMenuItem_About(object sender, RoutedEventArgs e)
    {
        throw new NotImplementedException();
    }
}