#ifndef SHADER_CLASS_H
#define SHADER_CLASS_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

std::string get_file_contents(const char* filename);

class Shader {
public:
    GLuint ID;
    Shader();
    Shader(const char* vertexFile, const char* fragmentFile);
    Shader(const std::string& vertexSource, const std::string& fragmentSource, bool fromSource);

    void Activate() const;
    void Delete();

    void SetMat4(const std::string& name, const glm::mat4& mat) const;
    void SetVec2(const std::string& name, const glm::vec2& vec) const;
    void SetVec2(const std::string& name, float x, float y) const;
    void SetVec3(const std::string& name, const glm::vec3& vec) const;
    void SetVec3(const std::string& name, float x, float y, float z) const;
    void SetVec4(const std::string& name, const glm::vec4& vec) const;
    void SetVec4(const std::string& name, float x, float y, float z, float w) const;
    void SetFloat(const std::string& name, float value) const;
    void SetInt(const std::string& name, int value) const;

private:
    void CompileProgram(const char* vSource, const char* fSource);
    void CheckCompileErrors(GLuint shader, const std::string& type);
};

#endif