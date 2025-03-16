namespace Editor.GameProject;

public class Scene(string name) : ViewModelBase
{
    private string _name = name;

    public string Name
    {
        get => _name;
        set => SetField(ref _name, value);
    }
}