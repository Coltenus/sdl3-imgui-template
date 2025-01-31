#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
  
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

namespace utils {

class Shader
{
public:
    // the program ID
    unsigned int ID;
  
    // constructor reads and builds the shader
    Shader(const char* vertex_data, const char* fragment_data, bool path);
    ~Shader();
    // use/activate the shader
    void use();
    // utility uniform functions
    void set_bool(const std::string &name, bool value) const;  
    void set_int(const std::string &name, int value) const;   
    void set_float(const std::string &name, float value) const;
    void set_vec3(const std::string &name, glm::vec3 value) const;
    void set_vec4(const std::string &name, glm::vec4 value) const;
    void set_matrix4fv(const std::string &name, glm::mat4 value) const;

private:
    unsigned int shader_compile(const char* source, GLenum shaderType);
    unsigned int program_create(unsigned int vertex, unsigned int fragment);
};

};
  
#endif
