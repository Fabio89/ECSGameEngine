using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Reflection;
using Editor.GameProject;
using Component = Editor.GameProject.Component;

namespace Editor;

public class ComponentViewModel : ViewModelBase
{
    public Component Component { get; set; }
    public string ComponentName { get; set; }

    public ObservableCollection<PropertyViewModel> Properties { get; set; }

    public ComponentViewModel() : this(new Component()) {}
    
    public ComponentViewModel(Component component)
    {
        Component = component;
        ComponentName = Utils.PrettifyName(component.GetType().Name);

        Properties = new ObservableCollection<PropertyViewModel>
        (
            component.GetType().GetProperties(BindingFlags.Public | BindingFlags.Instance)
                .Where(p => p is { CanRead: true, CanWrite: true })
                .Select(p => new PropertyViewModel(component, p))
        );

        foreach (var property in Properties)
        {
            property.PropertyChanged += OnPropertyChanged;
        }
    }

    private void OnPropertyChanged(object? sender, PropertyChangedEventArgs e)
    {
        var property = Component.GetType().GetProperty(e.PropertyName!);
        Console.WriteLine($"[ComponentViewModel] Component property changed: {Component.GetType().Name}.{e.PropertyName} = {property?.GetValue(Component)}");
        TriggerPropertyChanged(e.PropertyName);
    }
}

public class EntityViewModel : ViewModelBase
{
    private readonly Entity? _entity;

    public Entity? Entity
    {
        get => _entity;
        private init
        {
            SetField(ref _entity, value);
            Components = value != null ? new ObservableCollection<ComponentViewModel>(value.Components.Values.Select(c => new ComponentViewModel(c))) : [];

            foreach (var component in Components)
            {
                component.PropertyChanged += OnComponentPropertyChanged;
            }
        }
    }

    public ObservableCollection<ComponentViewModel> Components { get; private init; } = [];

    public EntityViewModel(Entity entity)
    {
        Entity = entity;
    }
    
    private static void OnComponentPropertyChanged(object? sender, PropertyChangedEventArgs e)
    {
        var component = ((ComponentViewModel)sender!).Component;
        var property = component.GetType().GetProperty(e.PropertyName!);
        Console.WriteLine($"[EntityViewModel] Component property changed: {component.GetType().Name}.{e.PropertyName} = {property?.GetValue(component)}");
    }
}

public partial class DetailsPanel
{
    private ObservableCollection<EntityViewModel> Entities { get; set; } = [];

    private EntityViewModel? SelectedEntity { get; set; }

    public DetailsPanel()
    {
        InitializeComponent();

        SceneManager.Instance.Loaded += OnSceneLoaded;
        EntitySelector.Instance.SelectionChanged += OnEntitySelectionChanged;
    }

    private void OnSceneLoaded(Scene scene)
    {
        Entities = new ObservableCollection<EntityViewModel>(scene.Entities.Select(x => new EntityViewModel(x)));
    }

    private void OnEntitySelectionChanged(Entity? obj)
    {
        SelectedEntity = Entities.First(x => x.Entity == obj);
        MyListBox.ItemsSource = SelectedEntity.Components;
    }
}