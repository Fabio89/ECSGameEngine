namespace Editor.GameProject;

public class EntitySelector
{
    private static EntitySelector? _instance;
    public static EntitySelector Instance => _instance ??= new EntitySelector();

    private Entity? _selectedEntity;
    public event Action<Entity?>? SelectionChanged;

    public Entity? SelectedEntity
    {
        get => _selectedEntity;
        set
        {
            if (_selectedEntity != value)
            {
                _selectedEntity = value;
                SelectionChanged?.Invoke(_selectedEntity);
            }
        }
    }

    private EntitySelector() { }
}