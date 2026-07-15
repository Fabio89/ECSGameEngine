export module Editor.EditingContext;
export import Editor.EditingContextId;
import Core;
import Engine.WorldManager;
import Editor.Selection;
import Editor.SnapshotFrame;
import EventBus;

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

    EditingContext(EditingContextId id, World& world, WorldHandle editorWorld, EventBus& events);
    EditingContext(const EditingContext&) = delete;
    EditingContext(EditingContext&&) = delete;
    EditingContext& operator=(const EditingContext&) = delete;
    EditingContext& operator=(EditingContext&&) = delete;
};

export class EditingContextManager
{
public:
    EditingContextManager(WorldManager& worlds, EventBus& editorEvents);

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
    WorldManager& m_worlds;
    EventBus& m_editorEvents;
    EditingContextId::ValueType m_lastId{0};
    std::unordered_map<EditingContextId::ValueType, std::unique_ptr<EditingContext>> m_contexts;
};
