using System.Collections.ObjectModel;
using System.Runtime.Serialization;

namespace Editor.GameProject;

[DataContract]
public class Project(string name) : ViewModelBase
{
    public const string Extension = ".ma";

    [DataMember] public string Name { get; } = name;

    // public string Path { get; } = path;
    // public string FullPath => $"{Path}{Name}{Extension}";
    [DataMember] private ObservableCollection<Scene> Scenes { get; set; } = [];
}