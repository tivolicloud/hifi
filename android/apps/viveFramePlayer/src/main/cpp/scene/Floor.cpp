// "WaveVR SDK
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#define LOG_TAG "Floor"
#include <Floor.h>
#include <log.h>
#include <Shader.h>
#include <Texture.h>
#include <Object.h>
#include <VertexArrayObject.h>
#include <GLES3/gl31.h>
#include <GLES2/gl2ext.h>


#define VERTEX_POS_INDEX 0
#define VERTEX_COLOR_INDEX 1
#define VERTEX_NORMAL_INDEX 2

namespace{
float UNIT_SIZE=0.15f;
GLfloat vertices[30] =
{
    -200*UNIT_SIZE,0,-200*UNIT_SIZE,
    0,0,
    -200*UNIT_SIZE,0,200*UNIT_SIZE,
     0,1,
    200*UNIT_SIZE,0,200*UNIT_SIZE,
    1,1,

    200*UNIT_SIZE,0,200*UNIT_SIZE,
    1,1,
    200*UNIT_SIZE,0,-200*UNIT_SIZE,
    1,0,
    -200*UNIT_SIZE,0,-200*UNIT_SIZE,
    0,0
};

const GLfloat floor_color[4] = {
    0.0f, 1.0f, 0.0f, 1.0f,
};

const GLfloat floor_normal[3] = {
    0.0f, 1.0f, 0.0f,
};

} // namespace

static void dumpMatrix(const char * name, const Matrix4& mat) {
    const float * ptr = mat.get();
    LOGD("%s =\n"
        " ⎡%+6f  %+6f  %+6f  %+6f⎤\n"
        " ⎢%+6f  %+6f  %+6f  %+6f⎥\n"
        " ⎢%+6f  %+6f  %+6f  %+6f⎥\n"
        " ⎣%+6f  %+6f  %+6f  %+6f⎦\n",
        name,
        ptr[0], ptr[4], ptr[8],  ptr[12],
        ptr[1], ptr[5], ptr[9],  ptr[13],
        ptr[2], ptr[6], ptr[10], ptr[14],
        ptr[3], ptr[7], ptr[11], ptr[15]);
}

Floor::Floor() : Object(), mFloorDepth(20.0f) {
    mName = LOG_TAG;

    loadShaderFromAsset("shader/vertex/light_vertex.glsl", "shader/fragment/grid_fragment.glsl");
    if (mHasError)
        return;

    mModelLocation = mShader->getUniformLocation("u_Model");
    mModelviewLocation = mShader->getUniformLocation("u_MVMatrix");
    mModelviewProjectionLocation = mShader->getUniformLocation("u_MVP");
    mLightPosLocation = mShader->getUniformLocation("u_LightPos");

    mVAO = new VertexArrayObject(true, false);

    light_pos_world_space_.set(0.0f, 2.0f, 0.0f, 1.0f);
    mModelFloor.set(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -mFloorDepth, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);

    mTexture = Texture::loadTexture("textures/land.png");
    if (mTexture == NULL) {
        mHasError = true;
        return;
   }
    initFloor();
    initTexture();
}

void Floor::initTexture() {
    if (!mTexture) return;
    mTexture->bindTexture();
    mTexture->bindBitmap(GL_RGB5_A1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    if (Object::hasGlExtension("GL_EXT_texture_filter_anisotropic")) {
        GLfloat fLargest;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);
    }

    mTexture->unbindTexture();
    mTexture->cleanBitmap();
}

void Floor::initFloor() {
    if (!mVAO) return;

    GLuint offset = 0;
    int stride = (3 + 2) * sizeof(GLfloat);

    mVAO->bindVAO();
    mVAO->bindArrayBuffer();
    glBufferData(GL_ARRAY_BUFFER, 30 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(VERTEX_POS_INDEX);
    glVertexAttribPointer(VERTEX_POS_INDEX, 3, GL_FLOAT, false, stride, (const void*)offset);

    offset += 3*sizeof(GLfloat);
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, false, stride, (const void*)offset);
    mVAO->unbindArrayBuffer();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);

    mHasError = false;
    mVAO->unbindVAO();

}

void Floor::draw(const Matrix4& projection, const Matrix4& eye, const Matrix4& view, const Vector4& lightDir) {
    if (!mEnable || mHasError || !mVAO || !mTexture)
        return;

    mShader->useProgram();
    mVAO->bindVAO();

    light_pos_eye_space_ = eye * view * lightDir;

    // if using 'color & normal VBO' then uncomment this paragraph
    glVertexAttrib4fv(VERTEX_COLOR_INDEX, floor_color);
    glVertexAttrib3fv(VERTEX_NORMAL_INDEX, floor_normal);
    glDisableVertexAttribArray(VERTEX_COLOR_INDEX);
    glDisableVertexAttribArray(VERTEX_NORMAL_INDEX);
    // end

    float light_pos_eye_space[4] = {light_pos_eye_space_.x, light_pos_eye_space_.y, light_pos_eye_space_.z, light_pos_eye_space_.w};

    Matrix4 modelview = eye * view * getTransforms();
    Matrix4 modelview_projection = projection * modelview;

    // Set ModelView, MVP, position, normals, and color.
    glUniform3fv(mLightPosLocation, 1, light_pos_eye_space);
    glUniformMatrix4fv(mModelLocation, 1, GL_FALSE, mModelFloor.get());
    glUniformMatrix4fv(mModelviewLocation, 1, GL_FALSE, modelview.get());
    glUniformMatrix4fv(mModelviewProjectionLocation, 1, GL_FALSE, modelview_projection.get());

    glActiveTexture(GL_TEXTURE0);
    mTexture->bindTexture();
    glDrawArrays(GL_TRIANGLES, 0, 6);

    mShader->unuseProgram();
    mTexture->unbindTexture();
    mVAO->unbindVAO();

}
