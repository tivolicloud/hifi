// "WaveVR SDK 
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#define LOG_TAG "Picture"
#include <Object.h>
#include <Context.h>
#include <Shader.h>
#include <Texture.h>
#include <VertexArrayObject.h>
#include <glad/glad.h>
#include <Picture.h>

Picture::Picture() : Object() {
    mName = LOG_TAG;
    loadShaderFromAsset("shader/vertex/vt_vertex.glsl", "shader/fragment/t_fragment.glsl");
    if (mHasError)
        return;
    mEnable = false;
    mMatrixLocation = mShader->getUniformLocation("matrix");

    mVAO = new VertexArrayObject(true, false);
    mTexture = Texture::loadTexture("textures/flsw_egr.png");
    if (mTexture == NULL) {
        mHasError = true;
        return;
    }

    if (!mVAO) return;

    mVAO->bindVAO();
    mVAO->bindArrayBuffer();

    const float scale = 1.2f;
    const float x = 1.0f * scale;
    const float y = 1.0f * scale * (float) mTexture->getHeight() / (float) mTexture->getWidth();
    const float z = -2.0f;
    const float VrtxCoord [] = {
            //AB
            //CD
            // Textcoord need upside down
            -x,  y, z, 0, 0,  // A
            -x, -y, z, 0, 1,  // C
             x, -y, z, 1, 1,  // D
            -x,  y, z, 0, 0,  // A
             x, -y, z, 1, 1,  // D
             x,  y, z, 1, 0   // B
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(VrtxCoord), VrtxCoord, GL_STREAM_DRAW);

    int stride = (2 + 3) * sizeof(float);
    GLuint offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, (const void*) offset);

    offset += sizeof(float) * 3;
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, stride, (const void*) offset);

    mVAO->unbindVAO();
    mVAO->unbindArrayBuffer();

    mTexture->bindTexture();
    mTexture->bindBitmap(GL_RGB5_A1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    mTexture->unbindTexture();
    mTexture->cleanBitmap();
}

Picture::~Picture() {
}

void Picture::draw(const Matrix4& projection, const Matrix4& eye, const Matrix4& view, const Vector4& lightDir) {
    if (!mEnable)
        return;

    if (!mVAO)
        return;

    Matrix4 matrix = projection;

    mShader->useProgram();
    glUniformMatrix4fv(mMatrixLocation, 1, false, matrix.get());

    glActiveTexture(GL_TEXTURE0);
    mTexture->bindTexture();

    mVAO->bindVAO();
    glDrawArrays(GL_TRIANGLES, 0, 6);
    mShader->unuseProgram();
    mVAO->unbindVAO();
}
