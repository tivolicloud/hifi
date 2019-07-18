// "WaveVR SDK 
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#define LOG_TAG "ReticlePointer"
#include <log.h>
#include <Object.h>
#include <Context.h>
#include <Shader.h>
#include <Texture.h>
#include <VertexArrayObject.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl31.h>
#include <ReticlePointer.h>

//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
ReticlePointer::ReticlePointer() : Object() {

    mName = LOG_TAG;
    loadShaderFromAsset("shader/vertex/vc_vertex.glsl", "shader/fragment/c_fragment.glsl");
    if (mHasError)
        return;
    mMatrixLocation = mShader->getUniformLocation("matrix");

    mVAO = new VertexArrayObject(true, true);

    init();
}


ReticlePointer::~ReticlePointer() {
}

void ReticlePointer::init() {
    if (!mVAO) return;
    mVAO->bindVAO();
    mVAO->bindArrayBuffer();

    const int stride = 2 * 3 * sizeof(float);
    GLuint offset = 0;
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, (const void *) offset);
    offset += 3 * sizeof(float);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, stride, (const void *) offset);

    mVAO->unbindVAO();
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    mVAO->unbindArrayBuffer();
}

int ReticlePointer::makeVertices(const Matrix4& pose, std::vector<float>& buffer) {
    int vertCount = 0;
    const float size = 0.02f;
    const float z_distance = -2.2f;

    const Vector4 edge[4] = {
            pose * Vector4(size, 0, z_distance, 1), //right
            pose * Vector4(0, size, z_distance, 1), //top
            pose * Vector4(-size, 0, z_distance, 1), //left
            pose * Vector4(0, -size, z_distance, 1), //bottom
    };
    Vector4 end = pose * Vector4(0, 0, z_distance, 1);
    Vector3 color(1.0f, 1.0f, 1.0f); //white

    for (int i = 0; i < 3; i++) {
        buffer.push_back(edge[i].x);
        buffer.push_back(edge[i].y);
        buffer.push_back(edge[i].z);
        buffer.push_back(color.x);
        buffer.push_back(color.y);
        buffer.push_back(color.z);

        buffer.push_back(edge[i+1].x);
        buffer.push_back(edge[i+1].y);
        buffer.push_back(edge[i+1].z);
        buffer.push_back(color.x);
        buffer.push_back(color.y);
        buffer.push_back(color.z);

        buffer.push_back(end.x);
        buffer.push_back(end.y);
        buffer.push_back(end.z);
        buffer.push_back(color.x);
        buffer.push_back(color.y);
        buffer.push_back(color.z);
        vertCount += 3;
    }

    buffer.push_back(edge[3].x);
    buffer.push_back(edge[3].y);
    buffer.push_back(edge[3].z);
    buffer.push_back(color.x);
    buffer.push_back(color.y);
    buffer.push_back(color.z);

    buffer.push_back(edge[0].x);
    buffer.push_back(edge[0].y);
    buffer.push_back(edge[0].z);
    buffer.push_back(color.x);
    buffer.push_back(color.y);
    buffer.push_back(color.z);

    buffer.push_back(end.x);
    buffer.push_back(end.y);
    buffer.push_back(end.z);
    buffer.push_back(color.x);
    buffer.push_back(color.y);
    buffer.push_back(color.z);
    vertCount += 3;

    return vertCount;
}

void ReticlePointer::setVertices(const std::vector<float>& buffer, int verticesCount) {
    mVertCount = verticesCount;
    if (!mVAO) return;
    if (buffer.size() > 0) {
        mVAO->bindArrayBuffer();
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * buffer.size(), &buffer[0], GL_STREAM_DRAW);
        mVAO->unbindArrayBuffer();
    }
}

void ReticlePointer::draw(const Matrix4& projection, const Matrix4& eye, const Matrix4& view, const Vector4& lightDir) {
    if (!mEnable || mHasError || !mVAO )
        return;

    Matrix4 matrix = projection * eye * view;

    mShader->useProgram();
    glUniformMatrix4fv(mMatrixLocation, 1, false, matrix.get());
    mVAO->bindVAO();
    glDrawArrays(GL_TRIANGLES, 0, mVertCount);

    mVAO->unbindVAO();
    mShader->unuseProgram();
}
