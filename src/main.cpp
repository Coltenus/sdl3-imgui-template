#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include <GL/glew.h>
#include <GL/gl.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <SDL3/SDL_opengles2.h>
#else
#include <SDL3/SDL_opengl.h>
#endif
#include <SDL3/SDL_tray.h>
#include <SDL3_image/SDL_image.h>
#include <glm/gtc/matrix_transform.hpp>
#include <tiny_obj_loader.h>

#include "ui/titlebar.h"
#include "utils/common.h"
#include "ui/logger.h"
#include "ui/terminal.h"
#include "utils/shader.h"
#include "utils/text_renderer.h"
#include "utils/texture.h"
#include "ui/cube.h"
#include "ui/scene.h"

#ifdef WIN32
#include <windows.h>
extern "C"
{
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000000;
}
#endif //def WIN32

static SDL_Window *window = NULL;
static SDL_GLContext gl_context = NULL;
static std::string ini_path;
static const int titlebar_height = 32;
static const glm::vec4 titlebar_color = {0.25f, 0.25f, 0.5f, 1.0f};
static const glm::vec2 window_size = {800, titlebar_height + 600};
static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
static SDL_Tray *tray = NULL;
static SDL_TrayMenu *menu = NULL;
static bool hidden = false;
static ui::Titlebar* titlebar = NULL;
static bool show_subwindows = true;
static bool exit_on_close = true;
static ui::Logger *logger = NULL;
static ui::Terminal *terminal = NULL;
static utils::TextRenderer *text_renderer = NULL;
static utils::Texture *texture[4] = {NULL, NULL, NULL, NULL};
static bool texture_choice = false;
static ui::Cube* cube = NULL;
static ui::Scene* scene = NULL;

void APIENTRY GLDebugMessageCallback(GLenum source, GLenum type, GLuint id,GLenum severity, GLsizei length,const GLchar* msg, const void* data) {
	printf("%d: %s\n",id, msg);
}

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
    else if(sscanf(line, "SceneColor=%f,%f,%f,%f", &buf_vec4.x, &buf_vec4.y, &buf_vec4.z, &buf_vec4.w) == 4)
        scene->set_clear_color(buf_vec4);
}

