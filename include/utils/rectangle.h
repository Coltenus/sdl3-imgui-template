#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "utils/shader.h"

namespace utils {

class Rectangle {
private:
    Shader* shader;
    GLuint vaoID;

public:
    glm::vec2 position;
    glm::vec2 size;
    glm::vec3 color;

    Rectangle(glm::vec2 position, glm::vec2 size, glm::vec3 color, glm::vec2 win_size);
    ~Rectangle();
    void draw();
    void set_color(glm::vec3 color);
    void set_color(glm::vec4 color);
};

};

#endif
