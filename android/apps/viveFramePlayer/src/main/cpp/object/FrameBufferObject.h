// "WaveVR SDK
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl31.h>
#include <GLES3/gl3ext.h>
#include <glad/glad.h>
#include <vector>

class FrameBufferObject {
private:
    bool mMSAA = false;
    int mWidth = 0;
    int mHeight = 0;
    float mScale = 1.0;
    int mScaledWidth = 0;
    int mScaledHeight = 0;
    int mViewportX = 0;
    int mViewportY = 0;
    unsigned int mViewportWidth = 0;
    unsigned int mViewportHeight = 0;
    GLuint mRenderFrameBufferId = 0;
    GLuint mFrameBufferId = 0;
    GLuint mDepthBufferId = 0;
    GLuint mRenderTextureId = 0;
    GLuint mTextureId = 0;
    bool mHasError = false;

public:
    FrameBufferObject(int textureId, int width, int height, bool msaa = false);

public:
    static inline FrameBufferObject * getFBOInstance(int width, int height) {
        FrameBufferObject * fbo = new FrameBufferObject(0, width, height);
        if (fbo->mFrameBufferId == 0)
            return NULL;
        else
            return fbo;
    }

    ~FrameBufferObject();
    void initMSAA();
    void init();

    inline void bindTexture() {
        glBindTexture(GL_TEXTURE_2D, mTextureId);
    }

    inline void unbindTexture() {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void bindFrameBuffer();
    void unbindFrameBuffer();

    inline void glViewportFull() {
        glViewport(0, 0, mWidth, mHeight);
    }

    inline int getTextureId() {
        return mTextureId;
    }

    inline int hasError() {
        return mHasError;
    }

    void clear();

    void glViewportScale(std::vector<float> lowerLeft, std::vector<float> topRight);

    void resizeFrameBuffer(float scale);

};
