#include "PostProcessor.h"
#include <iostream>

PostProcessor::PostProcessor()
    : fbo(0), colorTexture(0), rbo(0), quadVAO(0), quadVBO(0),
      width(1280), height(720), crtEnabled(false), warpIntensity(0.0f), initialized(false) {
}

PostProcessor::~PostProcessor() {
    Cleanup();
}

bool PostProcessor::Init(int screenWidth, int screenHeight) {
    width = screenWidth;
    height = screenHeight;

    // Load post-processing shaders
    shader = Shader("resource/shaders/postprocess.vert", "resource/shaders/postprocess.frag");
    if (shader.ID == 0) {
        shader = Shader("../resource/shaders/postprocess.vert", "../resource/shaders/postprocess.frag");
    }
    if (shader.ID == 0) {
        std::cerr << "[PostProcessor] Failed to compile post-processing shaders!\n";
        return false;
    }

    SetupFBO();
    SetupQuad();

    initialized = true;
    std::cout << "[PostProcessor] Post-processing pipeline initialized (" << width << "x" << height << ").\n";
    return true;
}

void PostProcessor::SetupQuad() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void PostProcessor::SetupFBO() {
    // Generate FBO
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Create color texture attachment with HDR support (GL_RGBA16F)
    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);

    // Create depth and stencil renderbuffer
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "[PostProcessor] Framebuffer is not complete!\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::Resize(int screenWidth, int screenHeight) {
    if (width == screenWidth && height == screenHeight) return;
    width = screenWidth;
    height = screenHeight;

    if (fbo) {
        glDeleteTextures(1, &colorTexture);
        glDeleteRenderbuffers(1, &rbo);
        glDeleteFramebuffers(1, &fbo);
        fbo = colorTexture = rbo = 0;
    }
    SetupFBO();
}

void PostProcessor::BeginRender() {
    if (!initialized) return;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
}

void PostProcessor::EndRender() {
    if (!initialized) return;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessor::Render(float time) {
    if (!initialized) return;

    glDisable(GL_DEPTH_TEST);
    shader.Activate();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    shader.SetInt("uScreenTexture", 0);
    shader.SetFloat("uTime", time);
    shader.SetInt("uCRTEnabled", crtEnabled ? 1 : 0);
    shader.SetFloat("uWarpIntensity", warpIntensity);
    shader.SetVec2("uResolution", glm::vec2(static_cast<float>(width), static_cast<float>(height)));

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);
}

void PostProcessor::Cleanup() {
    if (fbo) {
        glDeleteFramebuffers(1, &fbo);
        fbo = 0;
    }
    if (colorTexture) {
        glDeleteTextures(1, &colorTexture);
        colorTexture = 0;
    }
    if (rbo) {
        glDeleteRenderbuffers(1, &rbo);
        rbo = 0;
    }
    if (quadVAO) {
        glDeleteVertexArrays(1, &quadVAO);
        quadVAO = 0;
    }
    if (quadVBO) {
        glDeleteBuffers(1, &quadVBO);
        quadVBO = 0;
    }
    shader.Delete();
    initialized = false;
}
