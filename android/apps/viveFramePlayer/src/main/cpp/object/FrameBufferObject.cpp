// "WaveVR SDK
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include <FrameBufferObject.h>
#include <Object.h>

#include <EGL/egl.h>
#include <GLES3/gl31.h>
#include <log.h>


#ifndef GL_TEXTURE_2D_MULTISAMPLE_ARRAY
#define GL_TEXTURE_2D_MULTISAMPLE_ARRAY        0x9102
#endif

#ifndef GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE
#define GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE       0x8D56
#endif

FrameBufferObject::FrameBufferObject(int textureId, int width, int height, bool msaa) :
        mMSAA(msaa),
        mWidth(width),
        mHeight(height),
        mFrameBufferId(0),
        mDepthBufferId(0),
        mTextureId(textureId){
        if (msaa) {
            int samples;
            glGetIntegerv(GL_MAX_SAMPLES_EXT, &samples);
            if (samples < 4 || !Object::hasGlExtension("GL_EXT_multisampled_render_to_texture"))
                msaa = false;
        }


        if (msaa) {
            initMSAA();
        } else {
            msaa = false;
            init();
        }

    // check FBO status
    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        mHasError = true;
        clear();
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBufferObject::~FrameBufferObject() {
    clear();
}

void FrameBufferObject::initMSAA() {
    // Reference to https://www.khronos.org/registry/gles/extensions/EXT/EXT_multisampled_render_to_texture.txt
    PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT =
        (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)eglGetProcAddress( "glRenderbufferStorageMultisampleEXT" );

    PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC glFramebufferTexture2DMultisampleEXT =
        (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)eglGetProcAddress( "glFramebufferTexture2DMultisampleEXT" );

    glGenRenderbuffers(1, &mDepthBufferId);
    glBindRenderbuffer(GL_RENDERBUFFER, mDepthBufferId);
    glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, 2, GL_DEPTH_COMPONENT24, mWidth, mHeight);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glGenFramebuffers(1, &mFrameBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBufferId);
    glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureId, 0, 2);
}

void FrameBufferObject::init() {
    glGenFramebuffers(1, &mFrameBufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);

    glGenRenderbuffers(1, &mDepthBufferId);
    glBindRenderbuffer(GL_RENDERBUFFER, mDepthBufferId);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mWidth, mHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBufferId);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureId, 0);
}

void FrameBufferObject::clear() {
    if (mDepthBufferId != 0) {
        glDeleteRenderbuffers(1, &mDepthBufferId);
    }
    if (mFrameBufferId != 0)
        glDeleteFramebuffers(1, &mFrameBufferId);
    mFrameBufferId = mDepthBufferId = mTextureId = 0;
}

void FrameBufferObject::bindFrameBuffer() {
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferId);
}

void FrameBufferObject::unbindFrameBuffer() {
    GLenum invalidateList[] = { GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT };
    GLsizei invalidateListCount = 2;
    glInvalidateFramebuffer(GL_FRAMEBUFFER, invalidateListCount, invalidateList);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBufferObject::glViewportScale(std::vector<float> lowerLeft, std::vector<float> topRight) {
    float widthFactor = topRight[0] - lowerLeft[0];
    float heightFactor = topRight[1] - lowerLeft[1];
    mViewportX = (int) (lowerLeft[0] * mWidth);
    mViewportY = (int) (lowerLeft[1] * mHeight);
    mViewportWidth = (unsigned int) (mWidth * widthFactor);
    mViewportHeight = (unsigned int) (mHeight * heightFactor);
    // This sample only demo the same scale factor for width and height
    mScale = widthFactor;
    glViewport(mViewportX, mViewportY, mViewportWidth, mViewportHeight);
}

void FrameBufferObject::resizeFrameBuffer(float scale) {
    mScaledWidth = (unsigned int) (mWidth * scale);
    mScaledHeight = (unsigned int) (mHeight * scale);

    if (mMSAA) {
        PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT =
                (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)eglGetProcAddress( "glRenderbufferStorageMultisampleEXT" );
        glBindRenderbuffer(GL_RENDERBUFFER, mDepthBufferId);
        glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, 2, GL_DEPTH_COMPONENT24, mScaledWidth, mScaledHeight);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

    } else {
        glBindRenderbuffer(GL_RENDERBUFFER, mDepthBufferId);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mScaledWidth, mScaledHeight);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }
}