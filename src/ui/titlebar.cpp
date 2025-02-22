#include "ui/titlebar.h"
#include "utils/common.h"

namespace ui {

Titlebar::Titlebar(SDL_Renderer* ren, SDL_Window* win, const char* title, float height, SDL_Color color, TTF_Font* font)
{
    size_ = {0, height};
    int x;
    SDL_GetWindowSize(win, &x, NULL);
    size_.x = x;
    this->color = color;
    this->texture = utils::CreateTextTexture(ren, font, title, {(Uint8)(255 - color.r), (Uint8)(255 - color.g), (Uint8)(255 - color.b), 255});
    this->ren = ren;
    event_type_ = 0;
    SDL_Texture* btn_texture = utils::OpenImageTexture(ren, "assets/close.png");
    this->buttons.push_back(new Button(ren, {size_.x - 40, height / 10, height * 8 / 10, height * 8 / 10}, btn_texture, {200, 60, 60, 255}));
    btn_texture = utils::OpenImageTexture(ren, "assets/min.png");
    this->buttons.push_back(new Button(ren, {size_.x - 80, height / 10, height * 8 / 10, height * 8 / 10}, btn_texture, {60, 60, 200, 255}));
    this->buttons.push_back(new Button(ren, {size_.x - 190, height / 10}, height * 8 / 10, {150, 150, 40, 255}, "Subwindows", font));
    this->dragging = 0;
    this->offset = {0, 0};
    this->text_rect = {10, size_.y / 10, strlen(title) * height / 3, height * 8 / 10};
}

Titlebar::~Titlebar()
{
    for(auto& button : buttons) {
        delete button;
    }
    SDL_DestroyTexture(texture);
}

void Titlebar::draw()
{
    static SDL_FRect rect;
    if(ren == NULL) {
        return;
    }
    rect = {0, 0, size_.x, size_.y};
    SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(ren, &rect);
    rect = text_rect;
    SDL_RenderTexture(ren, texture, NULL, &rect);
    for(auto& button : buttons) {
        button->draw(NULL, 0);
    }
}

int Titlebar::events(SDL_Event* e)
{
    static int collision;
    static SDL_FPoint mouse_pos;
    static SDL_FRect rect;
    for(int i = 0; i < (int)buttons.size(); i++) {
        if(buttons[i]->events(e)) {
            event_type_ = i + 1;
            return 1;
        }
    }

    collision = 0;
    mouse_pos = {e->button.x, e->button.y};
    for(auto& button : buttons) {
        rect = button->rect();
        if(SDL_PointInRectFloat(&mouse_pos, &rect)) {
            collision = 1;
            break;
        }
    }
    if(!collision) {
        if (e->type == SDL_EVENT_MOUSE_BUTTON_DOWN && e->button.button == SDL_BUTTON_LEFT) {
            SDL_FPoint mouse_pos = {e->button.x, e->button.y};
            SDL_FRect rect = {0, 0, size_.x, size_.y};
            if (SDL_PointInRectFloat(&mouse_pos, &rect)) {
                dragging = 1;
                offset.x = mouse_pos.x;
                offset.y = mouse_pos.y;
            }
        } else if (e->type == SDL_EVENT_MOUSE_BUTTON_UP && e->button.button == SDL_BUTTON_LEFT) {
            dragging = 0;
        } else if (e->type == SDL_EVENT_MOUSE_MOTION && dragging) {
            float x, y;
            SDL_GetGlobalMouseState(&x, &y);
            SDL_SetWindowPosition(SDL_GetWindowFromID(e->motion.windowID), x - offset.x, y - offset.y);
        } else if (e->type == SDL_EVENT_WINDOW_RESTORED) {
            dragging = 0;
            offset = {0, 0};
        }
    }

    return 0;
}

};
