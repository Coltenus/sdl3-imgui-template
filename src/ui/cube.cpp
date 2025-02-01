#include "cube.h"
#include <glm/gtc/matrix_transform.hpp>
#include "cube_shaders.h"

namespace ui {

Cube::Cube(glm::vec2 win_size) : window_size(win_size) {
    shader = new utils::Shader(vertexShaderSource, fragmentShaderSource, false);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &vVBO);
    glGenBuffers(1, &cVBO);

    glBindVertexArray(VAO);


    glBindBuffer(GL_ARRAY_BUFFER, vVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    glBindBuffer(GL_ARRAY_BUFFER, cVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_color_buffer_data), g_color_buffer_data, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

Cube::~Cube() {
    delete shader;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &vVBO);
    glDeleteBuffers(1, &cVBO);
}

void Cube::draw(float rotationAngle) {
    glEnable(GL_DEPTH_TEST);
    shader->use();

    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), window_size.x / window_size.y, 0.1f, 100.0f); // Perspective projection
    glm::mat4 model = glm::rotate(glm::mat4(1.0f), rotationAngle, glm::vec3(0.5f, 1.0f, 0.25f));
    shader->set_matrix4fv("model", model);
    shader->set_matrix4fv("view", view);
    shader->set_matrix4fv("projection", projection);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

};