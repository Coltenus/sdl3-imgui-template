#ifndef CUBE_SHADERS_H
#define CUBE_SHADERS_H

const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec3 aColor;

    out vec3 color;

    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main() {
        gl_Position = projection * view * model * vec4(aPos, 1.0);
        color = aColor;
    }
)glsl";

const char* fragmentShaderSource = R"glsl(
    #version 330 core
    out vec4 FragColor;
    in vec3 color;

    void main() {
        FragColor = vec4(color, 1.0);
    }
)glsl";

#endif
