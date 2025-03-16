using System.Windows;
using System.Windows.Controls;
using Editor.GameProject;

namespace Editor;

public partial class OpenProjectView : UserControl
{
    public OpenProjectView()
    {
        InitializeComponent();
        BrowsePathButton.Click += OnClick_BrowsePath;
    }
    
    private async void OnClick_BrowsePath(object sender, RoutedEventArgs e)
    {
        try
        {
            var result = await FilePicker.PickFileAsync([$"{Project.Extension}"]);
            if (result != null)
            {
                ProjectOpener.ProjectFilePath = result;
                ProjectOpener.OpenProject(result);
            }
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex);
        }
    }
}