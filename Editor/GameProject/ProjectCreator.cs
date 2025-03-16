using System.Collections.ObjectModel;
using System.IO;
using System.Runtime.Serialization;
using Editor.Serialisation;

namespace Editor.GameProject;

[DataContract]
public class ProjectTemplate
{
    [DataMember] public required string ProjectType { get; set; }
    [DataMember] public required string ProjectFile { get; set; }
    [DataMember] public List<string> Folders { get; set; } = [];

    public string IconPath { get; set; } = string.Empty;
    public string ScreenshotPath { get; set; } = string.Empty;
    public string ProjectFilePath { get; set; } = string.Empty;
    public byte[] Icon { get; set; } = [];
    public byte[] Screenshot { get; set; } = [];
}

public class ProjectCreator : ViewModelBase
{
    private const string TemplatePath = @"..\..\..\ProjectTemplates";
    private string _projectName = "NewProject";
    private string _projectPath = $@"{Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments)}\Mashi Projects\";
    private ValidationResult _pathValidity = ValidationResult.Invalid("Uninitialised");

    private readonly ObservableCollection<ProjectTemplate> _templates = CollectTemplates();

    public string ProjectName
    {
        get => _projectName;
        set { SetField(ref _projectName, value); UpdateProjectPathValidity(); }
    }

    public string ProjectPath
    {
        get => _projectPath;
        set
        {
            if (!Path.EndsInDirectorySeparator(value))
                value += @"\";
            SetField(ref _projectPath, value);
            UpdateProjectPathValidity();
        }
    }

    public ValidationResult PathValidity
    {
        get => _pathValidity;
        private set => SetField(ref _pathValidity, value);
    }

    public ProjectCreator()
    {
        UpdateProjectPathValidity();
    }

    public void CreateProject()
    {
        if (!UpdateProjectPathValidity() || _templates.Count == 0)
            return;
        
        var template = _templates.First();

        var path = $@"{ProjectPath}{ProjectName}\";

        try
        {
            if (!Directory.Exists(path))
                Directory.CreateDirectory(path);

            foreach (var folder in template.Folders)
            {
                Directory.CreateDirectory(Path.GetFullPath(Path.Combine(Path.GetDirectoryName(path) ?? throw new InvalidOperationException(), folder)));
            }

            File.Create($"{path}{template.ProjectFile}");
        }
        catch (Exception e)
        {
            Console.WriteLine(e);
            throw;
        }
    }

    private static ObservableCollection<ProjectTemplate> CollectTemplates()
    {
        ObservableCollection<ProjectTemplate> templates = [];
        
        try
        {
            var templateFiles = Directory.GetFiles(TemplatePath, "template.xml", SearchOption.AllDirectories);

            foreach (var templateFile in templateFiles)
            {
                var template = Serialiser.Read<ProjectTemplate>(templateFile);
                if (template == null)
                {
                    template = new ProjectTemplate
                    {
                        ProjectType = "Empty Project",
                        ProjectFile = "project.ma",
                        Folders = ["Content", "Code"]
                    };
                    Serialiser.Write(template, templateFile);
                }

                template.IconPath = ConvertToFullPath("icon.png");
                template.Icon = File.ReadAllBytes(template.IconPath);
                template.ScreenshotPath = ConvertToFullPath("screenshot.png");
                template.Screenshot = File.ReadAllBytes(template.ScreenshotPath);
                template.ProjectFilePath = ConvertToFullPath(template.ProjectFile);
                
                templates.Add(template);
                continue;

                string ConvertToFullPath(string fileName)
                {
                    return Path.GetFullPath(Path.Combine(Path.GetDirectoryName(templateFile) ?? throw new InvalidOperationException(), fileName));
                }
            }
        }
        catch (DirectoryNotFoundException ex)
        {
            Console.WriteLine(ex.Message);
            return templates;
        }

        return templates;
    }

    private bool UpdateProjectPathValidity()
    {
        PathValidity = Utils.ValidateProjectPath(ProjectPath, ProjectName);
        return PathValidity.IsValid;
    }
}