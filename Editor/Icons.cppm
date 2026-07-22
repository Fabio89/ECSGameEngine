module;

#include "External/IconsFontAwesome6.h"

export module Editor.Icons;
export import Core;
export import ImGui;

export namespace Editor::Icons
{
    inline constexpr ImWchar fontRange[] =
    {
        ICON_MIN_FA,
        ICON_MAX_FA,
        0
    };

    inline constexpr std::string_view File = ICON_FA_FILE;
    inline constexpr std::string_view Image = ICON_FA_IMAGE;
    inline constexpr std::string_view Entity = ICON_FA_CUBE;
    inline constexpr std::string_view Mesh = ICON_FA_CUBE;
    inline constexpr std::string_view Camera = ICON_FA_VIDEO;
    inline constexpr std::string_view FolderClosed = ICON_FA_FOLDER;
    inline constexpr std::string_view FolderOpen = ICON_FA_FOLDER_OPEN;
}
