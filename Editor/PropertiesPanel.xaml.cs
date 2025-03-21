using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Globalization;
using System.Reflection;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;

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

public class PropertyEditorTemplateSelector : DataTemplateSelector
{
    public DataTemplate? TextTemplate { get; set; }
    public DataTemplate? NumberTemplate { get; set; }
    public DataTemplate? BooleanTemplate { get; set; }

    public override DataTemplate? SelectTemplate(object? item, DependencyObject container)
    {
        if (item is not PropertyViewModel property)
            return base.SelectTemplate(item, container);
        var type = property.PropertyType;
        if (type == typeof(string))
            return TextTemplate;
        if (type == typeof(int) || type == typeof(float) || type == typeof(double))
            return NumberTemplate;
        if (type == typeof(bool))
            return BooleanTemplate;
        
        return TextTemplate;
    }
}

public class PropertyViewModel : ViewModelBase
{
    public string Name { get; }

    public object? Value
    {
        get => _propertyInfo.GetValue(_parentObject);
        set
        {
            try
            {
                var castedValue = Convert.ChangeType(value, _propertyInfo.PropertyType);
                _propertyInfo.SetValue(_parentObject, castedValue);

                var type = value?.GetType();
                _isComplexType = type == null || type != typeof(string) && (type.IsClass || (type is { IsValueType: true, IsPrimitive: false } && type != typeof(decimal)));

                _children = [];
                if (_isComplexType && value != null)
                {
                    var properties = PropertyType.GetProperties(BindingFlags.Public | BindingFlags.Instance)
                        .Where(p => p is { CanRead: true, CanWrite: true } && p.Name != nameof(Value));

                    foreach (var prop in properties)
                    {
                        var childProperty = new PropertyViewModel(value, prop);
                        childProperty.PropertyChanged += OnChildPropertyChanged;
                        _children.Add(childProperty);
                    }
                }

                TriggerPropertyChanged(_propertyInfo.Name);
            }
            catch (FormatException)
            {
            }
        }
    }
    public bool ShouldBeInlined => !IsComplexType(PropertyType) || Value == null;

    public Type PropertyType { get; }
    public List<PropertyViewModel> Children => _children;

    private bool _isComplexType;
    private readonly PropertyInfo _propertyInfo;
    private readonly object _parentObject;
    private List<PropertyViewModel> _children = [];

    public PropertyViewModel(object parentObject, PropertyInfo propertyInfo)
    {
        _parentObject = parentObject;
        _propertyInfo = propertyInfo;
        Name = Utils.PrettifyName(propertyInfo.Name);
        PropertyType = propertyInfo.PropertyType;
        Value = _propertyInfo.GetValue(_parentObject);
    }

    private bool IsComplexType(Type type) =>
        type != typeof(string) && (type.IsClass || (type is { IsValueType: true, IsPrimitive: false } && type != typeof(decimal)));
    
    private void OnChildPropertyChanged(object? sender, PropertyChangedEventArgs e)
    {
        TriggerPropertyChanged(_propertyInfo.Name);
    }
}

public partial class PropertiesPanel : UserControl
{
    public PropertiesPanel()
    {
        InitializeComponent();
    }
}