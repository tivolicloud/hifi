// "WaveVR SDK
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#define LOG_TAG "Sphere"
#include <Sphere.h>
#include <Shader.h>
#include <Object.h>
#include <VertexArrayObject.h>
#include <glad/glad.h>

#define VERTEX_POS_INDEX 0
#define VERTEX_COLOR_INDEX 1
#define VERTEX_NORMAL_INDEX 2

namespace{
    const GLfloat red_color[3] = {0.678,0.231,0.129};
    const GLfloat green_color[3] = {0.0,1.0,0.0};
    const GLfloat blue_color[3] = {0.0,0.0,1.0};
    const float r = 0.8f;
    const float UNIT_SIZE=1.0f;
    const double
            PI=3.14159265358979323846264338327950288419716939937510582097494459230781640628;
    GLfloat lightLocation[3];
}

Sphere::Sphere(Vector3& pos) : Object(), mSphereDepth(20.0f) {
    mName = LOG_TAG;
    loadShaderFromAsset("shader/vertex/sphere_vertex.glsl", "shader/fragment/sphere_fragment.glsl");
    if (mHasError)
        return;
    mPositionHandle = mShader->getAttributeLocation("aPosition");
    mNormalHandle = mShader->getAttributeLocation("aNormal");
    mLightLocationHandle = mShader->getUniformLocation("uLightLocation");
    mColor = mShader->getUniformLocation("v_Color");
    mCameraHandle = mShader->getUniformLocation("uCamera");
    mModelviewProjectionLocation = mShader->getUniformLocation("uMVPMatrix");
    mMMatrixHandle = mShader->getUniformLocation("uMMatrix");

    mVAO = new VertexArrayObject(true, false);
    light_pos_world_space_.set(0.0f, 2.0f, 0.0f, 1.0f);
    initSphere();
    mSphereColor = green;
    mOriginalPos=pos;
    setSpherePos(mOriginalPos);
}

void setLightLocation(float x,float y,float z)
{
    lightLocation[0]=x;
    lightLocation[1]=y;
    lightLocation[2]=z;
}
float length(float x, float y, float z) {
    return sqrt(x * x + y * y + z * z);
}
void translateM(
        float m[], int mOffset,
        float x, float y, float z) {
    for (int i=0 ; i<4 ; i++) {
        int mi = mOffset + i;
        m[12 + mi] += m[mi] * x + m[4 + mi] * y + m[8 + mi] * z;
    }
}
void setRotateM(float rm[], int rmOffset,
                float a, float x, float y, float z) {
    rm[rmOffset + 3] = 0;
    rm[rmOffset + 7] = 0;
    rm[rmOffset + 11]= 0;
    rm[rmOffset + 12]= 0;
    rm[rmOffset + 13]= 0;
    rm[rmOffset + 14]= 0;
    rm[rmOffset + 15]= 1;
    a *= (float) (PI / 180.0f);
    float s = sin(a);
    float c = cos(a);
    if (1.0f == x && 0.0f == y && 0.0f == z) {
        rm[rmOffset + 5] = c;   rm[rmOffset + 10]= c;
        rm[rmOffset + 6] = s;   rm[rmOffset + 9] = -s;
        rm[rmOffset + 1] = 0;   rm[rmOffset + 2] = 0;
        rm[rmOffset + 4] = 0;   rm[rmOffset + 8] = 0;
        rm[rmOffset + 0] = 1;
    } else if (0.0f == x && 1.0f == y && 0.0f == z) {
        rm[rmOffset + 0] = c;   rm[rmOffset + 10]= c;
        rm[rmOffset + 8] = s;   rm[rmOffset + 2] = -s;
        rm[rmOffset + 1] = 0;   rm[rmOffset + 4] = 0;
        rm[rmOffset + 6] = 0;   rm[rmOffset + 9] = 0;
        rm[rmOffset + 5] = 1;
    } else if (0.0f == x && 0.0f == y && 1.0f == z) {
        rm[rmOffset + 0] = c;   rm[rmOffset + 5] = c;
        rm[rmOffset + 1] = s;   rm[rmOffset + 4] = -s;
        rm[rmOffset + 2] = 0;   rm[rmOffset + 6] = 0;
        rm[rmOffset + 8] = 0;   rm[rmOffset + 9] = 0;
        rm[rmOffset + 10]= 1;
    } else {
        float len = length(x, y, z);
        if (1.0f != len) {
            float recipLen = 1.0f / len;
            x *= recipLen;
            y *= recipLen;
            z *= recipLen;
        }
        float nc = 1.0f - c;
        float xy = x * y;
        float yz = y * z;
        float zx = z * x;
        float xs = x * s;
        float ys = y * s;
        float zs = z * s;
        rm[rmOffset +  0] = x*x*nc +  c;
        rm[rmOffset +  4] =  xy*nc - zs;
        rm[rmOffset +  8] =  zx*nc + ys;
        rm[rmOffset +  1] =  xy*nc + zs;
        rm[rmOffset +  5] = y*y*nc +  c;
        rm[rmOffset +  9] =  yz*nc - xs;
        rm[rmOffset +  2] =  zx*nc - ys;
        rm[rmOffset +  6] =  yz*nc + xs;
        rm[rmOffset + 10] = z*z*nc +  c;
    }
}
double toRadians(double vAngle) {
    double e;
    double sign=(vAngle<0.0)?-1.0:1.0;
    if(vAngle<0)
        vAngle=vAngle*(-1.0);
    e=sign*vAngle*PI/180;
    return e;
}

