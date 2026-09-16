#ifndef POST_PROCESSOR_H
#define POST_PROCESSOR_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shaderClass.h"

class PostProcessor {
public:
    PostProcessor();
    ~PostProcessor();

    bool Init(int screenWidth, int screenHeight);
    void Resize(int screenWidth, int screenHeight);
    void BeginRender();
    void EndRender();
    void Render(float time);
    void Cleanup();

    void ToggleCRT() { crtEnabled = !crtEnabled; }
    void SetCRTEnabled(bool enabled) { crtEnabled = enabled; }
    bool IsCRTEnabled() const { return crtEnabled; }

    void SetWarpIntensity(float intensity) { warpIntensity = intensity; }
    float GetWarpIntensity() const { return warpIntensity; }

private:
    void SetupQuad();
    void SetupFBO();

    GLuint fbo;
    GLuint colorTexture;
    GLuint rbo;
    GLuint quadVAO;
    GLuint quadVBO;

    Shader shader;
    int width;
    int height;
    bool crtEnabled;
    float warpIntensity;
    bool initialized;
};

#endif
