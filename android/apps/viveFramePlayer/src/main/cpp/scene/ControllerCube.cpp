// "WaveVR SDK 
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#define LOG_TAG "ControllerCube"
#include <log.h>
#include <Object.h>
#include <Context.h>
#include <Shader.h>
#include <Texture.h>
#include <VertexArrayObject.h>
#include <glad/glad.h>
#include <ControllerCube.h>

//-----------------------------------------------------------------------------
// Purpose: Create/destroy GL Render Models
//-----------------------------------------------------------------------------
ControllerCube::ControllerCube(WVR_DeviceType deviceType)
    : Object(), mDeviceType(deviceType), mMatrixLocation(0),
    mNormalMatrixLocation(0), mLightDirLocation(0) {

    mName = LOG_TAG;
    loadShaderFromAsset("shader/vertex/vtn_vertex.glsl", "shader/fragment/ti_fragment.glsl");
    if (mHasError)
        return;
    mMatrixLocation = mShader->getUniformLocation("matrix");
    mNormalMatrixLocation = mShader->getUniformLocation("normalm");
    mLightDirLocation = mShader->getUniformLocation("l_dir");

    mVAO = new VertexArrayObject(true, true);

    mTexture = Texture::loadTexture("textures/cube_controller.jpg");
    if (mTexture == NULL) {
        mHasError = true;
        return;
    }

    initCubes();
    initTexture();
}


ControllerCube::~ControllerCube() {
}


void ControllerCube::initCubes() {
    if (!mVAO) return;

    std::vector<float> vertdataarray;
    std::vector<uint32_t> indexarray;
    mHasError = true;

    const float scale = 0.06f;
    Matrix4 matScale;
    matScale.scale(scale, scale, scale);
    Matrix4 matTransform;
    matTransform.identity();
    Matrix4 mat = matScale * matTransform;
    int index = 0;

    index = addCubeToScene(mat, vertdataarray, index, indexarray);
    if (indexarray.size() == 0)
        return;

    mVAO->bindVAO();

    // populate the vertex buffer
    mVAO->bindArrayBuffer();
    GLuint offset = 0;
    int stride = (3 + 2 + 3) * sizeof(float);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertdataarray.size(), &vertdataarray[0], GL_STATIC_DRAW);
    if (hasGLError())
        return;

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, (const void *)offset);

    offset += 3 * sizeof(float);
    glEnableVertexAttribArray( 1);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, stride, (const void *)offset);

    offset += 2 * sizeof(float);
    glEnableVertexAttribArray( 2);
    glVertexAttribPointer(2, 3, GL_FLOAT, false, stride, (const void *)offset);

    // populate the index buffer
    mVAO->bindElementArrayBuffer();
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indexarray.size(), &indexarray[0], GL_STATIC_DRAW);
    if (hasGLError())
        return;

    mTrianglesX3 = indexarray.size();
    mHasError = false;
    mVAO->unbindVAO();
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

