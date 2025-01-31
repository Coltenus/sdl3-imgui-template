#include "button.h"
#include "../utils/common.h"
#include "titlebar.h"

namespace ui {

Button::Button(glm::vec2 pos, float height, glm::vec4 color, const char* text, glm::vec2 win_size)
: texture_(NULL)
{
    this->position_ = pos;
    this->size_ = {strlen(text) * height / 2, height};
    this->draw_rect_ = new utils::Rectangle({position_.x, win_size.y - position_.y - height}, {size_.x, size_.y}, color, win_size);
    this->text_ = text;
    this->text_renderer_ = new utils::TextRenderer(height * 8 / 10, {0, 0, 0, 1}, win_size);
    this->text_renderer_->add_text(text_, {position_.x + 5, position_.y}, 1.0f);
}

Button::Button(glm::vec2 pos, glm::vec2 size, utils::Texture* texture, glm::vec4 color, glm::vec2 win_size)
: text_renderer_(NULL)
{
    this->position_ = pos;
    this->size_ = size;
    this->draw_rect_ = new utils::Rectangle({position_.x, win_size.y - position_.y - size_.y}, size_, color, win_size);
    this->texture_ = texture;
}

Button::~Button()
{
    delete draw_rect_;
    delete texture_;
    delete text_renderer_;
}

void Button::draw(int* selected, int value)
{
    if(selected != NULL && *selected == value) {
        draw_rect_->set_color({0.5, 0.5, 0.5});
    }
    draw_rect_->draw();
    if(texture_ != NULL) {
        texture_->draw();
    }
    else if(text_renderer_ != NULL) {
        text_renderer_->draw();
    }
}

int Button::events(SDL_Event* e)
{
    if(e == NULL) {
        return 0;
    }
    if (e->type == SDL_EVENT_MOUSE_BUTTON_UP) {
        SDL_Point point = {(int)e->button.x, (int)e->button.y};
        SDL_Rect rect = {(int)position_.x, (int)position_.y, (int)size_.x, (int)size_.y};
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
    size_.x = width;
}

};
