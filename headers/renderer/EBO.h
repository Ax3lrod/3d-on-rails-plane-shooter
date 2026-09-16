#ifndef EBO_CLASS_H
#define EBO_CLASS_H

#include <glad/glad.h>

class EBO {
public:
    GLuint ID;
    EBO();
    EBO(const GLuint* indices, GLsizeiptr size, GLenum usage = GL_STATIC_DRAW);

    void BufferData(const GLuint* indices, GLsizeiptr size, GLenum usage = GL_STATIC_DRAW);
    void Bind() const;
    void Unbind() const;
    void Delete();
};

#endif