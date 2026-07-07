export module Editor.Panels.Hierarchy;
import Component.Hierarchy;
import Component.Name;
import Component.Model;
import Component.Transform;
import ComponentRegistry;
import Editor;
import Editor.Controller;
import ImGui;
import Editor.Panel.Impl;
import Editor.Requests;
import Engine;
import Math;
import Properties;
import Editor.Selection;

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

class HierarchyController : public EditorControllerImpl<HierarchySnapshot>
{
public:
    using EditorControllerImpl::EditorControllerImpl;

private:
    HierarchySnapshot buildSnapshot(const EditingContext& context) override;
};

void traverseNode(HierarchyNode& node, Entity entity, const World& world, const Editor::Selection& selection)
{
    node.entity = entity;
    node.name = world.readComponent<NameComponent>(entity).name;
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

HierarchySnapshot HierarchyController::buildSnapshot(const EditingContext& context)
{
    World& world = Engine::getWorld(context.world);

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
    class HierarchyPanel : public PanelImpl
    {
    public:
        explicit HierarchyPanel(const PanelCreateInfo& info) : PanelImpl{info}
        {
            Editor::addController<HierarchyController>(info.contextId);
        }

        static constexpr auto Name = "Hierarchy";

    private:
        void doDraw() override
        {
            if (!context().snapshotPublisher.frame().contains<HierarchySnapshot>())
                return;

            const HierarchySnapshot& snapshot = context().snapshotPublisher.frame().get<HierarchySnapshot>();

            ImGui::Begin(Name, &m_open);

            for (const HierarchyNode& node : snapshot.nodes)
            {
                drawEntity(node);
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

            if (ImGui::IsItemClicked() || ImGui::IsItemFocused())
            {
                Editor::request(Editor::ChangeSelection{.contextId = context().id, .entities = {node.entity}});
            }

            if (ImGui::TreeNodeEx(node.name.c_str(), flags))
            {
                for(const HierarchyNode& child : node.children)
                    drawEntity(child);

                ImGui::TreePop();
            }

            ImGui::PopID();
        }

        bool m_open{true};
    };
}

bool Panels::isEditorOnly(const World& world, Entity entity)
{
    return world.hasComponent<TagsComponent>(entity) && std::ranges::contains(world.readComponent<TagsComponent>(entity).tags, Tag::editorOnly);
}