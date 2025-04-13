#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlrenderer3.h"
#include "imgui_internal.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif
#include <SDL3_image/SDL_image.h>
#include <SDL3/SDL_tray.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "ui/titlebar.h"
#include "utils/common.h"
#include "ui/logger.h"
#include "ui/terminal.h"
#include "utils/serial_port.h"
#include "utils/request.h"

#ifdef WIN32
#include <windows.h>
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000000;
}
#endif //def WIN32

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static std::string ini_path;
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
static ui::Titlebar* titlebar = NULL;
static const SDL_Color titlebar_color = {60, 60, 120, 255};
static bool show_subwindows = true;
static bool exit_on_close = true;
static ui::Logger *logger = NULL;
static ui::Terminal *terminal = NULL;
static bool texture_choice = false;
static char serial_buffer[64];

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

static void* UserData_ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name)
{
    if(strcmp(name, "Settings") == 0) {
        return (void*)name;
    }
    return NULL;
}

static void UserData_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line)
{
    int c;
    ImVec4 buf_vec4;
    if(sscanf(line, "ExitOnClose=%d", &c) == 1)
        exit_on_close = (c != 0);
    else if (sscanf(line, "Choice=%d", &c) == 1)
        texture_choice = (c != 0);
    else if(sscanf(line, "Color=%f,%f,%f,%f", &clear_color.x, &clear_color.y, &clear_color.z, &clear_color.w) == 4)
        clear_color = ImVec4(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
}

static void UserData_WriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
{ 
    buf->appendf("[%s][%s]\n", "UserData", "Settings");
    buf->appendf("ExitOnClose=%d\n", exit_on_close);
    buf->appendf("Choice=%d\n", texture_choice);
    buf->appendf("Color=%f,%f,%f,%f\n", clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    buf->append("\n");
}

SDL_AppResult window_init(int argc, char *argv[]) {
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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ini_path = SDL_GetBasePath() + std::string("app.ini");
    io.IniFilename = ini_path.c_str();

    ImGuiSettingsHandler ini_handler;
    ini_handler.TypeName = "UserData";
    ini_handler.TypeHash = ImHashStr("UserData");
    ini_handler.ReadOpenFn = UserData_ReadOpen;
    ini_handler.ReadLineFn = UserData_ReadLine;
    ini_handler.WriteAllFn = UserData_WriteAll;
    ImGui::AddSettingsHandler(&ini_handler);

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

    return SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    logger = new ui::Logger("Log");
    logger->add("Hello, world!");
    logger->add("Hey, you!");
    logger->add("This is\n a test.");

    terminal = new ui::Terminal("Terminal");
    terminal->execute("ls");
    terminal->execute("pwd");
    terminal->execute("echo Hello, world!");

    auto result = window_init(argc, argv);
    if(result != SDL_APP_SUCCESS) {
        return result;
    }

    text_texture = utils::CreateTextTexture(renderer, font, text, {0, 0, 0, 255});

    // Titlebar
    titlebar = new ui::Titlebar(renderer, window, "Example", titlebar_height, titlebar_color, font);

    memset(serial_buffer, 0, sizeof(serial_buffer));

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    static int res;
    if(titlebar->events(event)) {
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
        if(res != 0) return SDL_APP_CONTINUE;
    }
    ImGui_ImplSDL3_ProcessEvent(event);
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;
    }
    else if(event->type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
        if(exit_on_close) return SDL_APP_SUCCESS;
        hide_show();
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
            ImGui::Checkbox("Texture", &texture_choice);
            ImGui::InputText("Serial command", serial_buffer, sizeof(serial_buffer), ImGuiInputTextFlags_CharsNoBlank);
            if(ImGui::Button("Send")) {
                utils::SerialPort serial_port;
                const char* port_name = "COM13";
                std::string command = serial_buffer;
                if(serial_port.open(port_name, 115200)) {
                    logger->add("Opened serial port: %s", port_name);
                    serial_port.write(command);
                    std::string response = serial_port.read_delim("\r\n");
                    serial_port.close();
                }
                else {
                    logger->add("Failed to open serial port.");
                }
            }
            if(ImGui::Button("Send request")) {
                try {
                    Request request;
                    request.setUrl("https://api.jikan.moe/v4/top/anime?sfw");
                    request.setMethod("GET");
                    request.setHeader("Content-Type", "application/json");
                    request.setTimeout(5);
                    request.setFollowRedirects(true);
                    request.sendRequest();
                    auto response = request.getResponseJson();
                    request.clean();
                    logger->add("Response: %s", response["data"][0]["title"].get<std::string>().c_str());
                } catch (const std::exception &e) {
                    logger->add("Error: %s", e.what());
                }
            }
            ImGui::End();
        }

        logger->draw();
        terminal->draw();
    }

    ImGui::Render();
    SDL_SetRenderDrawColorFloat(renderer, clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    SDL_RenderClear(renderer);
    if(texture_choice) SDL_RenderTexture(renderer, text_texture, NULL, &text_rect);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
    titlebar->draw();
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
    SDL_DestroyWindow(window);
}
