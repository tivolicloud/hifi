// "WaveVR SDK 
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#define LOG_TAG "Controller"
#include <Object.h>
#include <Context.h>
#include <Shader.h>
#include <Texture.h>
#include <VertexArrayObject.h>
#include <GLES3/gl31.h>
#include <ControllerAxes.h>

ControllerAxes::ControllerAxes() : Object() {
    mName = LOG_TAG;
    loadShaderFromAsset("shader/vertex/vc_vertex.glsl", "shader/fragment/c_fragment.glsl");
    if (mHasError)
        return;
    mMatrix = mShader->getUniformLocation("matrix");

    mVAO = new VertexArrayObject(true, false);

    init();
}

ControllerAxes::~ControllerAxes() {
}

void ControllerAxes::init() {
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

int ControllerAxes::makeVertices(const Matrix4& pose, std::vector<float>& buffer) {
    int vertCount = 0;
    const float s = 0.002f;
    const Vector4 center[4] = {
            pose * Vector4(0, s, s, 1),
            pose * Vector4(s, 0, s, 1),
            pose * Vector4(s, s, 0, 1),
            pose * Vector4(0, s, s, 1),
        };

    for (int i = 0; i < 3; ++i) {
        Vector3 color(0, 0, 0);
        Vector4 point(0, 0, 0, 1);
        point[i] += 0.06f;  // offset in X, Y, Z
        color[i] = 1.0;  // R, G, B
        point = pose * point;
        for (int j = 0; j < 3; j++) {
            buffer.push_back(center[j].x);
            buffer.push_back(center[j].y);
            buffer.push_back(center[j].z);

            buffer.push_back(color.x);
            buffer.push_back(color.y);
            buffer.push_back(color.z);

            buffer.push_back(center[j+1].x);
            buffer.push_back(center[j+1].y);
            buffer.push_back(center[j+1].z);

            buffer.push_back(color.x);
            buffer.push_back(color.y);
            buffer.push_back(color.z);

            buffer.push_back(point.x);
            buffer.push_back(point.y);
            buffer.push_back(point.z);

            buffer.push_back(color.x);
            buffer.push_back(color.y);
            buffer.push_back(color.z);
            vertCount += 3;
        }
    }

    const Vector4 start[4] = {
            pose * Vector4(0, s, s-0.02f, 1),
            pose * Vector4(s, 0, s-0.02f, 1),
            pose * Vector4(s, s,  -0.02f, 1),
            pose * Vector4(0, s, s-0.02f, 1),
        };
    Vector4 end = pose * Vector4(0, 0, -39.f, 1);
    Vector3 color(.92f, .92f, .71f);

    for (int j = 0; j < 3; j++) {
        buffer.push_back(start[j].x);
        buffer.push_back(start[j].y);
        buffer.push_back(start[j].z);
        buffer.push_back(color.x);
        buffer.push_back(color.y);
        buffer.push_back(color.z);

        buffer.push_back(start[j+1].x);
        buffer.push_back(start[j+1].y);
        buffer.push_back(start[j+1].z);
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
    return vertCount;
}

void ControllerAxes::setVertices(const std::vector<float>& buffer, int verticesCount) {
    mVertCount = verticesCount;
    if (!mVAO) return;
    if (buffer.size() > 0) {
        mVAO->bindArrayBuffer();
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * buffer.size(), &buffer[0], GL_STREAM_DRAW);
        mVAO->unbindArrayBuffer();
    }
}

void ControllerAxes::draw(const Matrix4& projection, const Matrix4& eye, const Matrix4& view, const Vector4& lightDir) {
    if (!mEnable || !mVAO)
        return;

    Matrix4 matrix = projection * eye * view;

    mShader->useProgram();
    glUniformMatrix4fv(mMatrix, 1, false, matrix.get());
    mVAO->bindVAO();
    glDrawArrays(GL_TRIANGLES, 0, mVertCount);

    mVAO->unbindVAO();
    mShader->unuseProgram();
}
