export module Editor.EditingContext;
export import Editor.EditingContextId;
import Core;
import Editor.Services;
import Engine.WorldManager;
import Editor.Selection;
import Editor.SnapshotFrame;
import WorldHandle;

export struct EditingContextCreateInfo
{
    WorldHandle world;
    WorldHandle editorWorld;
};

export struct EditingContext
{
    EditingContextId id;
    WorldHandle world;
    WorldHandle editorWorld;
    Editor::Selection selection;
    Editor::SnapshotPublisher snapshotPublisher;
};

export class EditingContextManager
{
public:
    EditingContextManager(EditorServices& services);

    EditingContextId add(EditingContextCreateInfo info);

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
    EditorServices& m_services;
    EditingContextId::ValueType m_lastId{0};
    std::unordered_map<EditingContextId::ValueType, std::unique_ptr<EditingContext>> m_contexts;
};
