export module EditorBridge;
import Core;
import Wrapper.Glfw;

export extern "C" __declspec(dllexport)
void editTranslation(Entity entity);

export extern "C" __declspec(dllexport)
void editRotation(Entity entity);

export extern "C" __declspec(dllexport)
void editScale(Entity entity);

export extern "C" __declspec(dllexport)
void stopEditing();

export extern "C" __declspec(dllexport)
void editorInit();

export extern "C" __declspec(dllexport)
void editorUpdate(GLFWwindow* window, float deltaTime);
