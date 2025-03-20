using System.Windows;
using Windows.Foundation.Metadata;
using Windows.UI;
using Windows.UI.ViewManagement;
using Microsoft.UI;
using Microsoft.Win32;

namespace Editor;

/// <summary>
/// Interaction logic for App.xaml
/// </summary>
public partial class App : Application
{
    public static Color GetAccentColor()
    {
        if (ApiInformation.IsApiContractPresent("Windows.Foundation.UniversalApiContract", 1))
        {
            var appSettings = new UISettings();
            return appSettings.GetColorValue(UIColorType.Accent);
        }
        return Colors.Gray;  // Fallback color if the API isn't available
    }
    
    public static bool IsWindowsInDarkMode()
    {
        var registryKey = Registry.GetValue(@"HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize", "AppsUseLightTheme", 1);
        return registryKey != null && (int)registryKey == 0;
    }
    
    protected override void OnStartup(StartupEventArgs e)
    {
        // if (IsWindowsInDarkMode())
        // {
        //     // Apply Dark Mode Resources
        //     this.Resources.MergedDictionaries.Clear();
        //     this.Resources.MergedDictionaries.Add(new ResourceDictionary
        //     {
        //         Source = new Uri("pack://application:,,,/Themes/Dark.xaml")
        //     });
        // }
        // else
        // {
        //     // Apply Light Mode Resources
        //     this.Resources.MergedDictionaries.Clear();
        //     this.Resources.MergedDictionaries.Add(new ResourceDictionary
        //     {
        //         Source = new Uri("pack://application:,,,/Themes/Light.xaml")
        //     });
        // }
        
        base.OnStartup(e);
    }
}