void ControllerCube::initTexture() {
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

void ControllerCube::addCubeVertex(const Vector4& v, float t0, float t1, const Vector3& n, std::vector<float>& vertdata) {
    vertdata.push_back(v.x);
    vertdata.push_back(v.y);
    vertdata.push_back(v.z);
    vertdata.push_back(t0);
    vertdata.push_back(t1);
    vertdata.push_back(n.x);
    vertdata.push_back(n.y);
    vertdata.push_back(n.z);
}

int ControllerCube::addCubeToScene(const Matrix4& mat, std::vector<float>& vertdata, int idx_start, std::vector<uint32_t>& indexdata) {
    int vc = 0;  // vertex count

    Vector4 A = mat * Vector4(-0.5f, -0.5f, -0.5f, 1);
    Vector4 B = mat * Vector4(0.5f, -0.5f, -0.5f, 1);
    Vector4 C = mat * Vector4(0.5f, 0.5f, -0.5f, 1);
    Vector4 D = mat * Vector4(-0.5f, 0.5f, -0.5f, 1);
    Vector4 E = mat * Vector4(-0.5f, -0.5f, 0.5f, 1);
    Vector4 F = mat * Vector4(0.5f, -0.5f, 0.5f, 1);
    Vector4 G = mat * Vector4(0.5f, 0.5f, 0.5f, 1);
    Vector4 H = mat * Vector4(-0.5f, 0.5f, 0.5f, 1);

    Vector3 NA(-1,-1,-1);
    Vector3 NB( 1,-1,-1);
    Vector3 NC( 1, 1,-1);
    Vector3 ND(-1, 1,-1);
    Vector3 NE(-1,-1, 1);
    Vector3 NF( 1,-1, 1);
    Vector3 NG( 1, 1, 1);
    Vector3 NH(-1, 1, 1);
    /*
         D-----C
        /|    /|
       H-+---G |
       | |   | |
       | A---+-B
       |/    |/
       E-----F
    */

    // Left
    addCubeVertex(A, 0, 2/3.0f, NA, vertdata); vc++;      // 0
    addCubeVertex(D, 0, 1/3.0f, ND, vertdata); vc++;      // 1
    // Right
    addCubeVertex(B, 3/4.0f, 2/3.0f, NB, vertdata); vc++; // 2
    addCubeVertex(C, 3/4.0f, 1/3.0f, NC, vertdata); vc++; // 3
    // Front
    addCubeVertex(E, 1/4.0f, 2/3.0f, NE, vertdata); vc++; // 4
    addCubeVertex(F, 2/4.0f, 2/3.0f, NF, vertdata); vc++; // 5
    addCubeVertex(G, 2/4.0f, 1/3.0f, NG, vertdata); vc++; // 6
    addCubeVertex(H, 1/4.0f, 1/3.0f, NH, vertdata); vc++; // 7

    // Top
    addCubeVertex(C, 2/4.0f, 0, NC, vertdata); vc++;      // 8
    addCubeVertex(D, 1/4.0f, 0, ND, vertdata); vc++;      // 9

    // Bottom
    addCubeVertex(A, 1/4.0f, 1, NA, vertdata); vc++;      // 10
    addCubeVertex(B, 2/4.0f, 1, NB, vertdata); vc++;      // 11
    
    // Back
    addCubeVertex(A, 1, 2/3.0f, NA, vertdata); vc++;      // 12
    addCubeVertex(D, 1, 1/3.0f, ND, vertdata); vc++;      // 13

    int indexes[] = {
        0,7,1, 0,4,7, // left
        10,5,4, 10,11,5, // bottom
        12,3,2, 12,13,3, // back
        6,9,7, 6,8,9, // top
        6,4,5, 6,7,4, // front
        6,2,3, 6,5,2  // right
    };

    const size_t N = sizeof(indexes) / sizeof(int);
    for (int i = 0; i < N; i++)
        indexdata.push_back(indexes[i] + idx_start);

    return idx_start + vc;
}

void ControllerCube::draw(const Matrix4& projection, const Matrix4& eye, const Matrix4& view, const Vector4& lightDir) {
    if (!mEnable || mHasError || !mVAO || !mTexture)
        return;

    Matrix4 matrix = projection * eye * view * getTransforms();
    if (!m3DOF)
        mNormalMatrix = makeNormalMatrix(getTransforms());

    mShader->useProgram();
    glUniformMatrix4fv(mMatrixLocation, 1, false, matrix.get());
    glUniformMatrix3fv(mNormalMatrixLocation, 1, false, mNormalMatrix.get());
    glUniform4f(mLightDirLocation, lightDir.x, lightDir.y, lightDir.z, lightDir.w);
    mVAO->bindVAO();
    glActiveTexture(GL_TEXTURE0);
    mTexture->bindTexture();
    glDrawElements(GL_TRIANGLES, mTrianglesX3, GL_UNSIGNED_INT, 0);

    mShader->unuseProgram();
    mTexture->unbindTexture();
    mVAO->unbindVAO();
}
