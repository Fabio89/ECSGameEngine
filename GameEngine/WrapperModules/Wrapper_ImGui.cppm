module;
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>

export module Wrapper.ImGui;

export namespace Wrapper_ImGui
{
    // ReSharper disable CppClangTidyMiscUnusedUsingDecls
    using ImGui::CreateContext;
    using ImGui::DestroyContext;
    using ImGui::GetCurrentContext;
    using ImGui::SetCurrentContext;

    using ImGui::GetIO;
    using ImGui::GetStyle;
    using ImGui::NewFrame;
    using ImGui::EndFrame;
    using ImGui::Render;
    using ImGui::GetDrawData;

    using ImGui::ShowDemoWindow;
    using ImGui::ShowMetricsWindow;
    using ImGui::ShowDebugLogWindow;
    using ImGui::ShowIDStackToolWindow;
    using ImGui::ShowAboutWindow;
    using ImGui::ShowStyleEditor;
    using ImGui::ShowStyleSelector;
    using ImGui::ShowFontSelector;
    using ImGui::ShowUserGuide;
    using ImGui::GetVersion;

    using ImGui::StyleColorsDark;
    using ImGui::StyleColorsLight;
    using ImGui::StyleColorsClassic;

    using ImGui::Begin;
    using ImGui::End;

    using ImGui::BeginChild;
    using ImGui::EndChild;

    using ImGui::IsWindowAppearing;
    using ImGui::IsWindowCollapsed;
    using ImGui::IsWindowFocused;
    using ImGui::IsWindowHovered;
    using ImGui::GetWindowDrawList;
    using ImGui::GetWindowPos;
    using ImGui::GetWindowSize;
    using ImGui::GetWindowWidth;
    using ImGui::GetWindowHeight;

    using ImGui::SetNextWindowPos;
    using ImGui::SetNextWindowSize;
    using ImGui::SetNextWindowSizeConstraints;
    using ImGui::SetNextWindowContentSize;
    using ImGui::SetNextWindowCollapsed;
    using ImGui::SetNextWindowFocus;
    using ImGui::SetNextWindowScroll;
    using ImGui::SetNextWindowBgAlpha;
    using ImGui::SetWindowPos;
    using ImGui::SetWindowSize;
    using ImGui::SetWindowCollapsed;
    using ImGui::SetWindowFocus;
    using ImGui::SetWindowFontScale;
    using ImGui::SetWindowPos;
    using ImGui::SetWindowSize;
    using ImGui::SetWindowCollapsed;
    using ImGui::SetWindowFocus;

    using ImGui::GetContentRegionAvail;
    using ImGui::GetContentRegionMax;
    using ImGui::GetWindowContentRegionMin;
    using ImGui::GetWindowContentRegionMax;

    using ImGui::GetScrollX;
    using ImGui::GetScrollY;
    using ImGui::SetScrollX;
    using ImGui::SetScrollY;
    using ImGui::GetScrollMaxX;
    using ImGui::GetScrollMaxY;
    using ImGui::SetScrollHereX;
    using ImGui::SetScrollHereY;
    using ImGui::SetScrollFromPosX;
    using ImGui::SetScrollFromPosY;

    using ImGui::PushFont;
    using ImGui::PopFont;
    using ImGui::PushStyleColor;
    using ImGui::PopStyleColor;
    using ImGui::PushStyleVar;
    using ImGui::PopStyleVar;
    using ImGui::PushTabStop;
    using ImGui::PopTabStop;
    using ImGui::PushButtonRepeat;
    using ImGui::PopButtonRepeat;
    using ImGui::PushItemWidth;
    using ImGui::PopItemWidth;
    using ImGui::SetNextItemWidth;
    using ImGui::CalcItemWidth;
    using ImGui::PushTextWrapPos;
    using ImGui::PopTextWrapPos;

    using ImGui::GetFont;
    using ImGui::GetFontSize;
    using ImGui::GetFontTexUvWhitePixel;
    using ImGui::GetColorU32;
    using ImGui::GetStyleColorVec4;

