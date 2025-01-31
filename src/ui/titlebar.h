#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include "button.h"
#include <vector>
#include <array>
#include "../utils/text_renderer.h"
#include "../utils/rectangle.h"

namespace ui {

class Titlebar {
private:
    glm::vec2 size_, win_size;
    glm::vec4 color, inverted;
    std::vector<Button*> buttons;
    int event_type_, dragging;
    glm::vec2 offset;
    utils::TextRenderer* text_renderer;
    std::string title;
    utils::Rectangle* rect;
    
public:
    Titlebar(const char* title, float height, glm::vec4 color, glm::vec2 win_size);
    ~Titlebar();
    void draw();
    int events(SDL_Event* e);

    glm::vec2 size() { return size_; }
    int event_type() { return event_type_; }
};

};

#endif
