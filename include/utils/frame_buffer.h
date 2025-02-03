#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>

namespace utils {

class FrameBuffer {
public:
    FrameBuffer();
    ~FrameBuffer();

    void bind();
    void unbind();
    void rescale(glm::vec2 size);
    GLuint get_texture() { return texture; }

private:
    GLuint fbo;
    GLuint texture;
    GLuint rbo;
};

};

#endif
