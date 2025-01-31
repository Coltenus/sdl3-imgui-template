#include "rectangle.h"
#include "rectangle_shaders.h"
#include <array>
#include "common.h"

namespace utils {

Rectangle::Rectangle(glm::vec2 position, glm::vec2 size, glm::vec3 color, glm::vec2 win_size)
: position(position), size(size), color(color) {
    shader = new Shader(rectangle_vertex, rectangle_fragment, false);

    std::array<float,8> vert = {
        convert_to_float(position.x, win_size.x), convert_to_float(position.y, win_size.y),
        convert_to_float(position.x + size.x, win_size.x), convert_to_float(position.y, win_size.y),
        convert_to_float(position.x + size.x, win_size.x), convert_to_float(position.y + size.y, win_size.y),
        convert_to_float(position.x, win_size.x), convert_to_float(position.y + size.y, win_size.y)
    };

    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);
    GLuint vboID;
    glGenBuffers(1, &vboID);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, vert.size()*sizeof(float), &vert[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);
}

Rectangle::~Rectangle() {
    delete shader;
}

void Rectangle::draw() {
    shader->use();
    shader->set_vec4("col", glm::vec4(color, 1.0f));
    glBindVertexArray(vaoID);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void Rectangle::set_color(glm::vec3 color) {
    shader->set_vec4("col", glm::vec4(color, 1.0f));
}

void Rectangle::set_color(glm::vec4 color) {
    shader->set_vec4("col", color);
}

};