void Sphere:: initVertexData(std::vector<float>& alVertix) {
    int angleSpan = 10;
    for (int vAngle = -90; vAngle < 90; vAngle = vAngle + angleSpan)
    {
        for (int hAngle = 0; hAngle <= 360; hAngle = hAngle + angleSpan)
        {
            float x0 = (float) (r * UNIT_SIZE
                                * cos(toRadians(vAngle)) * cos(toRadians(hAngle)));
            float y0 = (float) (r * UNIT_SIZE
                                * cos(toRadians(vAngle)) * sin(toRadians(hAngle)));
            float z0 = (float) (r * UNIT_SIZE * sin(toRadians(vAngle)));

            float x1 = (float) (r * UNIT_SIZE
                                * cos(toRadians(vAngle)) * cos(toRadians(hAngle + angleSpan)));
            float y1 = (float) (r * UNIT_SIZE
                                * cos(toRadians(vAngle)) * sin(toRadians(hAngle + angleSpan)));
            float z1 = (float) (r * UNIT_SIZE * sin(toRadians(vAngle)));

            float x2 = (float) (r * UNIT_SIZE
                                * cos(toRadians(vAngle + angleSpan)) * cos(toRadians(hAngle + angleSpan)));
            float y2 = (float) (r * UNIT_SIZE
                                * cos(toRadians(vAngle + angleSpan)) * sin(toRadians(hAngle + angleSpan)));
            float z2 = (float) (r * UNIT_SIZE * sin(toRadians(vAngle + angleSpan)));

            float x3 = (float) (r * UNIT_SIZE
                                * cos(toRadians(vAngle + angleSpan)) * cos(toRadians(hAngle)));
            float y3 = (float) (r * UNIT_SIZE
                                * cos(toRadians(vAngle + angleSpan)) * sin(toRadians(hAngle)));
            float z3 = (float) (r * UNIT_SIZE * sin(toRadians(vAngle + angleSpan)));

            alVertix.push_back(x1);
            alVertix.push_back(y1);
            alVertix.push_back(z1);
            alVertix.push_back(x3);
            alVertix.push_back(y3);
            alVertix.push_back(z3);
            alVertix.push_back(x0);
            alVertix.push_back(y0);
            alVertix.push_back(z0);

            alVertix.push_back(x1);
            alVertix.push_back(y1);
            alVertix.push_back(z1);
            alVertix.push_back(x2);
            alVertix.push_back(y2);
            alVertix.push_back(z2);
            alVertix.push_back(x3);
            alVertix.push_back(y3);
            alVertix.push_back(z3);
        }
    }
    vCount = alVertix.size() / 3;
}
void Sphere::initSphere() {
    GLuint offset = 0;
    std::vector<float> alVertix;
    initVertexData(alVertix);
    GLfloat  normals[vCount * 3];
    for (int i = 0; i < alVertix.size(); i++) {
        normals[i] = alVertix[i];
    }

    if (!mVAO) return;
    mVAO->bindVAO();
    mVAO->bindArrayBuffer();

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * alVertix.size(), &alVertix[0], GL_STATIC_DRAW);
    glVertexAttribPointer(mPositionHandle, 3, GL_FLOAT, false, 3 * 4, (const void *)offset);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * alVertix.size(), normals, GL_STATIC_DRAW);
    glVertexAttribPointer(mNormalHandle, 3, GL_FLOAT, false, 3 * 4, (const void *)offset);
    glEnableVertexAttribArray(mPositionHandle);
    glEnableVertexAttribArray(mNormalHandle);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_BLEND);
    mVAO->unbindVAO();
}

void Sphere::draw(const Matrix4& projection, const Matrix4& eye, const Matrix4& view, const Vector4& lightDir) {
    float *currMatrix=new float[16]();//current Transformation Matrix
    GLfloat cameraLocation[3];
    setRotateM(currMatrix, 0, 0, 1, 0, 0);
    float x=mTranslate.x;
    float y=mTranslate.y;
    float z=mTranslate.z;
    translateM(currMatrix,0,x,y,z);

    if (!mEnable || mHasError || !mVAO)
        return;
    mShader->useProgram();
    mVAO->bindVAO();

    Matrix4 modelview = eye * view * currMatrix;
    Matrix4 modelview_projection = projection * modelview;
    // Set ModelView, MVP, position, normals, and color.
    setLightLocation(-4.0f,0.0f, 1.5f);
    glUniform3fv(mLightLocationHandle, 1, &lightLocation[0]);
    cameraLocation[0]=eye[0];
    cameraLocation[1]=eye[1];
    cameraLocation[2]=eye[2];
    glUniform3fv(mCameraHandle, 1,cameraLocation);
    glUniformMatrix4fv(mMMatrixHandle, 1,GL_FALSE,currMatrix);
    glUniformMatrix4fv(mModelviewProjectionLocation, 1, GL_FALSE, modelview_projection.get());

    switch(mSphereColor) {
        case green:
            glUniform3f(mColor,green_color[0],green_color[1],green_color[2]);
            break;
        case red:
            glUniform3f(mColor,red_color[0],red_color[1],red_color[2]);
            break;
        case blue:
            glUniform3f(mColor,blue_color[0],blue_color[1],blue_color[2]);
            break;
    }
    glDrawArrays(GL_TRIANGLES, 0, vCount);
    mShader->unuseProgram();
    mVAO->unbindVAO();

    mCenter.x = currMatrix[12];
    mCenter.y = currMatrix[13];
    mCenter.z = currMatrix[14];
}

float Sphere::getRadius(){
    return  r;
}

Vector3 Sphere::getCenter() {
    return mCenter;
}

void Sphere::setSphereColor(int color) {
    mSphereColor = color;
}

void Sphere::setSpherePos(Vector3& offset) {
    mTranslate=offset;

}



