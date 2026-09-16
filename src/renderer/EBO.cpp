#include "EBO.h"

EBO::EBO() : ID(0) {}

EBO::EBO(const GLuint* indices, GLsizeiptr size, GLenum usage) {
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, usage);
}

void EBO::BufferData(const GLuint* indices, GLsizeiptr size, GLenum usage) {
    if (ID == 0) {
        glGenBuffers(1, &ID);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, usage);
}

void EBO::Bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
}

void EBO::Unbind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void EBO::Delete() {
    if (ID != 0) {
        glDeleteBuffers(1, &ID);
        ID = 0;
    }
}