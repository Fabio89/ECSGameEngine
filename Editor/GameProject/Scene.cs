using System.Numerics;
using System.Reflection;
using System.Text.Json;
using System.Text.Json.Serialization;

namespace Editor.GameProject;

public class Component
{
}

public class NameComponent : Component
{
    [JsonPropertyName("name")]
    public string Name { get; set; } = string.Empty;
}

public struct Vector3(float x, float y, float z)
{
    public float X { get; set; } = x;
    public float Y { get; set; } = y;
    public float Z { get; set; } = z;
}

public class Vector3Converter : JsonConverter<Vector3>
{
    public override Vector3 Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
    {
        if (reader.TokenType != JsonTokenType.StartArray)
            throw new JsonException("Expected a JSON array for Vector3.");

        reader.Read();
        float x = reader.GetSingle();
        reader.Read();
        float y = reader.GetSingle();
        reader.Read();
        float z = reader.GetSingle();
        reader.Read(); // Move past EndArray

        return new Vector3(x, y, z);
    }

    public override void Write(Utf8JsonWriter writer, Vector3 value, JsonSerializerOptions options)
    {
        writer.WriteStartArray();
        writer.WriteNumberValue(value.X);
        writer.WriteNumberValue(value.Y);
        writer.WriteNumberValue(value.Z);
        writer.WriteEndArray();
    }
}

public class TransformComponent : Component
{
    [JsonPropertyName("position")]
    public Vector3 Position { get; set; }

    [JsonPropertyName("rotation")]
    public Vector3 Rotation { get; set; }
    
    [JsonPropertyName("scale")]
    public float Scale { get; set; }
}

public class ModelComponent : Component
{
    // Example component with no properties
}

public class Entity
{
    [JsonPropertyName("components")]
    public Dictionary<string, Component> Components { get; set; } = [];
}

public class Scene
{
    [JsonPropertyName("entities")]
    public Entity[] Entities { get; set; } = [];
}

// public class ComponentConverter : JsonConverter<Component>
// {
//     public override Component? Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions? options)
//     {
//         using JsonDocument doc = JsonDocument.ParseValue(ref reader);
//         var rootElement = doc.RootElement;
//
//         foreach (var property in rootElement.EnumerateObject())
//         {
//
//
//             return (Component?)JsonSerializer.Deserialize(property.Value.GetRawText(), type, options);
//         }
//
//         throw new JsonException("No valid component found");
//     }
//
//     public override void Write(Utf8JsonWriter writer, Component value, JsonSerializerOptions options)
//     {
//         JsonSerializer.Serialize(writer, value, value.GetType(), options);
//     }
// }

public class EntityConverter : JsonConverter<Entity>
{
    public override Entity Read(ref Utf8JsonReader reader, Type typeToConvert, JsonSerializerOptions options)
    {
        using var doc = JsonDocument.ParseValue(ref reader);
        var rootElement = doc.RootElement;

        var components = new Dictionary<string, Component>();

        foreach (var componentProperty in rootElement.GetProperty("components").EnumerateObject())
        {
            var componentTypeName = componentProperty.Name;
            var componentType = Type.GetType($"Editor.GameProject.{componentTypeName}, {Assembly.GetExecutingAssembly().FullName}");
            if (componentType == null)
                throw new JsonException("Tried to parse a component with an unknown type: " + componentTypeName);
            
            var componentJson = componentProperty.Value;

            var component = (Component?)JsonSerializer.Deserialize(componentJson.GetRawText(), componentType, options);
            if (component != null)
                components[componentTypeName] = component;
        }

        return new Entity { Components = components };

        Component? DeserializeComponent(string json)
        {
            return JsonSerializer.Deserialize<Component>(json, options);
        }
    }

    public override void Write(Utf8JsonWriter writer, Entity value, JsonSerializerOptions options)
    {
        JsonSerializer.Serialize(writer, value, typeof(Entity), options);
    }
}