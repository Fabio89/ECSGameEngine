export module Editor.EditingContext;
export import Editor.EditingContextId;
import Editor.Selection;
import World;
import std;

export struct EditingContext
{
    EditingContextId id;
    std::reference_wrapper<World> world;
    Editor::Selection selection;
    Entity camera;
};

export class EditingContextManager
{
public:
    EditingContextId add(World& world);

    EditingContext& get(EditingContextId id);

    const EditingContext& get(EditingContextId id) const;

    void remove(EditingContextId contextId);

    [[nodiscard]]
    std::generator<EditingContext&> getAll()
    {
        for (std::unique_ptr<EditingContext>& context : m_contexts | std::views::values)
            co_yield *context;
    }

private:
    EditingContextId::ValueType m_lastId{0};
    std::unordered_map<EditingContextId::ValueType, std::unique_ptr<EditingContext>> m_contexts;
};
