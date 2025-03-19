using System.Collections.ObjectModel;
using System.Globalization;
using System.Reflection;
using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using Editor.GameProject;

namespace Editor;

public class BooleanToVisibilityConverter : IValueConverter
{
    public object Convert(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        if (value is bool booleanValue)
        {
            return booleanValue ? Visibility.Visible : Visibility.Collapsed;
        }
        return Visibility.Collapsed;
    }

    public object ConvertBack(object? value, Type targetType, object? parameter, CultureInfo culture)
    {
        throw new NotImplementedException();
    }
}

public partial class PropertyViewModel : ViewModelBase
{
    public string Name { get; }

    public object? Value
    {
        get => _value;
        set => SetField(ref _value, value);
    }
    
    public bool ShouldBeInlined => !IsComplexType(PropertyType) || Value == null;

    public Type PropertyType { get; }
    public ObservableCollection<PropertyViewModel> Children { get; } = [];

    private object? _value;
    private readonly PropertyInfo? _propertyInfo;
    private readonly object? _parentObject;

    public PropertyViewModel(object? parentObject, PropertyInfo? propertyInfo)
    {
        _parentObject = parentObject;
        _propertyInfo = propertyInfo;
        if (propertyInfo != null)
        {
            Name = PrettifyName(propertyInfo.Name);
            _value = propertyInfo.GetValue(parentObject);
            PropertyType = propertyInfo.PropertyType;
            if (IsComplexType(PropertyType))
            {
                // If it's a complex type, generate child properties
                PopulateChildren();
            }
        }
        else
        {
            PropertyType = typeof(object);
            Name = string.Empty;
        }
    }

    private bool IsComplexType(Type type) =>
        type != typeof(string) && (type.IsClass || (type.IsValueType && !type.IsPrimitive && type != typeof(decimal)));

    private void PopulateChildren()
    {
        if (Value == null) return;

        foreach (var prop in PropertyType.GetProperties(BindingFlags.Public | BindingFlags.Instance))
        {
            if (prop.CanRead && prop.CanWrite)
            {
                Children.Add(new PropertyViewModel(Value, prop));
            }
        }
    }

    private static string PrettifyName(string name) => MyRegex().Replace(name, " $1"); // Turns "MyProperty" into "My Property"
    
    [GeneratedRegex("(\\B[A-Z])")]
    private static partial Regex MyRegex();
}

public partial class PropertiesPanel : UserControl
{
    public static readonly DependencyProperty ViewModelProperty =
        DependencyProperty.Register(nameof(ViewModel), typeof(ObjectDetailsViewModel), typeof(PropertiesPanel), new PropertyMetadata(null));
    
    public ObjectDetailsViewModel ViewModel
    {
        get => (ObjectDetailsViewModel)GetValue(ViewModelProperty);
        set { SetValue(ViewModelProperty, value); DataContext = ViewModel; }
    }

    public PropertiesPanel()
    {
        InitializeComponent();
    }
}