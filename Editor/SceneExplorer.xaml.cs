using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;
using System.Windows;
using System.Windows.Controls;
using Editor.GameProject;

namespace Editor;

public class SceneItem
{
    public Entity Entity { get; }
    public string Name { get; }

    public SceneItem(Entity entity)
    {
        Entity = entity;
        Name = entity.Components.TryGetValue(nameof(NameComponent), out var nameComponent) ? ((NameComponent)nameComponent).Name : string.Empty;
    }
}

public partial class SceneExplorer
{
    public SceneExplorer()
    {
        InitializeComponent();
        ProjectOpener.Opened += OnProjectOpened;
    }

    private void OnProjectOpened()
    {
        var scene = EngineInterop.LoadScene();
        
        foreach (var entity in scene.Entities)
        {
            SceneTreeView.Items.Add(new SceneItem(entity));
        }
    }

    private void OnSelectedTreeViewItem(object sender, RoutedPropertyChangedEventArgs<object> e)
    {
        EntitySelector.Instance.SelectedEntity = (e.NewValue as SceneItem)?.Entity;
    }
}