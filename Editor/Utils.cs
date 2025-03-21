using System.Text.Json;
using System.Text.RegularExpressions;
using Editor.GameProject;

namespace Editor;

public static partial class Utils
{
    public static string PrettifyName(string name) => MyRegex().Replace(name, " $1"); // Turns "MyProperty" into "My Property"

    [GeneratedRegex("(\\B[A-Z])")]
    private static partial Regex MyRegex();
    
    private static readonly JsonSerializerOptions SerializerOptions = new()
    {
        Converters = { new EntityConverter(), new Vector3Converter() },
        WriteIndented = true,
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase
    };
    
    public static string Serialize<TValue>(TValue value)
    {
        return JsonSerializer.Serialize(value, SerializerOptions);
    }
    
    public static string Serialize(object value, Type type)
    {
        return JsonSerializer.Serialize(value, type, SerializerOptions);
    }
    
    public static TValue? Deserialize<TValue>(string json)
    {
        return JsonSerializer.Deserialize<TValue>(json, SerializerOptions);
    }
}