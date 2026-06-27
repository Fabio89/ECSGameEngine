export module Editor.Panel.Inspector;
import Core;
import Component.Name;
import Editor;
import Editor.ImGui;
import Editor.Panel;
import Engine;
import Properties;
import World;

export namespace Panels
{
    class Inspector : public Panel
    {
    public:
        Inspector(World& world) : Panel{world} {}

    private:
        void doDraw(World& world) override
        {
            ImGui::Begin("Details", &m_open);

            const Entity inspectedEntity = Editor::selection().isEmpty() ? Entity{} : Editor::selection().get().front();
            if (world.isValid(inspectedEntity))
            {
                for (const TypeId typeId : world.getComponentTypesInEntity(inspectedEntity))
                {
                    if (typeId != Component<NameComponent>::typeId())
                    {
                        drawComponent(world, inspectedEntity, typeId);
                    }
                }
            }

            ImGui::End();
        }

        void drawComponent(World& world, Entity entity, TypeId componentTypeId)
        {
            ImGui::Separator();

            ImGui::ImGuiTreeNodeFlags flags =
                ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen |
                ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow |
                ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_SpanFullWidth;

            const ComponentTypeBase* componentType = ComponentRegistry::get(componentTypeId);
            if (!componentType->hasProperties())
            {
                flags |= ImGui::ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Leaf;
            }

            if (const ComponentTypeBase* type = ComponentRegistry::get(componentTypeId))
            {
                ImGui::PushID(entity.value);
                ImGui::PushID(componentTypeId.value);

                if (ImGui::TreeNodeEx(type->getName().data(), flags))
                {
                    for (const PropertyDescriptorBase& property : componentType->getProperties())
                    {
                        ComponentBase& component = world.editComponent(entity, componentTypeId);
                        property.draw(&component);
                    }

                    ImGui::TreePop();
                }

                ImGui::PopID();
                ImGui::PopID();
            }
        }

        bool m_open{true};
    };
}
