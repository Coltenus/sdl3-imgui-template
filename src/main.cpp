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

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
static SDL_Texture *texture = NULL;
static SDL_FRect dstrect = {0, 0, 100, 100};
static SDL_Tray *tray = NULL;
static SDL_TrayMenu *menu = NULL;
static bool hidden = false;
static TTF_Font* font = NULL;
static SDL_Texture* text_texture = NULL;
static SDL_FRect text_rect = {400, 50, 100, 30};

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


/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    SDL_SetAppMetadata("Example HUMAN READABLE NAME", "1.0", "com.example.CATEGORY-NAME");

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("examples/CATEGORY/NAME", 640, 480, 0, &window, &renderer)) {
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

    SDL_Surface* text_surf = TTF_RenderText_Blended(font, "Hello, world!", strlen("Hello, world!"), (SDL_Color){0, 0, 0, 255});
    if(!text_surf) {
        SDL_Log("Couldn't render text: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    text_texture = SDL_CreateTextureFromSurface(renderer, text_surf);
    SDL_DestroySurface(text_surf);

    auto img_surf = IMG_Load("assets/close.png");
    if (!img_surf) {
        SDL_Log("Couldn't load image: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    texture = SDL_CreateTextureFromSurface(renderer, img_surf);
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

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    ImGui_ImplSDL3_ProcessEvent(event);
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    else if(event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        hide_show();
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    {
        ImGui::Begin("Hello, world!");
        ImGui::Text("This is some useful text.");
        ImGui::ColorEdit3("clear color", (float*)&clear_color);
        if (ImGui::Button("Button")) {
            SDL_Log("Button pressed.");
        }
        ImGui::End();
    }

    ImGui::Render();
    SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    SDL_RenderClear(renderer);
    SDL_RenderTexture(renderer, texture, NULL, &dstrect);
    SDL_RenderTexture(renderer, text_texture, NULL, &text_rect);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SDL_DestroyTexture(texture);
    SDL_DestroyTray(tray);
    SDL_DestroyTexture(text_texture);
    TTF_CloseFont(font);
    TTF_Quit();
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}
