export module Editor.Panels.AssetBrowser;
import AssetManager;
import Core;
import Editor.Controller;
import Editor.Icons;
import Editor.Panel.Impl;
import ImGui;

struct FolderNode
{
    std::filesystem::path path;
    std::vector<FolderNode> children;
    bool selected{};
};

struct FolderHierarchy
{
    std::vector<FolderNode> roots;
};

enum class AssetType
{
    None,
    Texture,
    Mesh,
    Directory,
    GenericFile,
};

struct AssetTypeData
{
    std::string_view icon;
};

struct AssetNode
{
    AssetType type{};
    std::filesystem::path path;
    bool selected{};
};

struct AssetBrowserSnapshot
{
    FolderHierarchy folders;
    std::vector<AssetNode> assets;
};

namespace Requests
{
    struct SelectAsset
    {
        std::filesystem::path path;
    };

    struct SelectFolder
    {
        std::filesystem::path path;
    };
}

using AssetBrowserRequest = std::variant<
    Requests::SelectAsset,
    Requests::SelectFolder
>;

namespace
{
    constexpr auto assetDragType = "Asset";
    std::unordered_map<AssetType, AssetTypeData> assetTypes{
        {AssetType::Mesh, {.icon = Editor::Icons::Mesh}},
        {AssetType::Texture, {.icon = Editor::Icons::Image}},
        {AssetType::GenericFile, {.icon = Editor::Icons::File}},
        {AssetType::Directory, {.icon = Editor::Icons::FolderClosed}},
    };
}

class AssetBrowserController : public EditorControllerImpl<AssetBrowserController, AssetBrowserSnapshot, AssetBrowserRequest>
{
public:
    using EditorControllerImpl::EditorControllerImpl;

    void execute(Requests::SelectAsset&& request)
    {
        m_selectedAssets = {std::move(request.path)};
    }

    void execute(Requests::SelectFolder&& request)
    {
        m_selectedFolder = request.path;
    }

private:
    AssetBrowserSnapshot buildSnapshot(const EditingContext& context) override;

    std::vector<FolderNode> getChildDirectories(const std::filesystem::path& relativePath);

    static AssetType getAssetType(const std::filesystem::path& path);

    std::filesystem::path m_selectedFolder{};
    std::unordered_set<std::filesystem::path> m_selectedAssets{};
};

AssetBrowserSnapshot AssetBrowserController::buildSnapshot(const EditingContext& context)
{
    AssetBrowserSnapshot result;

    if (!context.projectRoot)
        return result;

    const std::filesystem::path contentRoot = services().assets.getContentRoot(*context.projectRoot);
    result.folders.roots = {
        FolderNode{
            .path = contentRoot,
            .children = getChildDirectories("."),
            .selected = m_selectedFolder == contentRoot
        }
    };

    if (!m_selectedFolder.empty())
    {
        std::vector<std::filesystem::path> files = services().assets.listFiles(*context.projectRoot, m_selectedFolder);
        result.assets.reserve(files.size());

        for (std::filesystem::path& file : files)
        {
            const bool selected = m_selectedAssets.contains(file);
            result.assets.push_back({.type = getAssetType(file), .path = std::move(file), .selected = selected});
        }
    }

    return result;
}

std::vector<FolderNode> AssetBrowserController::getChildDirectories(const std::filesystem::path& relativePath)
{
    auto directories = services().assets.listDirectories(*context().projectRoot, relativePath);

    std::vector<FolderNode> nodes;
    nodes.reserve(directories.size());

    for (auto path : directories)
    {
        nodes.push_back(FolderNode{.path = path, .children = getChildDirectories(path), .selected = path == m_selectedFolder});
    }
    return nodes;
}

AssetType AssetBrowserController::getAssetType(const std::filesystem::path& path)
{
    if (std::filesystem::is_directory(path))
        return AssetType::Directory;

    if (!path.has_extension())
        return AssetType::GenericFile;

    if (path.extension() == ".obj")
        return AssetType::Mesh;

    if (path.extension() == ".png"
        || path.extension() == ".jpg")
        return AssetType::Texture;

    return AssetType::GenericFile;
}

export namespace Panels
{
    class AssetBrowserPanel : public PanelImpl<AssetBrowserController>
    {
    public:
        using PanelImpl::PanelImpl;
        static constexpr auto Name = "Asset Browser";

    private:
        void doDraw() override
        {
            SnapshotView snapshot = getSnapshot();
            if (!snapshot.get())
                return;

            ImGui::Begin(Name, &m_open);

            const float splitterWidth = 6.0f;

            ImGui::BeginChild(
                "Folders",
                ImVec2{m_folderWidth, 0},
                ImGuiChildFlags_None,
                ImGuiWindowFlags_AlwaysVerticalScrollbar
            );

            for (const FolderNode& node : snapshot->folders.roots)
            {
                drawFolder(node);
            }

            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::InvisibleButton(
                "##vertical_splitter",
                ImVec2{splitterWidth, -1}
            );

            if (ImGui::IsItemActive())
            {
                m_folderWidth += ImGui::GetIO().MouseDelta.x;

                m_folderWidth = std::clamp(
                    m_folderWidth,
                    150.0f,
                    500.0f
                );
            }

            if (ImGui::IsItemHovered())
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
            }

            ImGui::SameLine();

            ImGui::BeginChild(
                "Content",
                ImVec2{0, 0},
                ImGuiChildFlags_None
            );

            for (const AssetNode& file : snapshot->assets)
            {
                ImGui::Selectable(std::format("{} {}", assetTypes.at(file.type).icon, file.path.filename()).c_str(), file.selected);

                if (ImGui::IsItemClicked())
                {
                    request(Requests::SelectAsset{file.path});
                }

                if (file.type == AssetType::Directory && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                {
                    request(Requests::SelectFolder{file.path});
                }
            }

            ImGui::EndChild();

            ImGui::End();
        }

        void drawFolder(const FolderNode& node)
        {
            ImGuiTreeNodeFlags flags =
                    ImGuiTreeNodeFlags_DefaultOpen |
                    ImGuiTreeNodeFlags_OpenOnArrow |
                    ImGuiTreeNodeFlags_SpanFullWidth;

            if (node.children.empty())
                flags |= ImGuiTreeNodeFlags_Leaf;

            if (node.selected)
                flags |= ImGuiTreeNodeFlags_Selected;

            ImGui::PushID(node.path.string().c_str());

            const bool expanded = ImGui::TreeNodeEx("##node", flags);

            if (ImGui::IsItemClicked())
            {
                request(Requests::SelectFolder{node.path});
            }

            ImGui::SameLine();

            const std::string_view icon = expanded ? Editor::Icons::FolderOpen : Editor::Icons::FolderClosed;

            ImGui::TextUnformatted(icon.data());

            ImGui::SameLine();

            ImGui::TextUnformatted(node.path.filename().c_str());

            if (expanded)
            {
                for (auto& child : node.children)
                    drawFolder(child);

                ImGui::TreePop();
            }

            ImGui::PopID();
        }

        bool m_open{true};
        float m_folderWidth{300.f};
    };
}
