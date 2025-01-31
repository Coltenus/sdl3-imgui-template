#include "titlebar.h"
#include "../utils/common.h"

namespace ui {

Titlebar::Titlebar(const char* title, float height, glm::vec4 color, glm::vec2 win_size)
{
    this->win_size = win_size;
    size_ = {0, height};
    size_.x = win_size.x;
    this->color = color;
    this->inverted = {1.0f - color.r, 1.0f - color.g, 1.0f - color.b, color.a};
    event_type_ = 0;
    utils::Texture* btn_texture = new utils::Texture("assets/close.png", {size_.x - 40, height / 10}, {height * 8 / 10, height * 8 / 10}, {win_size.x, win_size.y});
    this->buttons.push_back(new Button({size_.x - 40, height / 10}, {height * 8 / 10, height * 8 / 10}, btn_texture, utils::convert_to_vec4_unsign({200, 60, 60, 255}), {win_size.x, win_size.y}));
    btn_texture = new utils::Texture("assets/min.png", {size_.x - 80, height / 10}, {height * 8 / 10, height * 8 / 10}, {win_size.x, win_size.y});
    this->buttons.push_back(new Button({size_.x - 80, height / 10}, {height * 8 / 10, height * 8 / 10}, btn_texture, utils::convert_to_vec4_unsign({60, 60, 200, 255}), {win_size.x, win_size.y}));
    this->buttons.push_back(new Button({size_.x - 220, height / 10}, height * 8 / 10, utils::convert_to_vec4_unsign({150, 150, 40, 255}), "Subwindows", {win_size.x, win_size.y}));
    this->dragging = 0;
    this->offset = {0, 0};
    this->win_size = {(float)win_size.x, (float)win_size.y};
    this->text_renderer = new utils::TextRenderer(height * 8 / 10, inverted, {win_size.x, win_size.y});
    this->text_renderer->add_text(title, {10, 0}, 1.0f);
    this->title = title;
    this->rect = new utils::Rectangle({0, win_size.y - size_.y}, {size_.x, size_.y}, color, {win_size.x, win_size.y});
}

Titlebar::~Titlebar()
{
    delete rect;
    delete text_renderer;
    for(auto& button : buttons) {
        delete button;
    }
}

void Titlebar::draw()
{
    rect->draw();
    text_renderer->draw();
    for(auto& button : buttons) {
        button->draw(&event_type_, 0);
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
        auto pos = button->position();
        auto size = button->size();
        rect = {pos.x, pos.y, size.x, size.y};
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
