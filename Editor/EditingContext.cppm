export module Editor.EditingContext;
export import Editor.EditingContextId;
import Core;
import Editor.Selection;
import Editor.SnapshotFrame;
import WorldHandle;

export struct EditingContext
{
    EditingContextId id;
    WorldHandle world;
    Editor::Selection selection;
    Editor::SnapshotPublisher snapshotPublisher;
    Entity camera;
};

export class EditingContextManager
{
public:
    EditingContextId add(WorldHandle world);

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
