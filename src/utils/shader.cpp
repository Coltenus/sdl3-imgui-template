#include "utils/shader.h"
#include <glm/gtc/type_ptr.hpp>

namespace utils {

Shader::Shader(const char* vertex_data, const char* fragment_data, bool path) {
    std::string vertexCode;
    std::string fragmentCode;
    if(path) {
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try 
        {
            // open files
            vShaderFile.open(vertex_data);
            fShaderFile.open(fragment_data);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();		
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode   = vShaderStream.str();
            fragmentCode = fShaderStream.str();		
        }
        catch(std::ifstream::failure e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
        }
    }
    else {
        vertexCode = vertex_data;
        fragmentCode = fragment_data;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;
    
    // vertex Shader
    vertex = shader_compile(vShaderCode, GL_VERTEX_SHADER);

    // fragment Shader
    fragment = shader_compile(fShaderCode, GL_FRAGMENT_SHADER);
    
    // shader Program
    ID = program_create(vertex, fragment);
    
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader() {
    glDeleteProgram(ID);
}

void Shader::use() {
    glUseProgram(ID);
}

void Shader::set_bool(const std::string &name, bool value) const
{         
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}
void Shader::set_int(const std::string &name, int value) const
{ 
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}
void Shader::set_float(const std::string &name, float value) const
{ 
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
}

void Shader::set_vec3(const std::string &name, glm::vec3 value) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), value[0], value[1], value[2]);
}

void Shader::set_vec4(const std::string &name, glm::vec4 value) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), value[0], value[1], value[2], value[3]);
}

void Shader::set_matrix4fv(const std::string &name, glm::mat4 value) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}

unsigned int Shader::shader_compile(const char* source, GLenum shaderType) {
    static char infoLog[512];
    int success;
    unsigned int shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    };
    return shader;
}

unsigned int Shader::program_create(unsigned int vertex, unsigned int fragment) {
    static char infoLog[512];
    int success;
    unsigned int id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    // print linking errors if any
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(id, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    return id;
}

};
