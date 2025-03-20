using System.Globalization;
using System.Windows.Data;
using System.Windows;
using Editor.GameProject;

namespace Editor;

public class InvertedBooleanToVisibilityConverter : IValueConverter
{
    public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        if (value is bool booleanValue)
        {
            return booleanValue ? Visibility.Hidden : Visibility.Visible;
        }
        return Visibility.Visible;
    }

    public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}

public partial class NewProjectDialog
{
    public NewProjectDialog()
    {
        InitializeComponent();
        BrowsePathButton.Click += OnClick_BrowsePath;
        CreateButton.Click += OnClick_CreateButton;
    }

    private async void OnClick_BrowsePath(object sender, RoutedEventArgs e)
    {
        try
        {
            var result = await FilePicker.PickFolderAsync();
            if (result != null)
                ProjectCreator.ProjectPath = result;
        }
        catch (Exception ex)
        {
            Console.WriteLine(ex);
        }
    }
    
    private void OnClick_CreateButton(object sender, RoutedEventArgs e)
    {
        var project = ProjectCreator.CreateProject();
        if (!string.IsNullOrEmpty(project))
            ProjectOpener.Instance.OpenProject(project);
    }
}