using System.Windows;
using System.Windows.Controls;
using Windows.Storage.Pickers;
using WinRT.Interop;

namespace Editor;

public static class FilePicker
{
    public static async Task<string?> PickFolderAsync()
    {
        var picker = new FolderPicker
        {
            SuggestedStartLocation = PickerLocationId.DocumentsLibrary
        };
        picker.FileTypeFilter.Add("*"); // Required
        IntPtr hwnd = System.Diagnostics.Process.GetCurrentProcess().MainWindowHandle;
        InitializeWithWindow.Initialize(picker, hwnd);

        var folder = await picker.PickSingleFolderAsync();
        return folder?.Path;
    }
    
    public static async Task<string?> PickFileAsync(string[] fileTypes)
    {
        var picker = new FileOpenPicker
        {
            SuggestedStartLocation = PickerLocationId.DocumentsLibrary
        };
        
        foreach (var fileType in fileTypes)
        {
            picker.FileTypeFilter.Add(fileType);
        }
        
        IntPtr hwnd = System.Diagnostics.Process.GetCurrentProcess().MainWindowHandle;
        InitializeWithWindow.Initialize(picker, hwnd);

        var file = await picker.PickSingleFileAsync();
        return file?.Path;
    }
}

public partial class ProjectBrowserDialog
{
    public ProjectBrowserDialog()
    {
        InitializeComponent();
        OpenProjectButton.IsChecked = true;
        CreateProjectButton.IsChecked = false;
    }

    private void OnToggleButton_Click(object sender, RoutedEventArgs e)
    {
        if (ReferenceEquals(sender, OpenProjectButton))
        {
            if (CreateProjectButton.IsChecked is true)
            {
                CreateProjectButton.IsChecked = false;
                BrowserContent.Margin = new Thickness(0);
            }
            OpenProjectButton.IsChecked = true;
        }
        else
        {
            if (OpenProjectButton.IsChecked is true)
            {
                OpenProjectButton.IsChecked = false;
                BrowserContent.Margin = new Thickness(-720, 0, 0, 0);
            }
            CreateProjectButton.IsChecked = true;
        }
    }
}