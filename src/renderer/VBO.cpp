#include "VBO.h"

VBO::VBO() : ID(0) {}

VBO::VBO(const void* vertices, GLsizeiptr size, GLenum usage) {
    glGenBuffers(1, &ID);
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, usage);
}

void VBO::BufferData(const void* vertices, GLsizeiptr size, GLenum usage) {
    if (ID == 0) {
        glGenBuffers(1, &ID);
    }
    glBindBuffer(GL_ARRAY_BUFFER, ID);
    glBufferData(GL_ARRAY_BUFFER, size, vertices, usage);
}

void VBO::Bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, ID);
}

void VBO::Unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VBO::Delete() {
    if (ID != 0) {
        glDeleteBuffers(1, &ID);
        ID = 0;
    }
}