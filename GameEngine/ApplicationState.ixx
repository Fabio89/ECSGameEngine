export module Engine.ApplicationState;
import Engine.Decl;
import std;

export class ImGuiHelper;
export class VulkanApplication;

export struct ApplicationState
{
    std::mutex mutex;
    VulkanApplication* application{};
    World* world{};
    ImGuiHelper* debugUI{};
    bool initialized = false;
    bool closing = false;
};