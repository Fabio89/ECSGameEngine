export module Editor.Panels.Details;
import Components.Name;
import Editor;
import Editor.Controller;
import Editor.Panel.Impl;
import Editor.PropertyDrawers;
import Editor.Requests;
import Editor.SnapshotFrame;
import Engine;
import ImGui;

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

export namespace Requests
{
    struct SetProperty
    {
        Entity entity;
        TypeId componentType;
        const PropertyDescriptorBase* property{};
        PropertyValue value;
    };
}

export using DetailsRequest = std::variant<Requests::SetProperty>;

class DetailsController : public EditorControllerImpl<DetailsController, DetailsSnapshot, DetailsRequest>
{
public:
    using EditorControllerImpl::EditorControllerImpl;

    void execute(Requests::SetProperty&& request);

private:
    DetailsSnapshot buildSnapshot(const EditingContext& context) override;
};

export namespace Panels
{
    class DetailsPanel : public PanelImpl<DetailsController>
    {
    public:
        using PanelImpl::PanelImpl;
        static constexpr auto Name = "Details";

    private:
        void doDraw() override;

        bool m_open{true};
    };
}
