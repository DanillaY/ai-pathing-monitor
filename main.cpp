#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <dlfcn.h>
#include <thread>
#include <unistd.h>
#include <sys/mman.h>

#include "imgui.h"
#include "shared.h"
#include "mem.h"
#include "overlay.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

typedef void (*SDL_GL_SwapWindow_t)(SDL_Window*);
static SDL_GL_SwapWindow_t real_SDL_GL_SwapWindow = nullptr;

//hijack the window rendering and show current imgui grid on top 
extern "C" void SDL_GL_SwapWindow(SDL_Window* window) {
    if (!real_SDL_GL_SwapWindow)
    {
        real_SDL_GL_SwapWindow = (SDL_GL_SwapWindow_t)dlsym(RTLD_NEXT, "SDL_GL_SwapWindow");
    }

    if (!g_init) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplSDL2_InitForOpenGL(window, SDL_GL_GetCurrentContext());
        ImGui_ImplOpenGL3_Init("#version 130");
        ImGui::GetIO().Fonts->AddFontDefault();
        ImGui::GetIO().IniFilename = NULL;
        g_init = true;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    draw_grid_window();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    real_SDL_GL_SwapWindow(window);
}

//initial patching
__attribute__((constructor))
static void init_mod() {

    std::thread([]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));

        make_text_writable_and_executable();
        patch_for_code_cave_jump(patch_addr, code_cave, &module_hook_function);

    }).detach();
}