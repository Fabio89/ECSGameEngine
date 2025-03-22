using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;

namespace Editor.PropertyEditors;

public class ClickSelectTextBox : TextBox
{
    public ClickSelectTextBox()
    {
        AddHandler(PreviewMouseLeftButtonDownEvent, new MouseButtonEventHandler(SelectivelyIgnoreMouseButton), true);
        AddHandler(GotKeyboardFocusEvent, new RoutedEventHandler(SelectAllText), true);
        AddHandler(MouseDoubleClickEvent, new RoutedEventHandler(SelectAllText), true);
    }

    private static void SelectivelyIgnoreMouseButton(object sender, MouseButtonEventArgs e)
    {
        // Find the TextBox
        DependencyObject parent = e.OriginalSource as UIElement;
        while (parent != null && parent is not TextBox)
            parent = VisualTreeHelper.GetParent(parent);

        if (parent != null)
        {
            var textBox = (TextBox)parent;
            if (!textBox.IsKeyboardFocusWithin)
            {
                // If the text box is not yet focussed, give it the focus and
                // stop further processing of this click event.
                textBox.Focus();
                e.Handled = true;
            }
        }
    }

    private static void SelectAllText(object sender, RoutedEventArgs e)
    {
        var textBox = e.OriginalSource as TextBox;
        if (textBox != null)
            textBox.SelectAll();
    }
}

public partial class BaseEditor : UserControl
{
    public BaseEditor()
    {
        InitializeComponent();
    }

    private void TextBox_GotFocus(object sender, RoutedEventArgs e)
    {
        var textBox = sender as TextBox;
        textBox?.SelectAll();
        e.Handled = true;
    }

    private void TextBox_PreviewMouseDown(object sender, MouseButtonEventArgs e)
    {
        // var textBox = sender as TextBox;
        // textBox?.Focus(); 
    }

    private void TextBox_PreviewMouseUp(object sender, MouseButtonEventArgs e)
    {
        // var textBox = sender as TextBox;
        // textBox?.SelectAll(); 
    }

    private void TextBox_KeyDown(object sender, KeyEventArgs e)
    {
        if (e.Key == Key.Enter)
        {
            e.Handled = true;
            var textBox = sender as TextBox;
            (DataContext as PropertyViewModel)!.Value = textBox?.Text;
        }
    }

    protected virtual void TextBox_PreviewTextInput(object sender, TextCompositionEventArgs e)
    {
    }
}