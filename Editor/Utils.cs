using System.Text.RegularExpressions;

namespace Editor;

public partial class Utils
{
    public static string PrettifyName(string name) => MyRegex().Replace(name, " $1"); // Turns "MyProperty" into "My Property"

    [GeneratedRegex("(\\B[A-Z])")]
    private static partial Regex MyRegex();
}