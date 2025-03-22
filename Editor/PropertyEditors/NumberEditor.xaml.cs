using System.Text.RegularExpressions;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace Editor.PropertyEditors;

public partial class NumberEditor : BaseEditor
{
    public NumberEditor()
    {
        InitializeComponent();
    }

    protected override void TextBox_PreviewTextInput(object sender, TextCompositionEventArgs e)
    {
        if (char.IsDigit(e.Text, 0) || e.Text == "." || e.Text == "-")
        {
            // Check if it's a negative sign
            var textBox = sender as TextBox;
            if (e.Text == "-" && (textBox?.SelectionStart == 0 || textBox.Text.IndexOf("-") == -1))
            {
                // Allow '-' only if it's at the start and not already present
                e.Handled = false;
            }
            else if (e.Text != "-")
            {
                // Allow other numeric characters and the decimal point
                e.Handled = false;
            }
            else
            {
                e.Handled = true; // Block further input of "-" in invalid places
            }
        }
        else
        {
            // Block non-numeric input
            e.Handled = true;
        }
    }

    [GeneratedRegex(@"[0-9.]")]
    private static partial Regex NumericalRegex();
}