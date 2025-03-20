using System.Windows;
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
        SceneManager.Instance.Loaded += OnSceneLoaded;
    }

    private void OnSceneLoaded(Scene scene)
    {
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