#include "ui/button.h"
#include "utils/common.h"

namespace ui {

Button::Button(SDL_Renderer* ren, SDL_FPoint pos, float height, SDL_Color color, const char* text, TTF_Font* font)
{
    this->rect_ = {pos.x, pos.y, strlen(text) * height * 3 / 8, height};
    this->color = color;
    this->ren = ren;
    this->text_rect = this->rect_;
    this->text_rect.x += this->text_rect.w * 10 / 100;
    this->text_rect.y += this->text_rect.h * 5 / 100;
    this->text_rect.w -= this->text_rect.w * 20 / 100;
    this->text_rect.h -= this->text_rect.h * 10 / 100;
    this->texture = utils::CreateTextTexture(ren, font, text, {0, 0, 0, 255});
}

Button::Button(SDL_Renderer* ren, SDL_FRect rect, SDL_Texture* texture, SDL_Color color)
{
    this->rect_ = rect;
    this->ren = ren;
    this->texture = texture;
    this->text_rect = rect;
    this->color = color;
}

Button::~Button()
{
    if(texture != NULL) {
        SDL_DestroyTexture(texture);
    }
}

void Button::draw(int* selected, int value)
{
    if(ren == NULL) {
        return;
    }
    if(selected != NULL && *selected == value) {
        SDL_SetRenderDrawColor(ren, 180, 180, 180, 255);
        SDL_RenderFillRect(ren, &rect_);
    }
    else {
        SDL_SetRenderDrawColor(ren, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(ren, &rect_);
    }
    if(texture != NULL) {
        SDL_RenderTexture(ren, texture, NULL, &text_rect);
    }
}

int Button::events(SDL_Event* e)
{
    if(ren == NULL || e == NULL) {
        return 0;
    }
    if (e->type == SDL_EVENT_MOUSE_BUTTON_UP) {
        SDL_Point point = {(int)e->button.x, (int)e->button.y};
        SDL_Rect rect = {(int)rect_.x, (int)rect_.y, (int)rect_.w, (int)rect_.h};
        if(SDL_PointInRect(&point, &rect)) {
            switch (e->button.button) {
                case SDL_BUTTON_LEFT:
                    return 1;
                case SDL_BUTTON_RIGHT:
                    return 2;
                case SDL_BUTTON_MIDDLE:
                    return 3;
            }
        }
    }
    return 0;
}

void Button::set_width(int width) {
    if(texture == NULL) {
        return;
    }
    rect_.w = width;
    text_rect.x = rect_.x + width / 2 - text_rect.w / 2;
}

void Button::set_gap(int gap) {
    if(texture == NULL || gap < 0 || gap > 50) {
        return;
    }
    text_rect.y = rect_.y;
    text_rect.h = rect_.h;
    text_rect.y += text_rect.h * gap / 100;
    text_rect.h -= text_rect.h * gap * 2 / 100;
}

};
