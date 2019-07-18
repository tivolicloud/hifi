// "WaveVR SDK 
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#define LOG_TAG "SkyBox"
#include <SkyBox.h>
#include <VertexArrayObject.h>
#include <Shader.h>
#include <Texture.h>
#include <GLES3/gl31.h>
#include <log.h>
#include <sys/time.h>
#include <stdlib.h>

static const float SkyBoxVertices[] = {
        // Positions
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
};

enum SkyBoxEnum {
    SKYBOX_WATERSKY = 0,
    SKYBOX_GALAXY,
    SKYBOX_CLOUDDAWN,
    SKYBOX_CLOUDSUN,
    SKYBOX_GROUNDSKY,
};

static const char * SkyBoxList[] = {
    "textures/skybox_watersky.jpg",
    "textures/skybox_galaxy.jpg",
    "textures/skybox_clouddawn.jpg",
    "textures/skybox_cloudsun.jpg",
    "textures/skybox_groundsky.jpg"
};

SkyBox::SkyBox(bool debug) : Object() {
    mName = LOG_TAG;
    loadShaderFromAsset("shader/vertex/skybox_vertex.glsl", "shader/fragment/skybox_fragment.glsl");
    if (mHasError)
        return;
    mMatrixLocation = mShader->getUniformLocation("matrix");
    mTextureLocation = mShader->getUniformLocation("atexture");

    mVAO = new VertexArrayObject(true, false);
    if (debug) {
        mTexture = Texture::loadSkyboxTexture("textures/skybox_simple.jpg");
        mLightDir = Vector4(0, 0, 0, 1);
    } else {
        loadRandomTexture();
    }
    if (mTexture == NULL) {
        mHasError = true;
        return;
    }

    initVertices();
}

SkyBox::~SkyBox() {
}

void SkyBox::initVertices() {
    if (!mVAO) return;
    mVAO->bindVAO();
    mVAO->bindArrayBuffer();
    glBufferData(GL_ARRAY_BUFFER, sizeof(SkyBoxVertices), SkyBoxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    mVAO->unbindArrayBuffer();
    mVAO->unbindVAO();
}

void SkyBox::loadRandomTexture() {
    enum SkyBoxEnum r;

    struct timeval now;
    gettimeofday(&now, NULL);
    srand(now.tv_usec);
    r = static_cast<enum SkyBoxEnum>(rand() % 5);

    LOGD("Random sky box idx is %u", r);
    mTexture = Texture::loadSkyboxTexture(SkyBoxList[r]);

    // Setup light direction by sky box.
    float light_scale = 1.5f;
    switch (r) {
        case SKYBOX_WATERSKY:
            mLightDir = Vector4(-0.15f, -0.035f, -0.988f, 0.45f);
            break;
        case SKYBOX_GALAXY:
            mLightDir = Vector4(0, 0, 1.0f, 0.4f);
            break;
        case SKYBOX_CLOUDDAWN:
            mLightDir = Vector4(0.655f, 0.385f, 0.65f, 0.35f);
            break;
        case SKYBOX_CLOUDSUN:
            mLightDir = Vector4(0.7f, 0.13f, -0.7f, 0.30f);
            break;
        case SKYBOX_GROUNDSKY:
            mLightDir = Vector4(-0.8f, 0.45f, 0.4f, 0.40f);
            break;
    }
    mLightDir *= light_scale;
}

void SkyBox::setDebug(bool debug) {
    if (mTexture != NULL)
        delete mTexture;
    mTexture = NULL;

    if (debug) {
        mTexture = Texture::loadSkyboxTexture("textures/skybox_simple.jpg");
        mLightDir = Vector4(0, 0, 0, 1);
    } else {
        loadRandomTexture();
    }
}

void SkyBox::draw(const Matrix4& projection, const Matrix4& eye, const Matrix4& view, const Vector4& lightDir) {
    if (!mEnable)
        return;

    Matrix4 viewClone = view;
    const float resetTranslation[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    viewClone.setColumn(3, resetTranslation);

    // Skybox didn't need eye.
    Matrix4 matrix = projection * viewClone;

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    if (!mTexture || !mVAO) return;
    glDepthFunc(GL_LEQUAL);
    mShader->useProgram();
    glUniformMatrix4fv(mMatrixLocation, 1, GL_FALSE, matrix.get());
    glActiveTexture(GL_TEXTURE0);
    mTexture->bindTextureCubeMap();
    glUniform1i(mTextureLocation, 0);
    mVAO->bindVAO();
    glDrawArrays(GL_TRIANGLES, 0, mVertices);

    mVAO->unbindVAO();
    mTexture->unbindTextureCubeMap();
    mShader->unuseProgram();

    glDepthFunc(GL_LESS);
}
