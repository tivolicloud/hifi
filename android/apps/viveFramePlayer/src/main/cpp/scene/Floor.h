// "WaveVR SDK
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#ifndef PROFILINGTOOL_FLOOR_H
#define PROFILINGTOOL_FLOOR_H

#include <Object.h>
#include <vector>
#include <array>

class Floor : public Object {
private:
    float mFloorDepth;

    int mModelLocation;
    int mModelviewLocation;
    int mModelviewProjectionLocation;
    int mLightPosLocation;

    Matrix4 mModelFloor;

    Vector4 light_pos_world_space_;
    Vector4 light_pos_eye_space_;

public:
    Floor();
    ~Floor() {}

private:
   void initFloor();
   void initTexture();

public:
    virtual void draw(const Matrix4& projection, const Matrix4& eye, const Matrix4& view, const Vector4& lightDir);
};

#endif //PROFILINGTOOL_FLOOR_H