    using ImGui::GetCursorScreenPos;
    using ImGui::SetCursorScreenPos;
    using ImGui::GetCursorPos;
    using ImGui::GetCursorPosX;
    using ImGui::GetCursorPosY;
    using ImGui::SetCursorPos;
    using ImGui::SetCursorPosX;
    using ImGui::SetCursorPosY;
    using ImGui::GetCursorStartPos;

    using ImGui::Separator;
    using ImGui::SameLine;
    using ImGui::NewLine;
    using ImGui::Spacing;
    using ImGui::Dummy;
    using ImGui::Indent;
    using ImGui::Unindent;
    using ImGui::BeginGroup;
    using ImGui::EndGroup;
    using ImGui::AlignTextToFramePadding;
    using ImGui::GetTextLineHeight;
    using ImGui::GetTextLineHeightWithSpacing;
    using ImGui::GetFrameHeight;
    using ImGui::GetFrameHeightWithSpacing;

    using ImGui::PushID;
    using ImGui::PopID;
    using ImGui::GetID;

    using ImGui::TextUnformatted;
    using ImGui::Text;
    using ImGui::TextV;
    using ImGui::TextColored;
    using ImGui::TextColoredV;
    using ImGui::TextDisabled;
    using ImGui::TextDisabledV;
    using ImGui::TextWrapped;
    using ImGui::TextWrappedV;
    using ImGui::LabelText;
    using ImGui::LabelTextV;
    using ImGui::BulletText;
    using ImGui::BulletTextV;
    using ImGui::SeparatorText;
    using ImGui::Button;
    using ImGui::SmallButton;
    using ImGui::InvisibleButton;
    using ImGui::ArrowButton;
    using ImGui::Checkbox;
    using ImGui::CheckboxFlags;
    using ImGui::RadioButton;
    using ImGui::ProgressBar;
    using ImGui::Bullet;
    using ImGui::Image;
    using ImGui::ImageButton;

    using ImGui::BeginCombo;
    using ImGui::EndCombo;
    using ImGui::Combo;

    using ImGui::DragFloat;
    using ImGui::DragFloat2;
    using ImGui::DragFloat3;
    using ImGui::DragFloat4;
    using ImGui::DragFloatRange2;
    using ImGui::DragInt;
    using ImGui::DragInt2;
    using ImGui::DragInt3;
    using ImGui::DragInt4;
    using ImGui::DragIntRange2;
    using ImGui::DragScalar;
    using ImGui::DragScalarN;

    using ImGui::SliderFloat;
    using ImGui::SliderFloat2;
    using ImGui::SliderFloat3;
    using ImGui::SliderFloat4;
    using ImGui::SliderAngle;
    using ImGui::SliderInt;
    using ImGui::SliderInt2;
    using ImGui::SliderInt3;
    using ImGui::SliderInt4;
    using ImGui::SliderScalar;
    using ImGui::SliderScalarN;
    using ImGui::VSliderFloat;
    using ImGui::VSliderInt;
    using ImGui::VSliderScalar;

    using ImGui::InputText;
    using ImGui::InputTextMultiline;
    using ImGui::InputTextWithHint;
    using ImGui::InputFloat;
    using ImGui::InputFloat2;
    using ImGui::InputFloat3;
    using ImGui::InputFloat4;
    using ImGui::InputInt;
    using ImGui::InputInt2;
    using ImGui::InputInt3;
    using ImGui::InputInt4;
    using ImGui::InputDouble;
    using ImGui::InputScalar;
    using ImGui::InputScalarN;

    using ImGui::ColorEdit3;
    using ImGui::ColorEdit4;
    using ImGui::ColorPicker3;
    using ImGui::ColorPicker4;
    using ImGui::ColorButton;
    using ImGui::SetColorEditOptions;

    using ImGui::TreeNode;
    using ImGui::TreeNodeV;
    using ImGui::TreeNodeEx;
    using ImGui::TreeNodeExV;
    using ImGui::TreePush;
    using ImGui::TreePop;
    using ImGui::GetTreeNodeToLabelSpacing;
    using ImGui::CollapsingHeader;
    using ImGui::SetNextItemOpen;

