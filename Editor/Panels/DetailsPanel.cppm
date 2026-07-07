export module Editor.Panels.Details;
import Component.Name;
import Editor;
import Editor.Controller;
import Editor.Panel.Impl;
import Editor.PropertyDrawers;
import Editor.Requests;
import Editor.SnapshotFrame;
import Engine;
import ImGui;

export namespace Panels
{
    struct PropertySnapshot
    {
        const PropertyDescriptorBase* descriptor{};
        PropertyValue value;
    };

    struct ComponentSnapshot
    {
        const ComponentTypeBase* type{};
        std::vector<PropertySnapshot> properties;
    };

    struct DetailsSnapshot
    {
        Entity entity;
        std::vector<ComponentSnapshot> components;
    };

    class DetailsController : public EditorControllerImpl<DetailsSnapshot>
    {
    public:
        using EditorControllerImpl::EditorControllerImpl;
    private:
        DetailsSnapshot buildSnapshot(const EditingContext& context) override;
    };

    class DetailsPanel : public PanelImpl
    {
    public:
        DetailsPanel(const PanelCreateInfo& info);
        static constexpr auto Name = "Details";

    private:
        void doDraw() override;

        bool m_open{true};
    };
}
