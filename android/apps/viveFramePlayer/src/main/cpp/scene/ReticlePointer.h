// "WaveVR SDK 
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#pragma once
#include <Object.h>
#include <memory>
#include <string>
#include <wvr/wvr_types.h>
#include <vector>

class ReticlePointer : public Object {
private:
    int mMatrixLocation;
    int mNormalMatrixLocation;
    int mLightDirLocation;

    Matrix3 mNormalMatrix;

    int mSize;
    int mTrianglesX3;
    int mVertCount = 0;

public:
    ReticlePointer();
    virtual ~ReticlePointer();

    void draw(const Matrix4& projection, const Matrix4& eye, const Matrix4& view, const Vector4& lightDir);

    inline Matrix3 & getNormalMatrix() {
        return mNormalMatrix;
    }

    int makeVertices(const Matrix4& mat, std::vector<float>& buffer);
    void setVertices(const std::vector<float>& buffer, int verticesCount);

private:
    void init();
};