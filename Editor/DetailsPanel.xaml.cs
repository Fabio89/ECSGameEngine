using System.Collections.ObjectModel;
using System.Reflection;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using Editor.GameProject;

namespace Editor;

public class ComponentViewModel
{
    public Component Component { get; set; }
    public string ComponentName { get; set; }

    public ObservableCollection<PropertyViewModel> Properties { get; set; }

    public ComponentViewModel(Component component)
    {
        Component = component;
        ComponentName = component.GetType().Name;

        Properties = new ObservableCollection<PropertyViewModel>
        (
            component.GetType().GetProperties(BindingFlags.Public | BindingFlags.Instance)
                .Where(p => p is { CanRead: true, CanWrite: true })
                .Select(p => new PropertyViewModel(component, p))
        );
    }
}

public class EntityViewModel : ViewModelBase
{
    private Entity? _selectedEntity;

    public Entity? SelectedEntity
    {
        get => _selectedEntity;
        set
        {
            SetField(ref _selectedEntity, value);
            Components = value != null ? new ObservableCollection<ComponentViewModel>(value.Components.Values.Select(c => new ComponentViewModel(c))) : [];
        }
    }

    public ObservableCollection<ComponentViewModel> Components { get; private set; } = [];
}

public class ObjectDetailsViewModel
{
    public ObservableCollection<PropertyViewModel> Properties { get; init; } = [];

    public ObjectDetailsViewModel()
    {
    }

    public ObjectDetailsViewModel(object obj)
    {
        LoadObject(obj);
    }
    
    public void LoadObject(object obj)
    {
        Properties.Clear();
        foreach (var prop in obj.GetType().GetProperties(BindingFlags.Public | BindingFlags.Instance))
        {
            if (prop is { CanRead: true, CanWrite: true })
            {
                Properties.Add(new PropertyViewModel(obj, prop));
            }
        }
    }
}

public partial class DetailsPanel : UserControl
{
    public EntityViewModel ViewModel { get; } = new();

    public DetailsPanel()
    {
        InitializeComponent();
        EntitySelector.Instance.SelectionChanged += OnEntitySelectionChanged;
        DataContext = ViewModel;
    }

    private void OnEntitySelectionChanged(Entity? obj)
    {
        ViewModel.SelectedEntity = obj;
        MyListBox.ItemsSource = ViewModel.Components;
    }
}