static void UserData_WriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
{ 
    buf->appendf("[%s][%s]\n", "UserData", "Settings");
    buf->appendf("ExitOnClose=%d\n", exit_on_close);
    buf->appendf("Choice=%d\n", texture_choice);
    buf->appendf("Color=%f,%f,%f,%f\n", clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    buf->appendf("SceneColor=%f,%f,%f,%f\n", scene->get_clear_color()[0], scene->get_clear_color()[1], scene->get_clear_color()[2], scene->get_clear_color()[3]);
    buf->append("\n");
}

SDL_AppResult window_init(int argc, char *argv[]) {
    const char* title = "SDL3 ImGui OpenGL Template";
    const char* app_id = "com.template.sdl3-imgui-opengl";
    SDL_SetAppMetadata(title, "1.0", app_id);

    for(int i = 1; i<argc; i++) {
        if(strncmp(argv[i], "-nvidia", 7) == 0) {
            #ifdef WIN32
            NvOptimusEnablement = 0x00000001;
            #endif
        }
        else if(strncmp(argv[i], "-intel", 6) == 0) {
            #ifdef WIN32
            NvOptimusEnablement = 0x00000000;
            #endif
        }
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow(title, window_size.x, window_size.y,
        SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    const char* glsl_version = "#version 460";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES,4);

    gl_context = SDL_GL_CreateContext(window);
    if (!gl_context) {
        SDL_Log("Couldn't create OpenGL context: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    if(!SDL_GL_SetSwapInterval(-1)) {
        SDL_GL_SetSwapInterval(1);
    }

    if(glewInit() != GLEW_OK) {
        SDL_Log("Couldn't initialize GLEW: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    
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
    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glDepthFunc( GL_LESS );
    glEnable(GL_DEPTH_TEST);

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

    {
        int minor, major;
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &major);
        SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &minor);
        printf( "OpenGL Version: %d.%d.\n", major, minor );
    }
    printf( "Renderer: %s.\n", glGetString( GL_RENDERER ) );
    printf( "Vendor: %s.\n", glGetString( GL_VENDOR ) );
    printf( "OpenGL version supported %s.\n", glGetString( GL_VERSION ) );

    titlebar = new ui::Titlebar(title, titlebar_height, titlebar_color, glm::vec2(window_size.x, window_size.y));

	glDebugMessageCallback(GLDebugMessageCallback, NULL);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    return SDL_APP_SUCCESS;
}

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    logger = new ui::Logger("Log");
    logger->add("Hello, world!");
    logger->add("Hey!");
    logger->add("This is\n a test.");

    terminal = new ui::Terminal("Terminal");
    terminal->execute("ls");
    terminal->execute("pwd");
    terminal->execute("echo Hello, world!");

    auto result = window_init(argc, argv);
    if(result != SDL_APP_SUCCESS) {
        return result;
    }

    text_renderer = new utils::TextRenderer(24, glm::vec4(0.0f, 0.2f, 0.4f, 0.8f), glm::vec2(window_size.x, window_size.y));
    text_renderer->add_text("Hello, world!", glm::vec2(10, titlebar_height), 1.0f);
    text_renderer->add_text("Hey!", glm::vec2(10, 40 + titlebar_height), 2.0f);
    text_renderer->add_text("This is a test.", glm::vec2(10, 100 + titlebar_height), 0.5f);

    texture[0] = new utils::Texture("assets/close.png", glm::vec2(600, 40 + titlebar_height), glm::vec2(32, 32), glm::vec2(window_size.x, window_size.y));
    texture[1] = new utils::Texture("assets/min.png", glm::vec2(600, 40 + titlebar_height), glm::vec2(32, 32), glm::vec2(window_size.x, window_size.y));
    texture[2] = new utils::Texture("assets/close.png", glm::vec2(600, 40 + titlebar_height), glm::vec2(128, 128), glm::vec2(window_size.x, window_size.y));
    texture[3] = new utils::Texture("assets/min.png", glm::vec2(600, 40 + titlebar_height), glm::vec2(128, 128), glm::vec2(window_size.x, window_size.y));

    cube = new ui::Cube(window_size);

    scene = new ui::Scene(true);

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
    static bool need_scene = false;
    if(hidden) {
        SDL_Delay(1000 / 60);
        return SDL_APP_CONTINUE;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    if(show_subwindows) {
        {
            ImGui::Begin("Hello, world!", NULL, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("This is some useful text.");
            ImGui::ColorEdit3("clear color", (float*)&clear_color);
            ImGui::ColorEdit3("scene clear color", scene->get_clear_color());
            if (ImGui::Button("Button")) {
                SDL_Log("Button pressed.");
            }
            ImGui::Checkbox("Exit on close", &exit_on_close);
            if(ImGui::Button("Add log")) {
                logger->add("This is a log message.");
            }
            ImGui::Checkbox("Texture", &texture_choice);
            ImGui::End();
        }

        logger->draw();
        terminal->draw();
        need_scene = scene->draw();
    }

    ImGui::Render();

    if(show_subwindows && need_scene) {
        scene->bind();
        if(texture_choice) texture[3]->draw();
        else texture[2]->draw();
        float time = SDL_GetTicks() / 1000.0f;
        cube->draw(time);
        scene->unbind();
    }

    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    text_renderer->draw();
    if(texture_choice) texture[1]->draw();
    else texture[0]->draw();
    titlebar->draw();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
        SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
    }
    SDL_GL_SwapWindow(window);
    SDL_Delay(1000 / 60);
    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    delete scene;
    delete cube;
    for(int i = 0; i<4; i++) {
        delete texture[i];
    }
    delete text_renderer;
    delete terminal;
    delete logger;
    delete titlebar;
    SDL_DestroyTray(tray);
    SDL_GL_DestroyContext(gl_context);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyWindow(window);
}
