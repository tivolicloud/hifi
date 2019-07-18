// "WaveVR SDK
// © 2017 HTC Corporation. All Rights Reserved.
//
// Unless otherwise required by copyright law and practice,
// upon the execution of HTC SDK license agreement,
// HTC grants you access to and use of the WaveVR SDK(s).
// You shall fully comply with all of HTC’s SDK license agreement terms and
// conditions signed by you and all SDK and API requirements,
// specifications, and documentation provided by HTC to You."

#ifndef WVR_HELLOVR_SPHERE_H
#define WVR_HELLOVR_SPHERE_H
#include <Object.h>
#include <vector>
#include <array>

class Sphere : public Object {
private:
    float mSphereDepth;
    int mModelviewProjectionLocation;
    int mPositionHandle;
    int mNormalHandle;
    int mLightLocationHandle;
    int mCameraHandle;
    int mMMatrixHandle;
    int mColor;
    int vCount = 0;

    Vector4 light_pos_world_space_;

    Vector3 mCenter;
    int mSphereColor;
    Vector3 mTranslate;
    Vector3 mOriginalPos;

public:
    Sphere(Vector3& pos);
    ~Sphere() {}

    enum Color
    {
        green = 0,
        red = 1,
        blue = 2
    };

    void setSphereColor(int color);
    void setSpherePos(Vector3& offset);
    Vector3 getCenter();

private:
    void initSphere();
    void initVertexData(std::vector<float>& alVertix);

public:
    virtual void draw(const Matrix4& projection, const Matrix4& eye, const Matrix4& view, const Vector4& lightDir);
    float getRadius();
    };
#endif //WVR_HELLOVR_SPHERE_H
