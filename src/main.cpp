#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_tray.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "titlebar.h"
#include "common.h"
#include "logger.h"
#include "terminal.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static const int titlebar_height = 32;
static const SDL_Point window_size = {800, titlebar_height + 600};
static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
static SDL_Tray *tray = NULL;
static SDL_TrayMenu *menu = NULL;
static bool hidden = false;
static TTF_Font* font = NULL;
static SDL_Texture* text_texture = NULL;
static const char* text = "Text example!";
static const int text_size = 32;
static SDL_FRect text_rect = {400, titlebar_height + 50, (float)strlen(text) * text_size / 3, text_size};
static Titlebar* titlebar = NULL;
static const SDL_Color titlebar_color = {60, 60, 120, 255};
static bool show_subwindows = true;
static bool exit_on_close = true;
static Logger *logger = NULL;
static Terminal *terminal = NULL;

void SDLCALL quit_callback(void *userdata, SDL_TrayEntry *entry) {
    SDL_Event event;
    event.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&event);
}

void SDLCALL submenu_callback(void *userdata, SDL_TrayEntry *entry) {
    SDL_Log("Submenu clicked.");
}

void hide_show() {
    if(hidden) {
        SDL_ShowWindow(window);
    }
    else {
        SDL_HideWindow(window);
    }
    hidden = !hidden;
}

void SDLCALL hide_show_callback(void *userdata, SDL_TrayEntry *entry) {
    hide_show();
}


SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    logger = new Logger("Log");
    logger->add("Hello, world!");
    logger->add("Hey, you!");
    logger->add("This is\n a test.");

    terminal = new Terminal("Terminal");
    terminal->execute("ls");
    terminal->execute("pwd");
    terminal->execute("echo Hello, world!");

    SDL_SetAppMetadata("Example HUMAN READABLE NAME", "1.0", "com.example.CATEGORY-NAME");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("examples/CATEGORY/NAME", window_size.x, window_size.y,
    SDL_WINDOW_BORDERLESS, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    if (!TTF_Init()) {
        SDL_Log("Couldn't initialize TTF: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    font = TTF_OpenFont("assets/font.ttf", 60);
    if(!font) {
        SDL_Log("Couldn't load font: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    text_texture = CreateTextTexture(renderer, font, text, {0, 0, 0, 255});

    auto img_surf = IMG_Load("assets/close.png");
    if (!img_surf) {
        SDL_Log("Couldn't load image: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    tray = SDL_CreateTray(img_surf, "Example");
    SDL_DestroySurface(img_surf);

    // Tray
    menu = SDL_CreateTrayMenu(tray);
    SDL_SetTrayTooltip(tray, "Example");
    SDL_InsertTrayEntryAt(menu, 0, "Quit", SDL_TRAYENTRY_BUTTON);
    SDL_InsertTrayEntryAt(menu, 1, "Submenu", SDL_TRAYENTRY_SUBMENU);
    SDL_InsertTrayEntryAt(menu, 2, "Hide/Show", SDL_TRAYENTRY_BUTTON);
    int size;
    auto items = SDL_GetTrayEntries(menu, &size);
    SDL_CreateTraySubmenu((SDL_TrayEntry*)items[1]);
    SDL_InsertTrayEntryAt(SDL_GetTraySubmenu((SDL_TrayEntry*)items[1]), 0, "Submenu item", SDL_TRAYENTRY_BUTTON);
    SDL_SetTrayEntryCallback((SDL_TrayEntry*)items[0], quit_callback, NULL);
    auto submenu = SDL_GetTraySubmenu((SDL_TrayEntry*)items[1]);
    auto sub_items = SDL_GetTrayEntries(submenu, &size);
    SDL_SetTrayEntryCallback((SDL_TrayEntry*)sub_items[0], submenu_callback, NULL);
    SDL_SetTrayEntryCallback((SDL_TrayEntry*)items[2], hide_show_callback, NULL);

    // Titlebar
    titlebar = new Titlebar(renderer, window, "Example", titlebar_height, titlebar_color, font);

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    static int res;
    ImGui_ImplSDL3_ProcessEvent(event);
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    else if(event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        if(exit_on_close) return SDL_APP_SUCCESS;
        hide_show();
    }
    else if(titlebar->events(event)) {
        res = titlebar->event_type();
        if(res == 1) {
            if(exit_on_close) return SDL_APP_SUCCESS;
            hide_show();
        }
        else if(res == 2) {
            SDL_MinimizeWindow(window);
        }
        else if(res == 3) {
            show_subwindows = !show_subwindows;
        }
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    if(show_subwindows) {
        {
            ImGui::Begin("Hello, world!", NULL, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("This is some useful text.");
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            if (ImGui::Button("Button")) {
                SDL_Log("Button pressed.");
            }
            ImGui::Checkbox("Exit on close", &exit_on_close);
            if(ImGui::Button("Add log")) {
                logger->add("This is a log message.");
            }
            ImGui::End();
        }

        logger->draw();
        terminal->draw();
    }

    ImGui::Render();
    SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, text_texture, NULL, &text_rect);
    titlebar->draw();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    delete terminal;
    delete logger;
    delete titlebar;
    SDL_DestroyTray(tray);
    SDL_DestroyTexture(text_texture);
    TTF_CloseFont(font);
    TTF_Quit();
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}
