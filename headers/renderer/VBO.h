#ifndef VBO_CLASS_H
#define VBO_CLASS_H

#include <glad/glad.h>

class VBO {
public:
    GLuint ID;
    VBO();
    VBO(const void* vertices, GLsizeiptr size, GLenum usage = GL_STATIC_DRAW);
    
    void BufferData(const void* vertices, GLsizeiptr size, GLenum usage = GL_STATIC_DRAW);
    void Bind() const;
    void Unbind() const;
    void Delete();
};

#endif