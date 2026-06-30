export module Editor.PropertyDrawers;
export import Properties;
import Editor.ImGui;
import Math;
import ComponentRegistry;

namespace
{
    class PropertyDrawerRegistry
    {
    public:
        using DrawerFunc = std::function<bool(std::string_view, PropertyValue&)>;

        template<typename T>
        void registerDrawer(DrawerFunc drawer)
        {
            m_drawers[getTypeId<T>()] = std::move(drawer);
        }

        bool draw(const PropertyDescriptorBase& property, PropertyValue& value)
        {
            if (auto it = m_drawers.find(property.getTypeId()); it != m_drawers.end())
                return it->second && (it->second)(property.getName(), value);
            return false;
        }
    private:
        std::unordered_map<TypeId, DrawerFunc> m_drawers;
    } propertyDrawers;
}

export namespace Editor
{
    bool drawProperty(const PropertyDescriptorBase& property, PropertyValue& value)
    {
        return propertyDrawers.draw(property, value);
    }

    template<typename T>
    void registerPropertyDrawer(std::function<bool(std::string_view, PropertyValue&)> drawer)
    {
        propertyDrawers.registerDrawer<T>(std::move(drawer));
    }

    void initPropertyDrawers()
    {
        registerPropertyDrawer<float>([](std::string_view name, PropertyValue& value)
        {
            return ImGui::DragFloat(name.data(), &std::any_cast<float&>(value));
        });

        registerPropertyDrawer<Vec3>([](std::string_view name, PropertyValue& value)
        {
            return ImGui::DragFloat3(name.data(), &std::any_cast<Vec3&>(value).x);
        });

        registerPropertyDrawer<Mat4>([](std::string_view name, PropertyValue& value)
        {
            auto& matrix = std::any_cast<Mat4&>(value);

            if (ImGui::TreeNode(name.data()))
            {
                if (ImGui::BeginTable(
                    "Matrix",
                    4,
                    ImGuiTableFlags_Borders |
                    ImGuiTableFlags_SizingStretchSame))
                {
                    for (int row = 0; row < 4; ++row)
                    {
                        ImGui::TableNextRow();

                        for (int col = 0; col < 4; ++col)
                        {
                            ImGui::TableSetColumnIndex(col);

                            ImGui::Text("%.3f", matrix[col][row]);
                        }
                    }

                    ImGui::EndTable();
                }

                ImGui::TreePop();
            }
            return true;
        });

        registerPropertyDrawer<std::string>([](std::string_view name, PropertyValue& value)
        {
            auto& str = std::any_cast<std::string&>(value);
            return ImGui::InputText(name.data(), &str);
        });

        registerPropertyDrawer<std::vector<std::string>>([](std::string_view name, PropertyValue& value)
        {
            auto& tags = std::any_cast<std::vector<std::string>&>(value);

            if (ImGui::TreeNode(name.data()))
            {
                for (std::size_t i = 0; i < tags.size(); ++i)
                {
                    ImGui::PushID(static_cast<int>(i));

                    ImGui::InputText("##tag", &tags[i]);

                    ImGui::SameLine();

                    if (ImGui::Button("X"))
                    {
                        tags.erase(tags.begin() + i);
                        ImGui::PopID();
                        break;
                    }

                    ImGui::PopID();
                }

                if (ImGui::Button("+ Add Tag"))
                {
                    tags.emplace_back();

                    int* p = new int[10];
                    p[10] = 42; // out of bounds
                    delete[] p;
                }

                ImGui::TreePop();
            }
            return false;
        });
    }
}
