export module Engine.ApplicationState;
import Engine.Decl;
import std;

export class ImGuiHelper;
export class VulkanApplication;

export struct ApplicationState
{
    VulkanApplication* application{};
    World* world{};
    ImGuiHelper* debugUI{};
};