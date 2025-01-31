#ifndef BUTTON_H
#define BUTTON_H

#include <SDL3/SDL.h>
#include "../utils/rectangle.h"
#include "../utils/text_renderer.h"
#include "../utils/texture.h"

namespace ui {

class Button {
private:
    glm::vec2 position_, size_;
    utils::Rectangle* draw_rect_;
    std::string text_;
    utils::Texture* texture_;
    utils::TextRenderer* text_renderer_;

public:
    Button(glm::vec2 pos, float height, glm::vec4 color, const char* text, glm::vec2 win_size);
    Button(glm::vec2 pos, glm::vec2 size, utils::Texture* texture, glm::vec4 color, glm::vec2 win_size);
    ~Button();
    void draw(int* selected, int value);
    int events(SDL_Event* e);
    void set_width(int width);

    glm::vec2 position() { return position_; }
    glm::vec2 size() { return size_; }
};

};

#endif
