export module Engine:Application;
import :Render.IRenderManager;

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
// Project
//------------------------------------------------------------------------------------------------------------------------

export extern "C" __declspec(dllexport)
void openProject(const char* path);

export extern "C" __declspec(dllexport)
void serializeScene(char* buffer, int bufferSize);