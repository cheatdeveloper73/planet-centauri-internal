#define _CRT_SECURE_NO_WARNINGS
#include "Includes.h"
#include "game.h"

twglSwapBuffers oSwapBuffers = NULL;
WNDPROC oWndProc;
static HWND Window = NULL;

HMODULE g_module{};

int init = false;
bool show = true;
bool uninject = false;

BOOL __stdcall hkSwapBuffers(_In_ HDC hDc)
{

    if (init == FALSE)
    {

        glewExperimental = GL_TRUE;

        if (glewInit() == GLEW_OK)
        {

            ImGui::CreateContext();

            ImGuiIO& io = ImGui::GetIO(); io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;

            ImGui_ImplWin32_Init(Window);
            ImGui_ImplOpenGL3_Init();

            init = TRUE;

        }

    }

    if (GetAsyncKeyState(VK_INSERT) & 1)
        show = !show;

    if (uninject)
    {

        MH_DisableHook(MH_ALL_HOOKS);
        SetWindowLongPtrW(Window, GWL_WNDPROC, (LONG_PTR)oWndProc); // Reset WndProc

    }

    if (GetAsyncKeyState(VK_END) & 1) // Unload
        uninject = true;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    if (show)
    {

        ImGui::Begin("ImGui Window");

        ImGui::End();

    }

    class BoundingBox {
    private:
        float x1, y1, x2, y2;

    public:
        // Constructor
        BoundingBox(float x1, float y1, float x2, float y2)
            : x1(x1), y1(y1), x2(x2), y2(y2) {
        }

        // Setters
        void setCoordinates(float x1, float y1, float x2, float y2) {
            this->x1 = x1;
            this->y1 = y1;
            this->x2 = x2;
            this->y2 = y2;
        }

        // Getters
        float getX1() const { return x1; }
        float getY1() const { return y1; }
        float getX2() const { return x2; }
        float getY2() const { return y2; }

        // Method to get the center of the bounding box
        ImVec2 getCenter() const {
            ImVec2 ret{};
            ret.x = (x1 + x2) / 2.0f;
            ret.y = (y1 + y2) / 2.0f;
            return ret;
        }
    };

    if (g_game_state && g_world_context) {
        auto alive_objects_begin = g_world_context->get_objects_begin();
        auto alive_objects_end = g_world_context->get_objects_end();
        auto object_idx = 0;

        auto game = g_world_context->get_game();

        auto io = ImGui::GetIO();

        auto screen_width = io.DisplaySize.x;  // Screen width
        auto screen_height = io.DisplaySize.y; // Screen height

        auto zoom = g_game_state->get_zoom();

        while (alive_objects_begin != alive_objects_end) {
            auto alive_object = *alive_objects_begin;

            auto view = alive_object->get_view();
            auto camera_left = view->left();
            auto camera_top = view->top();
            auto camera_right = view->right();
            auto camera_bottom = view->bottom();

            // Camera bounds
            BoundingBox camera_bounds(camera_left, camera_top, camera_right, camera_bottom);
            auto camera_center = camera_bounds.getCenter();

            // Object world position
            auto object_pos = reinterpret_cast<ImVec2*>((char*)alive_object + *(uintptr_t*)(*(uintptr_t*)alive_object - 32i64) + 32);

            // Calculate the screen position
            ImVec2 screen_center(screen_width / 2.0f, screen_height / 2.0f);
            ImVec2 screen_pos(
                screen_center.x + (camera_center.x - object_pos->x),
                screen_center.y - (camera_center.y - object_pos->y)
            );

            // Debug output
            printf("[%d] Screen Position: x=%.6f, y=%.6f\n", object_idx, screen_pos.x, screen_pos.y);

            // Render text at calculated screen position
            ImGui::GetBackgroundDrawList()->AddText({ screen_pos.x, screen_pos.y }, IM_COL32_WHITE, "obj");

            ++alive_objects_begin;
            ++object_idx;
        }
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    return oSwapBuffers(hDc);

}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{

    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;

    return CallWindowProcW(oWndProc, hWnd, uMsg, wParam, lParam);

}

DWORD WINAPI Initalization()
{

    while (GetModuleHandleA("opengl32.dll") == NULL)
        Sleep(100);

    AllocConsole();

    if (!freopen("CONOUT$", "w", stdout))
    {

        FreeConsole();
        return NULL;

    }

    HMODULE hMod = GetModuleHandleA("opengl32.dll");

    printf("[+] opengl base: %p\n", hMod);

    if (hMod)
    {

        void* ptr = GetProcAddress(hMod, "wglSwapBuffers");

        MH_Initialize();

        MH_CreateHook(ptr, hkSwapBuffers, reinterpret_cast<void**>(&oSwapBuffers));

        emplace_hooks();

        MH_EnableHook(MH_ALL_HOOKS);

        while (!Window)
            Window = FindWindowA(NULL, "Planet Centauri");

        oWndProc = (WNDPROC)SetWindowLongPtrW(Window, GWL_WNDPROC, (LONG_PTR)WndProc);

        while (true)
        {

            if (uninject)
                break;

            Sleep(1000);

        }

        system("cls");

        auto console_window = GetConsoleWindow();

        FreeConsole();
        PostMessageA(console_window, xi(WM_CLOSE), xi(0), xi(0));
        MH_Uninitialize();
        FreeLibraryAndExitThread(g_module, 0);

        return true;

    }
    else
        return false;

}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{

    switch (ul_reason_for_call)
    {

        case DLL_PROCESS_ATTACH:

            g_module = hModule;
            DisableThreadLibraryCalls(hModule);
            CreateThread(0, 0, LPTHREAD_START_ROUTINE(Initalization), 0, 0, 0); 

        case DLL_PROCESS_DETACH:
            break;

    }

    return TRUE;

}

