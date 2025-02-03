#ifndef RECTANGLE_SHADERS_H
#define RECTANGLE_SHADERS_H

const char* rectangle_vertex = R"(#version 330 core
layout(location = 0) in vec2 position;
void main() {
    gl_Position = vec4(position, 0.0, 1.0);
}
)";

const char* rectangle_fragment = R"(#version 330 core
out vec4 color;

uniform vec4 col;
void main() {
    color = vec4(col);
}
)";

#endif