export module Editor.Panels.Details;
import Component.Name;
import Editor;
import Editor.ImGui;
import Editor.Panel.Impl;
import Engine;
import Properties;

export namespace Panels
{
    class DetailsPanel : public PanelImpl
    {
    public:
        using PanelImpl::PanelImpl;

    private:
        void doDraw() override
        {
            World& world = context().world;
            ImGui::Begin("Details", &m_open);

            const Entity inspectedEntity = context().selection.isEmpty() ? Entity{} : context().selection.get().front();
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

            ImGuiTreeNodeFlags flags =
                ImGuiTreeNodeFlags_DefaultOpen |
                ImGuiTreeNodeFlags_OpenOnArrow |
                ImGuiTreeNodeFlags_SpanFullWidth;

            const ComponentTypeBase* componentType = ComponentRegistry::get(componentTypeId);
            if (!componentType->hasProperties())
            {
                flags |= ImGuiTreeNodeFlags_Leaf;
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
