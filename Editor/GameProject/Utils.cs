using System.IO;

namespace Editor.GameProject;

public class ValidationResult
{
    public bool IsValid { get; private init; } = true;
    public string ErrorMessage { get; private init; } = string.Empty;

    // Static methods to create success and error results
    public static ValidationResult Valid => new() { IsValid = true };
    public static ValidationResult Invalid(string errorMessage) => new() { IsValid = false, ErrorMessage = errorMessage };
}

public static class Utils
{
    public static ValidationResult ValidateProjectPath(string projectPath, string projectName)
    {
        var errorMsg = string.Empty;
        if (string.IsNullOrWhiteSpace(projectName.Trim()))
        {
            errorMsg = "Project name cannot be empty.";
        }
        else if (projectName.IndexOfAny(Path.GetInvalidFileNameChars()) != -1)
        {
            errorMsg = "Project name contains invalid characters.";
        }
        else if (string.IsNullOrWhiteSpace(projectPath.Trim()))
        {
            errorMsg = "Select a valid project folder.";
        }
        else if (projectPath.IndexOfAny(Path.GetInvalidPathChars()) != -1)
        {
            errorMsg = "Path contains invalid characters.";
        }
        else
        {
            var fullPath = projectPath + projectName + @"\";
            if (Directory.Exists(fullPath) && Directory.EnumerateFileSystemEntries(fullPath).Any())
                errorMsg = "Directory is not empty.";
        }

        return string.IsNullOrEmpty(errorMsg) ? ValidationResult.Valid : ValidationResult.Invalid(errorMsg);
    }
}