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


class ControllerCube : public Object
{
private:
    int mMatrixLocation;
    int mNormalMatrixLocation;
    int mLightDirLocation;
    bool m3DOF;
    Matrix3 mNormalMatrix;

    int mSize;
    int mTrianglesX3;
    WVR_DeviceType mDeviceType;

public:
    ControllerCube(WVR_DeviceType deviceType);
    virtual ~ControllerCube();

    void draw(const Matrix4& projection, const Matrix4& eye, const Matrix4& view, const Vector4& lightDir);

    inline void set3DOF(bool is3dof) {
        m3DOF = is3dof;
    }
    
    inline Matrix3 & getNormalMatrix() {
        return mNormalMatrix;
    }

    inline const WVR_DeviceType getDeviceType() {
        return mDeviceType;
    }

private:
    void initCubes();
    void initTexture();
    static int addCubeToScene(const Matrix4& mat, std::vector<float>& vertdata, int idx_start, std::vector<uint32_t>& indexdata);
    static void addCubeVertex(const Vector4& v, float t0, float t1, const Vector3& n, std::vector<float>& vertdata);

};

