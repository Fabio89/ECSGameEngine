namespace Editor;

using System;
using System.IO;
using System.Text.Json;

public static class SettingsManager
{
    public class Settings
    {
        public string[] RecentProjects { get; init; } = [];
    }
    
    private const string SettingsFilePath = "settings.json"; // Path to your settings file

    public static Settings LoadSettings()
    {
        try
        {
            var json = File.ReadAllText(SettingsFilePath);
            var settings = JsonSerializer.Deserialize<Settings>(json) ?? throw new InvalidOperationException();
            return settings;
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error reading settings file: {ex.Message}");
            return new Settings();
        }
    }

    public static void SaveSettings(Settings settings)
    {
        try
        {
            var json = JsonSerializer.Serialize(settings, new JsonSerializerOptions { WriteIndented = true });
            File.WriteAllText(SettingsFilePath, json);
        }
        catch (Exception ex)
        {
            Console.WriteLine($"Error saving settings file: {ex.Message}");
        }
    }
}