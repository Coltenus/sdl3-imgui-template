#ifndef TEXT_RENDERER_H
#define TEXT_RENDERER_H

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>
#include <vector>
#include <string>
#include "shader.h"

namespace utils {

struct Character {
    GLuint TextureID;
    glm::ivec2 Size;
    glm::ivec2 Bearing;
    GLuint Advance;
};

class TextRenderer {
private:
    std::map<char, Character> Characters;
    std::vector<std::tuple<std::string, GLuint, GLuint>> buffers;
    utils::Shader *shader;
    glm::vec2 win_size;
    int fontSize;
    glm::vec4 color;
    bool load_font(const char* fontPath, int fontSize);
    GLuint compile_shader(const char* source, GLenum shaderType);
    GLuint create_shader_program(const char* vertexSource, const char* fragmentSource);

public:
    TextRenderer(int fontSize, glm::vec4 color, glm::vec2 win_size);
    ~TextRenderer();
    void add_text(const std::string &text, const glm::vec2 &position, GLfloat scale);
    void draw();
};

};

#endif