    using ImGui::Selectable;

    using ImGui::BeginListBox;
    using ImGui::EndListBox;
    using ImGui::ListBox;

    using ImGui::PlotLines;
    using ImGui::PlotHistogram;

    using ImGui::Value;

    using ImGui::BeginMenuBar;
    using ImGui::EndMenuBar;
    using ImGui::BeginMainMenuBar;
    using ImGui::EndMainMenuBar;
    using ImGui::BeginMenu;
    using ImGui::EndMenu;
    using ImGui::MenuItem;

    using ImGui::BeginTooltip;
    using ImGui::EndTooltip;
    using ImGui::SetTooltip;
    using ImGui::SetTooltipV;

    using ImGui::BeginItemTooltip;
    using ImGui::SetItemTooltip;
    using ImGui::SetItemTooltipV;

    using ImGui::BeginPopup;
    using ImGui::BeginPopupModal;
    using ImGui::EndPopup;
    using ImGui::OpenPopup;
    using ImGui::OpenPopupOnItemClick;
    using ImGui::CloseCurrentPopup;
    using ImGui::BeginPopupContextItem;
    using ImGui::BeginPopupContextWindow;
    using ImGui::BeginPopupContextVoid;
    using ImGui::IsPopupOpen;

    using ImGui::BeginTable;
    using ImGui::EndTable;
    using ImGui::TableNextRow;
    using ImGui::TableNextColumn;
    using ImGui::TableSetColumnIndex;
    using ImGui::TableSetupColumn;
    using ImGui::TableSetupScrollFreeze;
    using ImGui::TableHeader;
    using ImGui::TableHeadersRow;
    using ImGui::TableAngledHeadersRow;
    using ImGui::TableGetSortSpecs;
    using ImGui::TableGetColumnCount;
    using ImGui::TableGetColumnIndex;
    using ImGui::TableGetRowIndex;
    using ImGui::TableGetColumnName;
    using ImGui::TableGetColumnFlags;
    using ImGui::TableSetColumnEnabled;
    using ImGui::TableSetBgColor;

    using ImGui::Columns;
    using ImGui::NextColumn;
    using ImGui::GetColumnIndex;
    using ImGui::GetColumnWidth;
    using ImGui::SetColumnWidth;
    using ImGui::GetColumnOffset;
    using ImGui::SetColumnOffset;
    using ImGui::GetColumnsCount;

    using ImGui::BeginTabBar;
    using ImGui::EndTabBar;
    using ImGui::BeginTabItem;
    using ImGui::EndTabItem;
    using ImGui::TabItemButton;
    using ImGui::SetTabItemClosed;

    using ImGui::LogToTTY;
    using ImGui::LogToFile;
    using ImGui::LogToClipboard;
    using ImGui::LogFinish;
    using ImGui::LogButtons;
    using ImGui::LogText;
    using ImGui::LogTextV;

    using ImGui::BeginDragDropSource;
    using ImGui::SetDragDropPayload;
    using ImGui::EndDragDropSource;
    using ImGui::BeginDragDropTarget;
    using ImGui::AcceptDragDropPayload;
    using ImGui::EndDragDropTarget;
    using ImGui::GetDragDropPayload;

    using ImGui::BeginDisabled;
    using ImGui::EndDisabled;

    using ImGui::PushClipRect;
    using ImGui::PopClipRect;

    using ImGui::SetItemDefaultFocus;
    using ImGui::SetKeyboardFocusHere;

    using ImGui::SetNextItemAllowOverlap;

    using ImGui::IsItemHovered;
    using ImGui::IsItemActive;
    using ImGui::IsItemFocused;
    using ImGui::IsItemClicked;
    using ImGui::IsItemVisible;
    using ImGui::IsItemEdited;
    using ImGui::IsItemActivated;
    using ImGui::IsItemDeactivated;
    using ImGui::IsItemDeactivatedAfterEdit;
    using ImGui::IsItemToggledOpen;
    using ImGui::IsAnyItemHovered;
    using ImGui::IsAnyItemActive;
    using ImGui::IsAnyItemFocused;
    using ImGui::GetItemID;
    using ImGui::GetItemRectMin;
    using ImGui::GetItemRectMax;
    using ImGui::GetItemRectSize;

