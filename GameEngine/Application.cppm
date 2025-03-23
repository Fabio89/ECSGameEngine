export module Application;
import Ecs;
import Render.IRenderManager;
import Wrapper.Glfw;
import Wrapper.Windows;

export using ::GLFWwindow;
export using ::HWND;

//------------------------------------------------------------------------------------------------------------------------
// Application
//------------------------------------------------------------------------------------------------------------------------

extern "C" __declspec(dllexport)
inline int getCoolestNumber() { return 22; }

export extern "C" __declspec(dllexport)
void runTest(GLFWwindow* window = nullptr);

export extern "C" __declspec(dllexport)
GLFWwindow* createWindow(HWND parentHwnd, int width, int height);

export extern "C" __declspec(dllexport)
void engineInit(GLFWwindow* window);

export extern "C" __declspec(dllexport)
bool engineUpdate(GLFWwindow* window, float deltaTime);

export extern "C" __declspec(dllexport)
void engineShutdown(GLFWwindow* window);

export extern "C" __declspec(dllexport)
void setViewport(GLFWwindow* window, int x, int y, int width, int height);

//------------------------------------------------------------------------------------------------------------------------
// Input
//------------------------------------------------------------------------------------------------------------------------
typedef void (*KeyEventCallback)(int key, int action);
export extern "C" __declspec(dllexport)
void addKeyEventCallback(KeyEventCallback callback);

//------------------------------------------------------------------------------------------------------------------------
// Project
//------------------------------------------------------------------------------------------------------------------------

export extern "C" __declspec(dllexport)
void openProject(const char* path);

export extern "C" __declspec(dllexport)
void saveCurrentProject();

export extern "C" __declspec(dllexport)
void serializeScene(char* buffer, int bufferSize);

export extern "C" __declspec(dllexport)
void patchEntity(Entity entity, const char* json);