export module DevUI.PropertyDrawers;
import Math;
import Properties;
import Wrapper.ImGui;

export namespace DevUI
{
    void initPropertyDrawers()
    {
        registerDrawer<float>([](std::string_view name, void* value)
        {
            ImGui::DragFloat(name.data(), static_cast<float*>(value));
        });

        registerDrawer<Vec3>([](std::string_view name, void* value)
        {
            ImGui::DragFloat3(name.data(), &static_cast<Vec3*>(value)->x);
        });

        registerDrawer<Mat4>([](std::string_view name, void* value)
        {
            auto& matrix = *static_cast<Mat4*>(value);

            if (ImGui::TreeNode(name.data()))
            {
                if (ImGui::BeginTable(
                    "Matrix",
                    4,
                    ImGui::ImGuiTableFlags_::ImGuiTableFlags_Borders |
                    ImGui::ImGuiTableFlags_::ImGuiTableFlags_SizingStretchSame))
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
        });

        registerDrawer<std::string>([](std::string_view name, void* value)
        {
            auto& str = *static_cast<std::string*>(value);
            ImGui::InputText(name.data(), &str);
        });

        registerDrawer<std::vector<std::string>>([](std::string_view name, void* value)
        {
            auto& tags = *static_cast<std::vector<std::string>*>(value);

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
        });
    }
}