    using ImGui::GetMainViewport;

    using ImGui::GetBackgroundDrawList;
    using ImGui::GetForegroundDrawList;

    using ImGui::IsRectVisible;
    using ImGui::GetTime;
    using ImGui::GetFrameCount;
    using ImGui::GetDrawListSharedData;
    using ImGui::GetStyleColorName;
    using ImGui::SetStateStorage;
    using ImGui::GetStateStorage;

    using ImGui::CalcTextSize;

    using ImGui::ColorConvertU32ToFloat4;
    using ImGui::ColorConvertFloat4ToU32;
    using ImGui::ColorConvertRGBtoHSV;
    using ImGui::ColorConvertHSVtoRGB;

    using ImGui::IsKeyDown;
    using ImGui::IsKeyPressed;
    using ImGui::IsKeyReleased;
    using ImGui::IsKeyChordPressed;
    using ImGui::GetKeyPressedAmount;
    using ImGui::GetKeyName;
    using ImGui::SetNextFrameWantCaptureKeyboard;

    using ImGui::Shortcut;
    using ImGui::SetNextItemShortcut;

    using ImGui::IsMouseDown;
    using ImGui::IsMouseClicked;
    using ImGui::IsMouseReleased;
    using ImGui::IsMouseDoubleClicked;
    using ImGui::GetMouseClickedCount;
    using ImGui::IsMouseHoveringRect;
    using ImGui::IsMousePosValid;
    using ImGui::IsAnyMouseDown;
    using ImGui::GetMousePos;
    using ImGui::GetMousePosOnOpeningCurrentPopup;
    using ImGui::IsMouseDragging;
    using ImGui::GetMouseDragDelta;
    using ImGui::ResetMouseDragDelta;
    using ImGui::GetMouseCursor;
    using ImGui::SetMouseCursor;
    using ImGui::SetNextFrameWantCaptureMouse;

    using ImGui::GetClipboardText;
    using ImGui::SetClipboardText;

    using ImGui::LoadIniSettingsFromDisk;
    using ImGui::LoadIniSettingsFromMemory;
    using ImGui::SaveIniSettingsToDisk;
    using ImGui::SaveIniSettingsToMemory;

    using ImGui::DebugTextEncoding;
    using ImGui::DebugFlashStyleColor;
    using ImGui::DebugStartItemPicker;
    using ImGui::DebugCheckVersionAndDataLayout;

    using ImGui::SetAllocatorFunctions;
    using ImGui::GetAllocatorFunctions;
    using ImGui::MemAlloc;
    using ImGui::MemFree;

    using VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;

    using ImGuiConfigFlags_::ImGuiConfigFlags_NavEnableKeyboard;
    using ImGuiConfigFlags_::ImGuiConfigFlags_NavEnableGamepad;

    using ImGuiChildFlags_::ImGuiChildFlags_Border;
    
    using ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen;
    
    using ImGuiStyleVar_::ImGuiStyleVar_ItemSpacing;
    using ImGuiStyleVar_::ImGuiStyleVar_ItemInnerSpacing;

    using ImGuiMouseCursor_::ImGuiMouseCursor_ResizeEW;
    
    using ::ImVec2;
    using ::ImGuiIO;
    using ::ImGui_ImplVulkan_InitInfo;
    
    using ::ImGui_ImplGlfw_InitForVulkan;
    using ::ImGui_ImplGlfw_NewFrame;
    using ::ImGui_ImplGlfw_Shutdown;
    using ::ImGui_ImplVulkan_Init;
    using ::ImGui_ImplVulkan_NewFrame;
    using ::ImGui_ImplVulkan_RenderDrawData;
    using ::ImGui_ImplVulkan_Shutdown;
    
    void CheckVersion() { IMGUI_CHECKVERSION(); }
}