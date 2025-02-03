#include "utils/texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "utils/texture_shaders.h"
#include "utils/common.h"

namespace utils {

Texture::Texture(const char* path, glm::vec2 position, glm::vec2 size, glm::vec2 win_size)
: position(position), size(size), win_size(win_size), buffers_initialized(false), texture_initialized(false) {
    init_texture(path);
    shader = new Shader(texture_vertex, texture_fragment, false);
    init_buffers();
    shader->use();
    shader->set_int("tex0", 0);
}

Texture::~Texture() {
    clear_buffers();
    delete shader;
    clear_texture();
}

void Texture::draw() {
    shader->use();
    shader->set_float("scale", 1.0f);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::clear_texture() {
    if(!texture_initialized) return;
    glDeleteTextures(1, &texture);
}

void Texture::init_texture(const char* path) {
    int width, height, num_channels;
    data = stbi_load(path, &width, &height, &num_channels, 0);
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::clear_buffers() {
    if(!buffers_initialized) return;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Texture::init_buffers() {
    clear_buffers();
    GLfloat vertices[] =
    { //     COORDINATES     /        COLORS      /   TexCoord  //
    	convert_to_float(position[0], win_size[0]),  -convert_to_float(position[1], win_size[1]), 0.0f,     0.0f, 1.0f, 0.0f,	0.0f, 0.0f, // Upper left corner
    	convert_to_float(position[0] + size[0], win_size[0]),  -convert_to_float(position[1], win_size[1]), 0.0f,     0.0f, 0.0f, 1.0f,	1.0f, 0.0f, // Upper right corner
    	convert_to_float(position[0] + size[0], win_size[0]),  -convert_to_float(position[1] + size[1], win_size[1]), 0.0f,     1.0f, 1.0f, 1.0f,	1.0f, 1.0f,  // Lower right corner
    	convert_to_float(position[0], win_size[0]),  -convert_to_float(position[1] + size[1], win_size[1]), 0.0f,     1.0f, 0.0f, 0.0f,	0.0f, 1.0f, // Lower left corner
    };

    GLuint indices[] =
    {
    	0, 2, 1, // Upper triangle
    	0, 3, 2 // Lower triangle
    };

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Texture::set_position(glm::vec2 position) {
    this->position = position;
    init_buffers();
}

void Texture::set_size(glm::vec2 size) {
    this->size = size;
    init_buffers();
}

void Texture::set_win_size(glm::vec2 win_size) {
    this->win_size = win_size;
    init_buffers();
}

};
