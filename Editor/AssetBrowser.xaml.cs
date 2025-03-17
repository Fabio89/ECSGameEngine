using System.Collections.ObjectModel;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using Editor.GameProject;

namespace Editor;

public class FolderItem
{
    public string Name { get; set; }
    public string Path { get; set; }
    public ObservableCollection<FolderItem> SubFolders { get; set; }

    public FolderItem(string path)
    {
        Name = System.IO.Path.GetFileName(path);
        Path = path;
        SubFolders = new ObservableCollection<FolderItem>();

        // Load subfolders recursively
        LoadSubDirectories();
    }

    private void LoadSubDirectories()
    {
        try
        {
            foreach (var dir in Directory.GetDirectories(Path))
            {
                SubFolders.Add(new FolderItem(dir));
            }
        }
        catch
        {
            // Handle exceptions if needed (like access issues)
        }
    }
}

public partial class AssetBrowser : UserControl
{
    public AssetBrowser()
    {
        InitializeComponent();
        ProjectOpener.Opened += OnProjectOpened;
    }

    private void OnProjectOpened()
    {
        LoadFolders();
    }

    private void LoadFolders()
    {
        var rootFolderPath = Path.GetDirectoryName(ProjectOpener.CurrentProjectPath);
        if (!Directory.Exists(rootFolderPath)) return;
        var rootFolder = new FolderItem(rootFolderPath);
        FolderTreeView.Items.Add(rootFolder);
    }

    private void FolderTreeView_SelectedItem(object sender, RoutedPropertyChangedEventArgs<object> e)
    {
        if (e.NewValue is FolderItem selectedItem)
        {
            string path = selectedItem.Path;
            LoadAssets(path);
        }
    }

    private void LoadAssets(string path)
    {
        AssetListView.Items.Clear();
        foreach (var file in Directory.GetFiles(path))
        {
            AssetListView.Items.Add(new FileInfo(file));
        }
    }
}