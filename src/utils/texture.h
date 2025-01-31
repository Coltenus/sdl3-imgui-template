#ifndef TEXTURE_H
#define TEXTURE_H

#include <GL/glew.h>
#include <GL/gl.h>
#include "shader.h"

namespace utils {

class Texture {
private:
    Shader *shader;
    unsigned char* data;
    GLuint texture;
    unsigned int VAO, VBO, EBO;
    glm::vec2 position, size, win_size;
    bool buffers_initialized, texture_initialized;

public:
    Texture(const char* path, glm::vec2 position, glm::vec2 size, glm::vec2 win_size);
    ~Texture();
    void draw();
    void clear_texture();
    void init_texture(const char* path);
    void clear_buffers();
    void init_buffers();
    void set_position(glm::vec2 position);
    void set_size(glm::vec2 size);
    void set_win_size(glm::vec2 win_size);
};

};

#endif
