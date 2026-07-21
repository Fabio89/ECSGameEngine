export module Editor.Panels.Hierarchy;
import Components.Hierarchy;
import Components.Name;
import Components.Model;
import Components.Transform;
import ComponentRegistry;
import Editor;
import Editor.Controller;
import Editor.Selection;
import ImGui;
import Editor.Panel.Impl;
import Editor.Requests;
import Engine;
import Math;
import Properties;
import World;

namespace Panels
{
    bool isEditorOnly(const World& world, Entity entity);
}

struct HierarchyNode
{
    Entity entity;
    std::string name;
    bool selected{};
    std::vector<HierarchyNode> children;
};

struct HierarchySnapshot
{
    std::vector<HierarchyNode> nodes;
};

namespace Requests
{
    struct SelectEntity
    {
        Entity entity;
    };

    struct ReparentEntity
    {
        Entity entity;
        Entity newParent;
    };
}

using HierarchyRequest = std::variant<
    Requests::SelectEntity,
    Requests::ReparentEntity
>;

class HierarchyController : public EditorControllerImpl<HierarchyController, HierarchySnapshot, HierarchyRequest>
{
public:
    using EditorControllerImpl::EditorControllerImpl;

    void execute(Requests::SelectEntity&& request);
    void execute(Requests::ReparentEntity&& request);

private:
    HierarchySnapshot buildSnapshot(const EditingContext& context) override;
};

void traverseNode(HierarchyNode& node, Entity entity, const World& world, const Editor::Selection& selection)
{
    node.entity = entity;
    node.name = NameUtils::getName(world, entity);
    node.selected = selection.contains(entity);

    for (Entity child : HierarchyUtils::children(world, entity))
    {
        if (!Panels::isEditorOnly(world, child))
        {
            HierarchyNode& childNode = node.children.emplace_back();
            traverseNode(childNode, child, world, selection);
        }
    }
}

void HierarchyController::execute(Requests::SelectEntity&& request)
{
    if (request.entity.isValid())
        context().selection.setSingle(request.entity);
    else
        context().selection.clear();
}

void HierarchyController::execute(Requests::ReparentEntity&& request)
{
    HierarchyUtils::setParent(services().worlds.get(context().world), request.entity, request.newParent);
}

HierarchySnapshot HierarchyController::buildSnapshot(const EditingContext& context)
{
    World& world = services().worlds.get(context.world);

    HierarchySnapshot snapshot;
    auto entitiesRange = world.getEntitiesRange();
    snapshot.nodes.reserve(entitiesRange.size());

    for (Entity entity : entitiesRange)
    {
        if (!HierarchyUtils::getParent(world, entity).isValid() && !Panels::isEditorOnly(world, entity))
        {
            HierarchyNode& node = snapshot.nodes.emplace_back(entity);
            traverseNode(node, entity, world, context.selection);
        }
    }

    return snapshot;
}

export namespace Panels
{
    class HierarchyPanel : public PanelImpl<HierarchyController>
    {
    public:
        using PanelImpl::PanelImpl;
        static constexpr auto Name = "Hierarchy";

    private:
        void doDraw() override
        {
            SnapshotView snapshot = getSnapshot();
            if (!snapshot.get())
                return;

            ImGui::Begin(Name, &m_open);

            for (const HierarchyNode& node : snapshot->nodes)
            {
                drawEntity(node);
            }

            if (ImGui::BeginDragDropTarget())
            {
                ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImColor{255, 255, 0, 255});

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
                {
                    const Entity dragged = *static_cast<const Entity*>(payload->Data);
                    request(Requests::ReparentEntity{.entity = dragged, .newParent = Entity{}});
                }

                ImGui::EndDragDropTarget();
            }

            ImGui::End();
        }

        void drawEntity(const HierarchyNode& node)
        {
            ImGui::PushID(node.entity.value);

            ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_DefaultOpen |
                ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_SpanFullWidth;

            if (node.selected)
                flags |= ImGuiTreeNodeFlags_Selected;

            if (node.children.empty())
                flags |= ImGuiTreeNodeFlags_Leaf;

            const bool expanded = ImGui::TreeNodeEx(node.name.c_str(), flags);

            if (ImGui::IsItemClicked())
            {
                request(Requests::SelectEntity{node.entity});
            }

            if (ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("Entity", &node.entity, sizeof(Entity));
                ImGui::Text("%s", node.name.c_str());
                ImGui::EndDragDropSource();
            }

            if (expanded)
            {
                for(const HierarchyNode& child : node.children)
                    drawEntity(child);

                ImGui::TreePop();
            }

            ImGui::PopID();

            if (ImGui::BeginDragDropTarget())
            {
                ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImColor{255, 255, 0, 255});

                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Entity"))
                {
                    const Entity dragged = *static_cast<const Entity*>(payload->Data);
                    request(Requests::ReparentEntity{.entity = dragged, .newParent = node.entity});
                }

                ImGui::EndDragDropTarget();
            }
        }

        bool m_open{true};
    };
}

bool Panels::isEditorOnly(const World& world, Entity entity)
{
    return world.hasComponent<TagsComponent>(entity) && std::ranges::contains(world.readComponent<TagsComponent>(entity).tags, Tag::editorOnly);
}