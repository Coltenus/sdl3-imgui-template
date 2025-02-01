#ifndef SCENE_H
#define SCENE_H

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <imgui.h>

#include "../utils/frame_buffer.h"


namespace ui {

class Scene {
private:
    utils::FrameBuffer* frame_buffer;
    ImVec4 clear_color;
    ImVec2 size;
    bool fixed_size;

public:
    Scene(bool fixed = false, ImVec2 size = ImVec2(400, 300), ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f));
    ~Scene();

    void bind();
    void unbind();
    void draw();
    float* get_clear_color() { return (float*)&clear_color; }
    void set_clear_color(ImVec4 color) { clear_color = color; }
};

};

#endif
