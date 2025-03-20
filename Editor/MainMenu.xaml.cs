using System.Windows;
using Editor.GameProject;

namespace Editor;

public partial class MainMenu
{
    public MainMenu()
    {
        InitializeComponent();
    }
    
    private void OnClickMenuItem_New(object sender, RoutedEventArgs e)
    {
        var projectBrowser = new NewProjectDialog();
        projectBrowser.ShowDialog();
    }
    
    private async void OnClickMenuItem_Open(object sender, RoutedEventArgs e)
    {
        try
        {
            var result = await FilePicker.PickFileAsync([$"{Project.Extension}"]);
            if (result != null)
            {
                ProjectOpener.Instance.OpenProject(result);
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